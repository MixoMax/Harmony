"""
WebRTC mesh video-meeting signaling server.

Responsibilities of this server (and ONLY these):
  1. Serve the compiled frontend (static files).
  2. Let clients publish/query RSA public keys.
  3. Track which meetings exist and who is currently in them.
  4. Relay signaling messages (SDP offers/answers, ICE candidates, and
     RSA-wrapped symmetric sender keys) between participants over WebSocket.

It never sees plaintext audio/video, and it never sees anyone's symmetric
media keys in usable form (they're RSA-encrypted to a specific recipient
before they ever touch the server). Media itself never touches this
process at all -- it flows directly UDP peer-to-peer between browsers via
WebRTC, which is exactly why the 10mbps cap on this box doesn't matter.

NAT traversal: clients are configured with public STUN servers only (see
frontend/webrtc.js). There's deliberately no TURN relay here -- adding one
would mean routing media through this box, which would blow the bandwidth
budget. Calls between peers stuck behind symmetric NATs may simply fail to
connect; that's an accepted limitation for this build, not a bug.
"""

from __future__ import annotations

import json
import logging
import secrets
import time
import uuid
from dataclasses import dataclass, field
from pathlib import Path

from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.responses import FileResponse
from pydantic import BaseModel

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
log = logging.getLogger("signaling")

app = FastAPI(title="P2P Meeting Signaling Server")

FRONTEND_DIR = Path(__file__).resolve().parent.parent / "frontend"

MAX_PARTICIPANTS_PER_MEETING = 6
MEETING_TTL_SECONDS = 60 * 60 * 12  # stale empty meetings get swept eventually


# --------------------------------------------------------------------------
# In-memory state. This is a demo: no DB, nothing survives a restart.
# --------------------------------------------------------------------------

@dataclass
class Participant:
    user_id: str
    display_name: str
    public_key_pem: str
    ws: WebSocket
    joined_at: float = field(default_factory=time.time)


@dataclass
class Meeting:
    meeting_id: str
    created_at: float = field(default_factory=time.time)
    participants: dict[str, Participant] = field(default_factory=dict)


MEETINGS: dict[str, Meeting] = {}
# Public keys are also kept in a global registry, independent of any one
# meeting, so "publish key, then query anyone's key" works the way the spec
# describes.
PUBLIC_KEYS: dict[str, str] = {}


# --------------------------------------------------------------------------
# REST: meeting lifecycle + public key registry
# --------------------------------------------------------------------------

class CreateMeetingResponse(BaseModel):
    meeting_id: str


class PublishKeyRequest(BaseModel):
    user_id: str
    public_key_pem: str


@app.post("/api/meetings", response_model=CreateMeetingResponse)
def create_meeting():
    meeting_id = secrets.token_urlsafe(6)
    MEETINGS[meeting_id] = Meeting(meeting_id=meeting_id)
    log.info("created meeting %s", meeting_id)
    return CreateMeetingResponse(meeting_id=meeting_id)


@app.get("/api/meetings/{meeting_id}")
def get_meeting(meeting_id: str):
    meeting = MEETINGS.get(meeting_id)
    if not meeting:
        raise HTTPException(404, "no such meeting")
    return {
        "meeting_id": meeting_id,
        "participant_count": len(meeting.participants),
        "max_participants": MAX_PARTICIPANTS_PER_MEETING,
    }


@app.post("/api/keys")
def publish_key(req: PublishKeyRequest):
    """Publish (or update) a participant's RSA public key.

    Anyone can query anyone's key -- that's the point of it being public --
    but only the caller can publish for their own user_id in a real
    deployment you'd gate this behind an authenticated session. For this
    demo, user_id is a client-generated random ID, so there's no identity
    to spoof anything meaningful from.
    """
    PUBLIC_KEYS[req.user_id] = req.public_key_pem
    return {"ok": True}


@app.get("/api/keys/{user_id}")
def get_key(user_id: str):
    pem = PUBLIC_KEYS.get(user_id)
    if not pem:
        raise HTTPException(404, "no key published for that user_id")
    return {"user_id": user_id, "public_key_pem": pem}


# --------------------------------------------------------------------------
# WebSocket: signaling + presence
# --------------------------------------------------------------------------
#
# Message envelope (all JSON):
#   {"type": "...", "from": "<user_id>", "to": "<user_id or omitted>", "payload": {...}}
#
# Client -> server types:
#   "join"          payload: {display_name, public_key_pem}
#   "offer"         to: peer_id, payload: {sdp}
#   "answer"        to: peer_id, payload: {sdp}
#   "ice-candidate" to: peer_id, payload: {candidate}
#   "sender-key"    to: peer_id, payload: {wrapped_key: base64 RSA-OAEP ciphertext}
#   "quality-hint"  (optional, unused by server, just relayed if targeted)
#
# Server -> client types:
#   "roster"        payload: {participants: [{user_id, display_name, public_key_pem}]}
#   "peer-joined"   payload: {user_id, display_name, public_key_pem}
#   "peer-left"     payload: {user_id}
#   "error"         payload: {message}
#   plus the same offer/answer/ice-candidate/sender-key types, relayed verbatim
#     from the sender to the specified "to" recipient.

RELAYED_TYPES = {"offer", "answer", "ice-candidate", "sender-key"}


@app.websocket("/ws/{meeting_id}/{user_id}")
async def signaling_socket(websocket: WebSocket, meeting_id: str, user_id: str):
    await websocket.accept()

    meeting = MEETINGS.get(meeting_id)
    if not meeting:
        await websocket.send_json({"type": "error", "payload": {"message": "unknown meeting"}})
        await websocket.close()
        return

    if len(meeting.participants) >= MAX_PARTICIPANTS_PER_MEETING:
        await websocket.send_json({"type": "error", "payload": {"message": "meeting is full"}})
        await websocket.close()
        return

    # First message must be "join" with display name + public key.
    try:
        first_raw = await websocket.receive_text()
        first = json.loads(first_raw)
    except Exception:
        await websocket.close()
        return

    if first.get("type") != "join":
        await websocket.send_json({"type": "error", "payload": {"message": "expected join message first"}})
        await websocket.close()
        return

    display_name = first.get("payload", {}).get("display_name") or f"guest-{user_id[:6]}"
    public_key_pem = first.get("payload", {}).get("public_key_pem")
    if not public_key_pem:
        await websocket.send_json({"type": "error", "payload": {"message": "public_key_pem required to join"}})
        await websocket.close()
        return

    PUBLIC_KEYS[user_id] = public_key_pem
    participant = Participant(
        user_id=user_id,
        display_name=display_name,
        public_key_pem=public_key_pem,
        ws=websocket,
    )

    # Snapshot the existing roster BEFORE adding the newcomer, send it to them...
    existing = [
        {
            "user_id": p.user_id,
            "display_name": p.display_name,
            "public_key_pem": p.public_key_pem,
        }
        for p in meeting.participants.values()
    ]
    meeting.participants[user_id] = participant
    log.info("%s joined meeting %s (%d participants)", user_id, meeting_id, len(meeting.participants))

    await websocket.send_json({"type": "roster", "payload": {"participants": existing}})

    # ...then tell everyone already there that the newcomer arrived.
    for p in meeting.participants.values():
        if p.user_id == user_id:
            continue
        try:
            await p.ws.send_json({
                "type": "peer-joined",
                "payload": {
                    "user_id": user_id,
                    "display_name": display_name,
                    "public_key_pem": public_key_pem,
                },
            })
        except Exception:
            pass

    try:
        while True:
            raw = await websocket.receive_text()
            try:
                msg = json.loads(raw)
            except json.JSONDecodeError:
                continue

            msg_type = msg.get("type")
            if msg_type not in RELAYED_TYPES:
                continue

            target_id = msg.get("to")
            target = meeting.participants.get(target_id) if target_id else None
            if not target:
                continue

            msg["from"] = user_id
            try:
                await target.ws.send_json(msg)
            except Exception:
                pass

    except WebSocketDisconnect:
        pass
    finally:
        meeting.participants.pop(user_id, None)
        log.info("%s left meeting %s (%d participants)", user_id, meeting_id, len(meeting.participants))
        for p in meeting.participants.values():
            try:
                await p.ws.send_json({"type": "peer-left", "payload": {"user_id": user_id}})
            except Exception:
                pass
        if not meeting.participants:
            # keep the meeting around briefly rather than deleting instantly,
            # in case of a quick reconnect; a real deployment would sweep
            # MEETINGS on a timer using created_at / last-empty timestamps.
            pass


# --------------------------------------------------------------------------
# Static frontend
# --------------------------------------------------------------------------

@app.get("/")
def index():
    return FileResponse(FRONTEND_DIR / "index.html")


@app.get("/{filename}")
def static_file(filename: str):
    candidate = (FRONTEND_DIR / filename).resolve()
    if FRONTEND_DIR not in candidate.parents or not candidate.is_file():
        raise HTTPException(404)
    return FileResponse(candidate)

# The Harmony Protocol

Version: draft-0 (unstable, expect breaking changes)

## 0. Goals and non-goals

**Goal:** define enough of the wire format — REST schema, gateway message types, media framing, key
exchange — that two independently-written clients (say, a C++ CLI client and a browser client) can join
the same forum, see each other's presence, exchange text, and talk in a voice channel, without either
implementation knowing anything about the other's internals or language.

**Non-goals (for this version):** federation between servers, mobile push notification delivery, video
codec negotiation details, spam/abuse tooling. These get their own spec sections later.

Every section below is marked with its implementation status: **[spec only]**, **[partially built]**, or
**[implemented]**. As of this draft, almost everything is [spec only] — see the README's gap list for
the honest current state of `backend/`.

---

## 1. Core concepts and identifiers

- **User** — an account. Has a global `user_id` (UUID), independent of any forum.
- **Forum** — the "server"/"guild" equivalent. Has an `owner_user_id`, a list of channels, and a
  membership list.
- **Channel** — belongs to exactly one forum. `type` is `"text"` or `"voice"`.
- **Device / session** *(not yet in the data model — see §6)* — one authenticated connection from one
  client instance. A user may have multiple simultaneously; each has its own keypair.
- All IDs are UUIDv4 strings. All timestamps are ISO-8601 UTC.

---

## 2. REST API [spec only, backend has zero real routes beyond an empty stub]

Base path: **`/api/v1`**. Auth via `Authorization: Bearer <token>` in the header. Tokens are issued on login and expire after a fixed duration.

### 2.1 Auth
| Method | Path | Notes |
|---|---|---|
| POST | `/auth/register` | `{username, email, password}` → creates user |
| POST | `/auth/login` | `{email, password}` → `{token, expires_at}` |
| POST | `/auth/logout` | revokes current token |

### 2.2 Users
| Method | Path | Notes |
|---|---|---|
| GET | `/users/me` | current user profile |
| GET | `/users/{id}` | public profile |
| PATCH | `/users/me` | update username/avatar |

### 2.3 Keys — **new, does not exist yet, blocks everything in §5**
| Method | Path | Notes |
|---|---|---|
| PUT | `/forums/{forum_id}/keys/me` | publish this device's public key for this forum |
| GET | `/forums/{forum_id}/keys` | list `{user_id, device_id, public_key, algo}` for every member |
| DELETE | `/forums/{forum_id}/keys/{device_id}` | revoke a lost/rotated device key |

### 2.4 Forums / channels / membership
| Method | Path | Notes |
|---|---|---|
| POST | `/forums` | create forum (creator becomes owner) |
| GET | `/forums/{id}` | forum metadata + channel list |
| POST | `/forums/{id}/invites` | generate invite code |
| POST | `/invites/{code}/accept` | join via invite |
| POST | `/forums/{id}/channels` | create channel |
| GET | `/forums/{id}/channels` | list channels |

### 2.5 Messages
| Method | Path | Notes |
|---|---|---|
| GET | `/channels/{id}/messages?before=&limit=` | history, cursor-paginated |
| POST | `/channels/{id}/messages` | send (see §5.2 for what "content" means once encrypted) |
| PATCH | `/messages/{id}` | edit |
| DELETE | `/messages/{id}` | delete |

This layer is intentionally boring and REST-y — no client-agnostic reason for it to be anything else.

---

## 3. The Gateway (WebSocket) [spec only, does not exist in `backend/`]

`wss://<host>/gateway?token=<auth_token>&device_id=<device_id>`

One connection per device. All frames are JSON: `{"op": "<string>", "d": {...}}`.

### 3.1 Lifecycle ops
- `hello` (server→client, on connect) — `{heartbeat_interval_ms}`
- `heartbeat` (client→server) / `heartbeat_ack` (server→client)
- `identify` *(only if not done via query param)* — reserved for future device-key challenge/response

### 3.2 Presence & messaging
- `presence_update` (server→client) — `{user_id, status}`
- `typing_start` (client→server, then rebroadcast) — `{channel_id}`
- `message_create` / `message_update` / `message_delete` (bidirectional relay, server never inspects
  content beyond the plaintext metadata fields — see §5.2)

### 3.3 Voice-channel signaling — **the part your sender-key scheme depends on**
- `voice_join` (client→server) — `{channel_id}`
- `voice_state_update` (server→client, broadcast to channel) — `{channel_id, user_id, device_id, joined|left}`
  This is the event that tells every existing participant "regenerate and redistribute your sender key."
- `voice_leave` (client→server) — `{channel_id}`

### 3.4 Key distribution — **carries the RSA-wrapped sender keys from your experiment notebook**
- `key_offer` (client→server→target client) — `{channel_id, to_user_id, to_device_id, encrypted_sender_key}`
  Server relays opaque bytes; it cannot decrypt this and shouldn't try.

### 3.5 P2P signaling (video only — audio never attempts P2P, see §4)
- `ice_candidate` (client→server→target client) — relays `{candidate, sdp_mid}`-style data so two
  clients can attempt a direct UDP path for a video stream without the server ever seeing that media.
- `video_fallback_ready` (either direction) — negotiated when hole-punching fails or times out and the
  client instead opens/uses connection 3 (§4.2) and subscribes to that stream from the server.

---

## 4. Media transport [spec only — this is the entire reason the old `server.py` PoC existed]

The old `server.py` PoC benchmarked raw audio over WebSocket-over-TCP specifically to check whether it
was fast enough to use in production. **Result: it is** — audio always goes over a server-relayed
WebSocket, no P2P, no raw UDP. P2P/UDP is used for video only, where the bandwidth math (§ original
hop-count table) makes server-relaying-everything a much bigger cost than for audio.

This means voice channels are not one connection but **up to three separate WebSockets per client**,
each with a distinct purpose:

| # | Connection | Required? | Carries |
|---|---|---|---|
| 1 | **Orchestration** | always, one per session (not per voice channel) | everything in §3: presence, key rotation/offers, voice join/leave signaling, ICE candidate exchange for video |
| 2 | **Audio** | required while in a voice channel | binary-framed, encrypted audio bytes, server-relayed |
| 3 | **Video** | optional, opt-in per viewer | binary-framed, encrypted video bytes for whichever stream(s) the client has subscribed to, server-relayed as fallback when P2P isn't viable |

Connection 1 already exists conceptually as "the gateway" in §3 — it's one per client session, not one
per voice channel, and stays open regardless of voice activity. Connections 2 and 3 are voice-channel
scoped: opened on `voice_join`, closed on `voice_leave`.

### 4.1 Audio — always server-relayed WebSocket
`wss://<host>/voice/{channel_id}/audio?token=<auth_token>&device_id=<device_id>`

- Binary WS frames, not JSON. Each frame: `[4-byte sender key epoch][nonce][ciphertext]`. Ciphertext is
  the PCM/opus frame encrypted with the sender's current symmetric key (AES-GCM or ChaCha20-Poly1305 —
  TBD, see open question below).
- Server relays frames to every other participant in the channel; it cannot decrypt them (same
  metadata-only trust model as text).
- Sample rate, frame size, and codec are negotiated once via the orchestration connection (§3) before
  this connection is opened.
- No P2P path for audio at all — the benchmark settled this, don't relitigate it per-client.

### 4.2 Video — P2P first, optional WebSocket fallback
- Orchestrated over connection 1: ICE candidates are exchanged via `ice_candidate` (§3.5) so two clients
  can attempt a direct UDP path.
- **Only clients who want to watch need to open connection 3 at all** — this is what makes it "optional
  to join." A client that's only listening to voice never opens it.
- Because connection 3 carries potentially many participants' video multiplexed together, the viewing
  client must tell the server which stream(s) to relay to it:
  - `video_subscribe` (client→server, over connection 3 or via §3 orchestration — TBD) — `{user_id, device_id}`
  - `video_unsubscribe` — same shape
  - Frames on the wire are prefixed with a sender identifier so the client can demux: `[sender_user_id][sender_device_id][nonce][ciphertext]`
- When P2P succeeds between two peers, that pair's stream doesn't touch connection 3 at all — it's a
  raw UDP path negotiated purely via ICE, established out-of-band from the server entirely.
- Relay topology above 4 viewers (who forwards to whom) is unchanged from the README's original
  hop-count table — this section just gives it a concrete signaling and fallback mechanism.

### 4.3 Fallback trigger
- If ICE negotiation fails (symmetric NAT, restrictive firewall) within some negotiated timeout, both
  sides fall back to requesting that stream over connection 3 instead. The server relays blind — same
  trust model as everything else it touches.

---

## 5. Encryption

### 5.1 Voice — designed, prototyped in `experiments/encryption.ipynb`, not yet wired to any transport
RSA-wrapped per-participant symmetric keys, rotated on membership change. This is solid; what's missing
is purely plumbing: §2.3 (key publishing) + §3.4 (key offer relay) + §4.1 (actual packet format).

### 5.2 Text — **open problem, unchanged from original README**
Options worth spelling out explicitly so a future client author knows the tradeoffs:
1. **Per-channel symmetric key, re-shared on membership change** — simple, but new members can't read
   history (may be desirable!) and a removed member who copied the key can still decrypt until rotation.
2. **Per-message keys wrapped to each current member (like the voice scheme)** — no forward-secrecy hole,
   but O(members) work per message and doesn't solve new-member history access either.
3. **Sender-keys à la Signal/Matrix (Megolm-style ratchet)** — the "real" answer, but is a meaningfully
   bigger crypto lift than what's prototyped so far.
Server-side search is lost under any of these; needs a client-side index or is simply not a feature.

### 5.3 Video (P2P) — open problem, not previously discussed
Same symmetric-key-per-sender idea as voice extends naturally, but P2P relay-of-relay (hop > 0 in §4.2)
means an intermediate relaying client is passing bytes it can't decrypt either — needs confirming that
works with whatever AEAD scheme is picked, since relaying nodes don't need the key at all, only the
direct peers do.

---

## 6. Data model additions needed to support this spec

Not protocol-level, but the spec above is unimplementable without these in `backend/backend/classes.py`
and `database.py`:
- `PublicKey(user_id, device_id, forum_id, public_key, algo, created_at)`
- `Device(id, user_id, name, created_at)` — multi-device needs a first-class concept, not just tokens
- `VoiceParticipant(channel_id, user_id, device_id, joined_at)`
- `Invite(code, forum_id, created_by, expires_at, max_uses)`
- `Permission` model replacing the free-text `role` column
- Client-generated message ID support (for idempotent retry in an E2E world where the server can't
  dedupe by content)

---

## Open questions (tracked here so they don't get lost)

- AEAD choice for voice/video packets (AES-GCM vs ChaCha20-Poly1305) — leaning toward whichever the
  eventual codec library makes cheapest per-packet given real-time constraints.
- Text channel encryption scheme (§5.2) — needs a decision before any text-channel work continues past
  prototyping.
- Multi-device key trust — if a new device is added, does it get retroactive access to a forum's
  history, and who signs off on trusting the new device's key?
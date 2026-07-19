# Harmony

Our own shitty Discord clone.

## Premises

- Discord uses too much RAM and CPU because it's built on Electron → Harmony should be lightweight.
- Discord has rolled out age/ID verification and reads messages to guess age → Harmony treats end-to-end
  encryption as a first-class citizen. The server should only ever see **metadata** (who sent something,
  when, and in which channel) — never message content, voice audio, or video.
- No single implementation should be "the" client. Harmony is a **protocol first, apps second**. Anyone
  should be able to write a fully-interoperable client — text-only, voice-only, TUI, whatever — against
  the spec in [`HARMONY_PROTOCOL.md`](./HARMONY_PROTOCOL.md), without touching this repo's code.

## Architecture at a glance

Harmony is split into three independently-specified layers. A client only needs to implement the
subset it cares about.

| Layer | Transport | Purpose | Spec section |
|---|---|---|---|
| **REST API** | HTTPS | Accounts, forums, channels, message history, key publishing, invites | §2 |
| **Orchestration gateway** | WebSocket (`wss://`) | Presence, typing, live messages, voice-channel signaling, key rotation, ICE candidate exchange | §3 |
| **Audio transport** | WebSocket, always server-relayed | Encrypted voice bytes — benchmarking showed WS-over-TCP is fast enough, so audio never attempts P2P | §4.1 |
| **Video transport** | P2P UDP first, optional WebSocket fallback | Encrypted video bytes; a client only opens this connection if it wants to watch | §4.2 |

This repo currently contains a reference server (`backend/`) and two reference clients (`static/` web
client, `frontend/cpp/` CLI client). **The two reference clients are not yet compatible with each
other** — that's the gap this rewrite closes. The root-level `server.py` is a disposable PoC that
benchmarked raw audio over WebSocket-over-TCP; the result was that WS is fast enough, so audio always
goes over a server-relayed WebSocket in the real design (§4.1 of the protocol spec). The PoC itself will
still be deleted once the real gateway in `backend/` replaces it — it was never meant to be productionized
as-is, just to answer the latency question.

## Status

Early prototyping. See `HARMONY_PROTOCOL.md` for the target design and its own "not yet implemented"
list; see the bottom of this file for what's missing in `backend/` specifically.

## Encryption model (summary — full detail in the protocol spec)

Every user has a public/private keypair per forum they belong to. Public keys are published to the
server and readable by everyone in the forum; private keys never leave the client.

- **Voice channels**: each participant generates a symmetric sender key, encrypts it once per other
  participant with that participant's public key, and distributes it. Audio is then encrypted once with
  the symmetric key regardless of listener count. Keys rotate whenever the participant set changes.
  Audio always flows over a server-relayed WebSocket (a benchmark confirmed this is fast enough) — every
  voice channel is up to three WebSockets per client: orchestration/signaling, audio, and an optional
  video connection a client only opens if it wants to watch (see `HARMONY_PROTOCOL.md` §4).
- **Text channels**: unresolved. See `HARMONY_PROTOCOL.md` §5.2 for the open design problem (new-member
  history access vs. forward secrecy vs. server-side search) and the candidate approaches under
  consideration.
- **Video (P2P)**: unresolved encryption story for direct peer streams — see §5.3.

## Running it

TODO — depends on the backend rewrite landing.

## What's missing in `backend/` right now

This list is the living answer to "what do we still need to build," kept separate from the protocol
spec because it's implementation status, not design:

**Data model**
- No public-key storage (table + endpoints) — blocks all encryption work
- `role` is a free-text string, not an enforceable permission model
- No voice-channel presence/membership table
- No sessions/devices concept (multi-device needs per-device keys + delivery targets)
- No invites, no key-rotation/revocation records
- No message metadata needed for an E2E world: client-generated IDs, edited flag, reply refs, attachments
- No read-state, categories, or channel ordering

**API / server**
- `routes/users.py` is an empty stub; no auth, forums, channels, or messages routers exist
- No password hashing, no auth dependency, no CORS/middleware, no exception handling
- No migrations — schema changes require hand-editing `_init_tables`
- **No WebSocket gateway in `backend/` at all** — this is the actual core of the protocol and doesn't
  exist yet; the root `server.py` PoC is not it and will be discarded
- No signaling endpoint for ICE candidate exchange / UDP hole-punching coordination
- No tests, no secrets/config management beyond `TOKEN_EXPIRY_SECONDS`
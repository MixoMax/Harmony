// webrtc.js
//
// Full-mesh WebRTC: one RTCPeerConnection per remote participant. Fine up
// to the ~6-person cap this demo targets; a mesh's upload cost grows as
// O(N) per participant and its total connection count as O(N^2), which is
// exactly why we don't try to scale this design past a small group -- past
// that you'd want an SFU, a different architecture entirely.
//
// STUN only, deliberately (see backend/main.py docstring): no TURN relay,
// so a call between two peers both behind symmetric NAT may simply fail.

import {
  generateSenderKey,
  wrapSenderKeyForPeer,
  unwrapSenderKey,
  attachSenderEncryption,
  attachReceiverDecryption,
  supportsInsertableStreams,
  importPeerPublicKey,
} from "./crypto.js";

export const ICE_SERVERS = [
  { urls: "stun:stun.l.google.com:19302" },
  { urls: "stun:stun1.l.google.com:19302" },
];

// Resolution/bitrate ladder, highest quality first. scaleResolutionDownBy is
// relative to the camera's native capture resolution (we ask getUserMedia
// for 1080p60, so index 0 is "no downscale").
export const QUALITY_LADDER = [
  { label: "1080p60", scale: 1, maxFramerate: 60, maxBitrate: 4_000_000 },
  { label: "720p30", scale: 1.5, maxFramerate: 30, maxBitrate: 1_500_000 },
  { label: "480p30", scale: 2.25, maxFramerate: 30, maxBitrate: 800_000 },
  { label: "360p15", scale: 3, maxFramerate: 15, maxBitrate: 400_000 },
  { label: "240p15", scale: 4.5, maxFramerate: 15, maxBitrate: 200_000 },
  { label: "144p10", scale: 7.5, maxFramerate: 10, maxBitrate: 100_000 },
];

const ADAPT_INTERVAL_MS = 2500;
// Hysteresis: require a couple of consecutive bad/good readings before
// stepping, so one noisy sample doesn't yo-yo the resolution.
const STREAK_TO_STEP_DOWN = 2;
const STREAK_TO_STEP_UP = 3;
const BAD_LOSS_FRACTION = 0.04; // >4% packet loss over the interval
const BAD_RTT_SECONDS = 0.3; // >300ms round trip
const GOOD_LOSS_FRACTION = 0.01;
const GOOD_RTT_SECONDS = 0.15;

class Peer {
  constructor(peerId, displayName, localStream, hooks) {
    this.peerId = peerId;
    this.displayName = displayName;
    this.hooks = hooks; // { onRemoteTrack, onIceCandidate, onQualityChange }
    this.senderKey = null; // our own AES key, once generated
    this.peerSenderKey = null; // their AES key, once we unwrap it
    this.qualityIndex = 0;
    this._downStreak = 0;
    this._upStreak = 0;

    this.pc = new RTCPeerConnection({
      iceServers: ICE_SERVERS,
      encodedInsertableStreams: supportsInsertableStreams(),
    });

    this.videoSender = null;

    for (const track of localStream.getTracks()) {
      const sender = this.pc.addTrack(track, localStream);
      if (track.kind === "video") this.videoSender = sender;
      attachSenderEncryption(sender, () => this.senderKey);
    }

    this.pc.addEventListener("track", (event) => {
      const receiver = event.receiver;
      attachReceiverDecryption(receiver, () => this.peerSenderKey);
      this.hooks.onRemoteTrack(this.peerId, event.streams[0] ?? new MediaStream([event.track]));
    });

    this.pc.addEventListener("icecandidate", (event) => {
      if (event.candidate) this.hooks.onIceCandidate(this.peerId, event.candidate);
    });

    this.pc.addEventListener("connectionstatechange", () => {
      this.hooks.onConnectionStateChange?.(this.peerId, this.pc.connectionState);
    });

    this._adaptTimer = setInterval(() => this._adapt(), ADAPT_INTERVAL_MS);
  }

  setOwnSenderKey(key) {
    this.senderKey = key;
  }

  async setPeerSenderKey(wrappedBase64, myPrivateKey) {
    this.peerSenderKey = await unwrapSenderKey(wrappedBase64, myPrivateKey);
  }

  async makeOffer() {
    const offer = await this.pc.createOffer();
    await this.pc.setLocalDescription(offer);
    return offer;
  }

  async makeAnswer(remoteOffer) {
    await this.pc.setRemoteDescription(remoteOffer);
    const answer = await this.pc.createAnswer();
    await this.pc.setLocalDescription(answer);
    return answer;
  }

  async acceptAnswer(remoteAnswer) {
    await this.pc.setRemoteDescription(remoteAnswer);
  }

  async addIceCandidate(candidate) {
    try {
      await this.pc.addIceCandidate(candidate);
    } catch (e) {
      // benign if it arrives before the remote description is set in some
      // orderings; browsers queue candidates internally in most cases.
    }
  }

  async _applyQuality(index) {
    if (!this.videoSender) return;
    const level = QUALITY_LADDER[index];
    const params = this.videoSender.getParameters();
    if (!params.encodings || params.encodings.length === 0) params.encodings = [{}];
    params.encodings[0].scaleResolutionDownBy = level.scale;
    params.encodings[0].maxFramerate = level.maxFramerate;
    params.encodings[0].maxBitrate = level.maxBitrate;
    try {
      await this.videoSender.setParameters(params);
      this.qualityIndex = index;
      this.hooks.onQualityChange?.(this.peerId, level);
    } catch (e) {
      // setParameters can reject transiently mid-negotiation; next tick retries.
    }
  }

  async _adapt() {
    if (this.pc.connectionState !== "connected") return;
    const stats = await this.pc.getStats(this.videoSender?.track);

    let outboundLossFraction = null;
    let rttSeconds = null;

    stats.forEach((report) => {
      if (report.type === "remote-inbound-rtp" && report.kind === "video") {
        if (typeof report.fractionLost === "number") outboundLossFraction = report.fractionLost;
        if (typeof report.roundTripTime === "number") rttSeconds = report.roundTripTime;
      }
      if (report.type === "candidate-pair" && report.state === "succeeded" && report.nominated) {
        if (rttSeconds == null && typeof report.currentRoundTripTime === "number") {
          rttSeconds = report.currentRoundTripTime;
        }
      }
    });

    // No signal yet (e.g. connection just opened) -- nothing to react to.
    if (outboundLossFraction == null && rttSeconds == null) return;

    const isBad =
      (outboundLossFraction != null && outboundLossFraction > BAD_LOSS_FRACTION) ||
      (rttSeconds != null && rttSeconds > BAD_RTT_SECONDS);
    const isGood =
      (outboundLossFraction == null || outboundLossFraction < GOOD_LOSS_FRACTION) &&
      (rttSeconds == null || rttSeconds < GOOD_RTT_SECONDS);

    if (isBad) {
      this._downStreak++;
      this._upStreak = 0;
      if (this._downStreak >= STREAK_TO_STEP_DOWN && this.qualityIndex < QUALITY_LADDER.length - 1) {
        this._downStreak = 0;
        await this._applyQuality(this.qualityIndex + 1);
      }
    } else if (isGood) {
      this._upStreak++;
      this._downStreak = 0;
      if (this._upStreak >= STREAK_TO_STEP_UP && this.qualityIndex > 0) {
        this._upStreak = 0;
        await this._applyQuality(this.qualityIndex - 1);
      }
    } else {
      this._downStreak = 0;
      this._upStreak = 0;
    }
  }

  close() {
    clearInterval(this._adaptTimer);
    this.pc.close();
  }
}

export class MeshManager {
  constructor({ myId, localStream, signaling, myPrivateKey, hooks }) {
    this.myId = myId;
    this.localStream = localStream;
    this.signaling = signaling;
    this.myPrivateKey = myPrivateKey;
    this.hooks = hooks; // { onRemoteStream, onPeerLeft, onQualityChange, onConnectionStateChange }
    this.peers = new Map(); // peerId -> Peer
    this.peerPublicKeys = new Map(); // peerId -> CryptoKey (RSA), set by app.js before addPeer

    signaling.addEventListener("roster", (e) => this._onRoster(e.detail));
    signaling.addEventListener("peer-joined", (e) => this._onPeerJoined(e.detail));
    signaling.addEventListener("peer-left", (e) => this._onPeerLeft(e.detail));
    signaling.addEventListener("offer", (e) => this._onOffer(e.detail));
    signaling.addEventListener("answer", (e) => this._onAnswer(e.detail));
    signaling.addEventListener("ice-candidate", (e) => this._onIceCandidate(e.detail));
    signaling.addEventListener("sender-key", (e) => this._onSenderKey(e.detail));
  }

  async _onRoster({ payload }) {
    // Same "lower user_id initiates" rule as _onPeerJoined below -- applied
    // here too, or a freshly-joined participant and an existing one could
    // both send offers to each other at once (glare).
    for (const p of payload.participants) {
      const iInitiate = this.myId < p.user_id;
      await this._addPeer(p.user_id, p.display_name, p.public_key_pem, iInitiate);
    }
  }

  async _onPeerJoined({ payload }) {
    // The lower user_id always initiates, so both sides agree on who sends
    // the offer without needing full perfect-negotiation glare handling.
    const iInitiate = this.myId < payload.user_id;
    await this._addPeer(payload.user_id, payload.display_name, payload.public_key_pem, iInitiate);
  }

  async _addPeer(peerId, displayName, publicKeyPem, initiate) {
    if (this.peers.has(peerId)) return;
    const peerPublicKey = await importPeerPublicKey(publicKeyPem);
    this.peerPublicKeys.set(peerId, peerPublicKey);

    const peer = new Peer(peerId, displayName, this.localStream, {
      onRemoteTrack: (id, stream) => this.hooks.onRemoteStream(id, displayName, stream),
      onIceCandidate: (id, candidate) => this.signaling.sendTo(id, "ice-candidate", { candidate }),
      onQualityChange: (id, level) => this.hooks.onQualityChange?.(id, level),
      onConnectionStateChange: (id, state) => this.hooks.onConnectionStateChange?.(id, state),
    });
    this.peers.set(peerId, peer);

    // Generate + hand out our sender key to this peer right away (doesn't
    // need to wait on ICE/DTLS -- it travels over the signaling channel).
    const senderKey = await generateSenderKey();
    peer.setOwnSenderKey(senderKey);
    const wrapped = await wrapSenderKeyForPeer(senderKey, peerPublicKey);
    this.signaling.sendTo(peerId, "sender-key", { wrapped_key: wrapped });

    if (initiate) {
      const offer = await peer.makeOffer();
      this.signaling.sendTo(peerId, "offer", { sdp: offer });
    }
  }

  async _onOffer({ from, payload }) {
    const peer = this.peers.get(from);
    if (!peer) return; // sender-key/peer-joined races: peer should already exist by the time an offer arrives
    const answer = await peer.makeAnswer(payload.sdp);
    this.signaling.sendTo(from, "answer", { sdp: answer });
  }

  async _onAnswer({ from, payload }) {
    const peer = this.peers.get(from);
    if (peer) await peer.acceptAnswer(payload.sdp);
  }

  async _onIceCandidate({ from, payload }) {
    const peer = this.peers.get(from);
    if (peer) await peer.addIceCandidate(payload.candidate);
  }

  async _onSenderKey({ from, payload }) {
    const peer = this.peers.get(from);
    if (peer) await peer.setPeerSenderKey(payload.wrapped_key, this.myPrivateKey);
  }

  _onPeerLeft({ payload }) {
    const peer = this.peers.get(payload.user_id);
    if (peer) {
      peer.close();
      this.peers.delete(payload.user_id);
    }
    this.peerPublicKeys.delete(payload.user_id);
    this.hooks.onPeerLeft(payload.user_id);
  }

  closeAll() {
    for (const peer of this.peers.values()) peer.close();
    this.peers.clear();
  }
}

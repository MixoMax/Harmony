import {
  generateIdentityKeyPair,
  exportPublicKeyPem,
  supportsInsertableStreams,
} from "./crypto.js";
import { SignalingClient } from "./signaling.js";
import { MeshManager, QUALITY_LADDER } from "./webrtc.js";

// ---- DOM refs --------------------------------------------------------------

const joinScreen = document.getElementById("join-screen");
const meetingScreen = document.getElementById("meeting-screen");
const displayNameInput = document.getElementById("display-name");
const meetingCodeInput = document.getElementById("meeting-code");
const joinBtn = document.getElementById("join-btn");
const joinError = document.getElementById("join-error");
const keyStatusDot = document.getElementById("key-status-dot");
const keyStatusText = document.getElementById("key-status-text");
const meetingCodeLabel = document.getElementById("meeting-code-label");
const copyLinkBtn = document.getElementById("copy-link-btn");
const participantCountEl = document.getElementById("participant-count");
const videoGrid = document.getElementById("video-grid");
const toggleMicBtn = document.getElementById("toggle-mic");
const toggleCamBtn = document.getElementById("toggle-cam");
const leaveBtn = document.getElementById("leave-btn");

// ---- State ------------------------------------------------------------------

const myId = crypto.randomUUID();
let myKeyPair = null;
let myPublicKeyPem = null;
let localStream = null;
let signaling = null;
let mesh = null;
const tiles = new Map(); // peerId -> { el, videoEl, ladderEl, textEl }

// ---- Boot: generate our RSA identity as soon as the page loads -----------

(async function boot() {
  const params = new URLSearchParams(location.search);
  const m = params.get("m");
  if (m) meetingCodeInput.value = m;

  myKeyPair = await generateIdentityKeyPair();
  myPublicKeyPem = await exportPublicKeyPem(myKeyPair.publicKey);

  keyStatusDot.classList.remove("dot-pending");
  keyStatusDot.classList.add("dot-ready");
  keyStatusText.textContent = "RSA identity ready (2048-bit)";

  if (!supportsInsertableStreams()) {
    keyStatusText.textContent +=
      " — note: this browser lacks Insertable Streams, media will NOT be end-to-end encrypted";
  }

  joinBtn.disabled = false;
})();

joinBtn.disabled = true;

// ---- Join flow ---------------------------------------------------------------

joinBtn.addEventListener("click", async () => {
  joinError.hidden = true;
  const displayName = displayNameInput.value.trim();
  if (!displayName) {
    showJoinError("enter a name first");
    return;
  }

  joinBtn.disabled = true;
  joinBtn.textContent = "connecting…";

  try {
    let meetingId = meetingCodeInput.value.trim();
    if (!meetingId) {
      const res = await fetch("/api/meetings", { method: "POST" });
      if (!res.ok) throw new Error("could not create meeting");
      meetingId = (await res.json()).meeting_id;
    }

    localStream = await navigator.mediaDevices.getUserMedia({
      video: { width: { ideal: 1920 }, height: { ideal: 1080 }, frameRate: { ideal: 60 } },
      audio: true,
    });

    signaling = new SignalingClient(meetingId, myId);
    await signaling.connect();
    signaling.addEventListener("error", (e) => showJoinError(e.detail.payload.message));
    signaling.addEventListener("closed", onSignalingClosed);

    mesh = new MeshManager({
      myId,
      localStream,
      signaling,
      myPrivateKey: myKeyPair.privateKey,
      hooks: {
        onRemoteStream: (id, name, stream) => upsertTile(id, name, stream, false),
        onPeerLeft: (id) => removeTile(id),
        onQualityChange: (id, level) => updateTileQuality(id, level),
        onConnectionStateChange: (id, state) => updateTileStatus(id, state),
      },
    });

    signaling.join(displayName, myPublicKeyPem);
    signaling.addEventListener("roster", () => refreshParticipantCount());
    signaling.addEventListener("peer-joined", () => refreshParticipantCount());
    signaling.addEventListener("peer-left", () => refreshParticipantCount());

    enterMeetingScreen(meetingId, displayName);
  } catch (err) {
    console.error(err);
    showJoinError(err.message || "failed to join meeting");
    joinBtn.disabled = false;
    joinBtn.textContent = "join / create meeting";
  }
});

function showJoinError(message) {
  joinError.textContent = message;
  joinError.hidden = false;
  joinBtn.disabled = false;
  joinBtn.textContent = "join / create meeting";
}

function onSignalingClosed() {
  // Server connection dropped -- surface it rather than silently freezing.
  participantCountEl.textContent = "disconnected";
}

// ---- Meeting screen ------------------------------------------------------

function enterMeetingScreen(meetingId, displayName) {
  joinScreen.hidden = true;
  meetingScreen.hidden = false;
  meetingCodeLabel.textContent = meetingId;

  upsertTile(myId, `${displayName} (you)`, localStream, true);
  refreshParticipantCount();
}

function refreshParticipantCount() {
  const count = 1 + (mesh ? mesh.peers.size : 0);
  participantCountEl.textContent = `${count} in meeting`;
}

copyLinkBtn.addEventListener("click", async () => {
  const url = `${location.origin}${location.pathname}?m=${meetingCodeLabel.textContent}`;
  try {
    await navigator.clipboard.writeText(url);
    copyLinkBtn.textContent = "copied";
    setTimeout(() => (copyLinkBtn.textContent = "copy link"), 1500);
  } catch {
    prompt("copy this link:", url);
  }
});

// ---- Tiles -----------------------------------------------------------------

function upsertTile(peerId, name, stream, isSelf) {
  let tile = tiles.get(peerId);
  if (!tile) {
    const el = document.createElement("div");
    el.className = "tile";
    el.innerHTML = `
      <video autoplay playsinline ${isSelf ? "muted" : ""}></video>
      <div class="tile-status" hidden></div>
      <div class="tile-label">
        <div class="quality-ladder" hidden>
          ${Array.from({ length: QUALITY_LADDER.length }).map(() => '<div class="bar"></div>').join("")}
        </div>
        <span class="tile-name"></span>
        <span class="tile-quality-text"></span>
      </div>
    `;
    videoGrid.appendChild(el);
    tile = {
      el,
      videoEl: el.querySelector("video"),
      statusEl: el.querySelector(".tile-status"),
      ladderEl: el.querySelector(".quality-ladder"),
      nameEl: el.querySelector(".tile-name"),
      qualityTextEl: el.querySelector(".tile-quality-text"),
    };
    tiles.set(peerId, tile);
  }
  tile.nameEl.textContent = name;
  tile.videoEl.srcObject = stream;
  if (!isSelf) tile.ladderEl.hidden = false;
}

function removeTile(peerId) {
  const tile = tiles.get(peerId);
  if (!tile) return;
  tile.el.remove();
  tiles.delete(peerId);
  refreshParticipantCount();
}

function updateTileQuality(peerId, level) {
  const tile = tiles.get(peerId);
  if (!tile) return;
  const index = QUALITY_LADDER.indexOf(level);
  const bars = tile.ladderEl.querySelectorAll(".bar");
  // ladder is worst-to-best left-to-right; QUALITY_LADDER is best-to-worst,
  // so light up (length - index) bars from the left.
  const litCount = QUALITY_LADDER.length - index;
  bars.forEach((bar, i) => bar.classList.toggle("lit", i < litCount));

  tile.ladderEl.classList.remove("warn", "bad");
  if (index >= QUALITY_LADDER.length - 2) tile.ladderEl.classList.add("bad");
  else if (index >= Math.ceil(QUALITY_LADDER.length / 2)) tile.ladderEl.classList.add("warn");

  tile.qualityTextEl.textContent = level.label;
}

function updateTileStatus(peerId, state) {
  const tile = tiles.get(peerId);
  if (!tile) return;
  if (state === "connected") {
    tile.statusEl.hidden = true;
  } else {
    tile.statusEl.hidden = false;
    tile.statusEl.textContent = state;
  }
}

// ---- Controls ----------------------------------------------------------------

toggleMicBtn.addEventListener("click", () => {
  const track = localStream?.getAudioTracks()[0];
  if (!track) return;
  track.enabled = !track.enabled;
  toggleMicBtn.dataset.active = String(track.enabled);
});

toggleCamBtn.addEventListener("click", () => {
  const track = localStream?.getVideoTracks()[0];
  if (!track) return;
  track.enabled = !track.enabled;
  toggleCamBtn.dataset.active = String(track.enabled);
});

leaveBtn.addEventListener("click", () => {
  mesh?.closeAll();
  signaling?.close();
  localStream?.getTracks().forEach((t) => t.stop());
  location.href = location.pathname;
});

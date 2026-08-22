// crypto.js
//
// Two layers of crypto happen here:
//
// 1. RSA-OAEP keypair, one per participant, used ONLY to wrap/unwrap the
//    small AES symmetric "sender keys" so they can be handed out over the
//    (untrusted-for-media, but fine-for-small-signaling) server relay.
//
// 2. AES-256-GCM "sender keys": each participant makes ONE for themselves
//    per meeting, and it's what actually encrypts their outgoing audio/video
//    frames. Every other participant gets that key via RSA wrapping, so in
//    an N-person meeting everyone ends up holding N AES keys (their own +
//    one per remote peer), exactly as specced. The frame encryption itself
//    runs through WebRTC's Insertable Streams API, so it happens on encoded
//    (already-compressed) media just before it leaves the RTP stack -- the
//    server and any network in between only ever see ciphertext.

const RSA_PARAMS = {
  name: "RSA-OAEP",
  modulusLength: 2048,
  publicExponent: new Uint8Array([1, 0, 1]),
  hash: "SHA-256",
};

export function supportsInsertableStreams() {
  return (
    typeof RTCRtpSender !== "undefined" &&
    "createEncodedStreams" in RTCRtpSender.prototype
  );
}

// ---- RSA identity keypair -------------------------------------------------

export async function generateIdentityKeyPair() {
  return crypto.subtle.generateKey(RSA_PARAMS, true, ["wrapKey", "unwrapKey"]);
}

export async function exportPublicKeyPem(publicKey) {
  const spki = await crypto.subtle.exportKey("spki", publicKey);
  return arrayBufferToPem(spki, "PUBLIC KEY");
}

export async function importPeerPublicKey(pem) {
  const der = pemToArrayBuffer(pem);
  return crypto.subtle.importKey("spki", der, RSA_PARAMS, true, ["wrapKey"]);
}

// ---- AES sender key ---------------------------------------------------------

export async function generateSenderKey() {
  return crypto.subtle.generateKey({ name: "AES-GCM", length: 256 }, true, [
    "encrypt",
    "decrypt",
  ]);
}

// Wrap our AES sender key with a specific peer's RSA public key. The result
// is safe to hand to the signaling server -- only that peer's private key
// can unwrap it.
export async function wrapSenderKeyForPeer(senderKey, peerPublicKey) {
  const wrapped = await crypto.subtle.wrapKey(
    "raw",
    senderKey,
    peerPublicKey,
    RSA_PARAMS
  );
  return arrayBufferToBase64(wrapped);
}

// Unwrap a sender key someone sent us, using our own RSA private key.
export async function unwrapSenderKey(wrappedBase64, myPrivateKey) {
  const wrapped = base64ToArrayBuffer(wrappedBase64);
  return crypto.subtle.unwrapKey(
    "raw",
    wrapped,
    myPrivateKey,
    RSA_PARAMS,
    { name: "AES-GCM", length: 256 },
    true,
    ["encrypt", "decrypt"]
  );
}

// ---- Per-frame E2EE via Insertable Streams --------------------------------
//
// Each encoded frame gets a fresh random 12-byte IV, AES-GCM-encrypted, with
// the IV appended after the ciphertext so the receiver can split it back
// out. Encryption happens in the sender's own RTCRtpSender pipeline (using
// the sender's own key); decryption happens in the matching receiver's
// pipeline on the OTHER end (using that sender's key, obtained via RSA
// wrap/unwrap above). Because this is a full mesh (one RTCPeerConnection per
// remote peer, not an SFU), each connection maps 1:1 to a single remote
// identity, so there's no ambiguity about "whose key do I use here".

const IV_LENGTH = 12;

function makeEncryptTransform(getKey) {
  return new TransformStream({
    async transform(encodedFrame, controller) {
      const key = getKey();
      if (!key) {
        // Key not established yet (e.g. very first frames before the
        // sender-key exchange completes) -- drop rather than send plaintext.
        return;
      }
      const iv = crypto.getRandomValues(new Uint8Array(IV_LENGTH));
      const plaintext = new Uint8Array(encodedFrame.data);
      const ciphertext = await crypto.subtle.encrypt(
        { name: "AES-GCM", iv },
        key,
        plaintext
      );
      const out = new Uint8Array(ciphertext.byteLength + IV_LENGTH);
      out.set(new Uint8Array(ciphertext), 0);
      out.set(iv, ciphertext.byteLength);
      encodedFrame.data = out.buffer;
      controller.enqueue(encodedFrame);
    },
  });
}

function makeDecryptTransform(getKey) {
  return new TransformStream({
    async transform(encodedFrame, controller) {
      const key = getKey();
      const buf = new Uint8Array(encodedFrame.data);
      if (!key || buf.byteLength <= IV_LENGTH) {
        return; // can't decrypt yet / malformed -- drop the frame
      }
      const iv = buf.slice(buf.byteLength - IV_LENGTH);
      const ciphertext = buf.slice(0, buf.byteLength - IV_LENGTH);
      try {
        const plaintext = await crypto.subtle.decrypt(
          { name: "AES-GCM", iv },
          key,
          ciphertext
        );
        encodedFrame.data = plaintext;
        controller.enqueue(encodedFrame);
      } catch (e) {
        // Bad key / desynced stream -- drop the frame rather than crash
        // the pipeline; the next good frame will recover.
      }
    },
  });
}

// getOwnKey / getPeerKey are zero-arg functions so the pipeline always reads
// the *current* key even if it's set slightly after the transform is wired
// up (there's an unavoidable short race between "peer connection created"
// and "sender key received").
export function attachSenderEncryption(rtcSender, getOwnKey) {
  if (!supportsInsertableStreams()) return;
  const { readable, writable } = rtcSender.createEncodedStreams();
  readable.pipeThrough(makeEncryptTransform(getOwnKey)).pipeTo(writable);
}

export function attachReceiverDecryption(rtcReceiver, getPeerKey) {
  if (!supportsInsertableStreams()) return;
  const { readable, writable } = rtcReceiver.createEncodedStreams();
  readable.pipeThrough(makeDecryptTransform(getPeerKey)).pipeTo(writable);
}

// ---- encoding helpers -------------------------------------------------------

function arrayBufferToBase64(buf) {
  let binary = "";
  const bytes = new Uint8Array(buf);
  for (let i = 0; i < bytes.byteLength; i++) binary += String.fromCharCode(bytes[i]);
  return btoa(binary);
}

function base64ToArrayBuffer(b64) {
  const binary = atob(b64);
  const bytes = new Uint8Array(binary.length);
  for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
  return bytes.buffer;
}

function arrayBufferToPem(buf, label) {
  const b64 = arrayBufferToBase64(buf);
  const lines = b64.match(/.{1,64}/g).join("\n");
  return `-----BEGIN ${label}-----\n${lines}\n-----END ${label}-----`;
}

function pemToArrayBuffer(pem) {
  const b64 = pem.replace(/-----[^-]+-----/g, "").replace(/\s+/g, "");
  return base64ToArrayBuffer(b64);
}

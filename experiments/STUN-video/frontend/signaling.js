// signaling.js
//
// Thin wrapper around the signaling WebSocket. Knows nothing about WebRTC
// or crypto -- just sends/receives the JSON envelope the backend expects
// and dispatches incoming messages by "type" to registered handlers.

export class SignalingClient extends EventTarget {
  constructor(meetingId, userId) {
    super();
    this.meetingId = meetingId;
    this.userId = userId;
    this.ws = null;
  }

  connect() {
    const proto = location.protocol === "https:" ? "wss" : "ws";
    const url = `${proto}://${location.host}/ws/${this.meetingId}/${this.userId}`;
    this.ws = new WebSocket(url);

    return new Promise((resolve, reject) => {
      this.ws.addEventListener("open", () => resolve(), { once: true });
      this.ws.addEventListener(
        "error",
        (e) => reject(new Error("signaling connection failed")),
        { once: true }
      );
      this.ws.addEventListener("message", (event) => {
        let msg;
        try {
          msg = JSON.parse(event.data);
        } catch {
          return;
        }
        this.dispatchEvent(new CustomEvent(msg.type, { detail: msg }));
      });
      this.ws.addEventListener("close", () => {
        this.dispatchEvent(new CustomEvent("closed"));
      });
    });
  }

  join(displayName, publicKeyPem) {
    this._send({
      type: "join",
      payload: { display_name: displayName, public_key_pem: publicKeyPem },
    });
  }

  sendTo(toUserId, type, payload) {
    this._send({ type, to: toUserId, payload });
  }

  _send(obj) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(obj));
    }
  }

  close() {
    this.ws?.close();
  }
}

// UI Elements
const screenJoin = document.getElementById('join-screen');
const screenRoom = document.getElementById('room-screen');
const usernameInput = document.getElementById('username');
const roomInput = document.getElementById('room');
const btnJoin = document.getElementById('btn-join');
const btnLeave = document.getElementById('btn-leave');
const btnMic = document.getElementById('btn-mic');
const currentRoomSpan = document.getElementById('current-room');
const logDiv = document.getElementById('log');

// State
let ws = null;
let audioCtx = null;
let micStream = null;
let scriptProcessor = null;
let mediaStreamSource = null;
let isMicActive = false;
let nextPlayTime = 0;

function log(msg) {
    const el = document.createElement('div');
    el.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
    logDiv.appendChild(el);
    logDiv.scrollTop = logDiv.scrollHeight;
}

// --- Connection Logic ---

btnJoin.addEventListener('click', () => {
    const username = usernameInput.value.trim();
    const room = roomInput.value.trim();

    if (!username || !room) {
        alert("Please enter a username and room ID.");
        return;
    }

    const protocol_prefix = window.location.protocol === 'https:' ? 'wss' : 'ws';

    const wsUrl = `${protocol_prefix}://${window.location.host}/ws/${encodeURIComponent(room)}/${encodeURIComponent(username)}`;
    ws = new WebSocket(wsUrl);
    ws.binaryType = "arraybuffer"; // Important: Handle raw bytes directly

    ws.onopen = () => {
        log(`Connected to room: ${room}`);
        screenJoin.classList.remove('active');
        screenRoom.classList.active = true;
        screenRoom.classList.add('active');
        currentRoomSpan.textContent = room;
    };

    ws.onmessage = (event) => {
        // Event data is the ArrayBuffer from the server
        console.log("Received audio packet of size:", event.data.byteLength);
        playAudioPacket(event.data);
    };

    ws.onclose = () => {
        log("Disconnected from server.");
        leaveRoom();
    };

    ws.onerror = (err) => {
        log("WebSocket error occurred.");
        console.error(err);
    };
});

btnLeave.addEventListener('click', () => leaveRoom());

function leaveRoom() {
    if (ws) ws.close();
    stopMic();
    screenRoom.classList.remove('active');
    screenJoin.classList.add('active');
    logDiv.innerHTML = '';
}

// --- Audio Capture and Playback Logic ---

btnMic.addEventListener('click', async () => {
    if (isMicActive) {
        stopMic();
    } else {
        await startMic();
    }
});

async function startMic() {
    try {
        // Enforce 48kHz sample rate so everyone operates on the same time domain
        audioCtx = new (window.AudioContext || window.webkitAudioContext)({ sampleRate: 48000 });
        
        micStream = await navigator.mediaDevices.getUserMedia({ audio: true, video: false });
        mediaStreamSource = audioCtx.createMediaStreamSource(micStream);
        
        // 2048 buffer size = ~42.6ms per packet at 48kHz.
        // ScriptProcessor is deprecated but remains the easiest standard way to grab raw PCM bytes in a simple JS file.
        scriptProcessor = audioCtx.createScriptProcessor(2048, 1, 1);
        
        scriptProcessor.onaudioprocess = (event) => {
            if (!ws || ws.readyState !== WebSocket.OPEN) return;
            
            // Get raw Float32 PCM data
            const pcmData = event.inputBuffer.getChannelData(0);
            
            // Send the underlying ArrayBuffer over the WebSocket
            ws.send(pcmData.slice().buffer); 
        };

        mediaStreamSource.connect(scriptProcessor);
        scriptProcessor.connect(audioCtx.destination);

        isMicActive = true;
        btnMic.textContent = "Stop Microphone";
        btnMic.classList.remove('success');
        btnMic.classList.add('danger');
        log("Microphone activated (Sending ~42ms packets).");

    } catch (err) {
        log("Failed to access microphone.");
        console.error(err);
    }
}

function stopMic() {
    if (scriptProcessor) scriptProcessor.disconnect();
    if (mediaStreamSource) mediaStreamSource.disconnect();
    if (micStream) micStream.getTracks().forEach(track => track.stop());
    
    isMicActive = false;
    btnMic.textContent = "Start Microphone";
    btnMic.classList.remove('danger');
    btnMic.classList.add('success');
    log("Microphone stopped.");
}

function playAudioPacket(arrayBuffer) {
    if (!audioCtx) {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)({ sampleRate: 48000 });
    }

    // Convert bytes back to Float32 array
    const f32Data = new Float32Array(arrayBuffer);
    
    // Create an audio buffer to hold the 50ms chunk
    const audioBuffer = audioCtx.createBuffer(1, f32Data.length, audioCtx.sampleRate);
    audioBuffer.getChannelData(0).set(f32Data);

    // Setup source node for playback
    const source = audioCtx.createBufferSource();
    source.buffer = audioBuffer;
    source.connect(audioCtx.destination);

    // Schedule playback continuously to avoid stuttering gaps
    const currentTime = audioCtx.currentTime;
    if (nextPlayTime < currentTime) {
        nextPlayTime = currentTime + 0.05; // Slight buffer if we fell behind
    }

    source.start(nextPlayTime);
    nextPlayTime += audioBuffer.duration;
}
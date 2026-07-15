from fastapi import FastAPI, websockets
from fastapi.responses import FileResponse, JSONResponse
import uuid
import os
import asyncio

app = FastAPI()

class WebSocketManager:
    rooms: dict[str, list[tuple[websockets.WebSocket, str]]] = {} # room_id: list of tuples (websocket, username)

    def __init__(self):
        self.rooms = {}

    async def connect(self, websocket: websockets.WebSocket, room_id: str, username: str):
        if room_id not in self.rooms:
            self.rooms[room_id] = []
        self.rooms[room_id].append((websocket, username))
        await websocket.accept()

    async def disconnect(self, websocket: websockets.WebSocket, room_id: str):
        if room_id in self.rooms:
            self.rooms[room_id] = [(ws, user) for ws, user in self.rooms[room_id] if ws != websocket]
            if not self.rooms[room_id]:  # If the room is empty, remove it
                del self.rooms[room_id]

    async def broadcast_bytes(self, room_id: str, data: bytes, excluded_user: str | None = None):
        if room_id in self.rooms:
            for websocket, user in self.rooms[room_id]:
                if user != excluded_user:
                    print(f"sending {len(data)} bytes to {user} in room {room_id}")
                    asyncio.create_task(websocket.send_bytes(data))

    def get_room_data(self):
        return {room_id: [user for _, user in users] for room_id, users in self.rooms.items()}

wsm = WebSocketManager()

@app.get("/api/v1/rooms")
async def get_rooms():
    return JSONResponse(content=wsm.get_room_data())

@app.websocket("/ws/{room_id}/{username}")
async def websocket_endpoint(websocket: websockets.WebSocket, room_id: str, username: str):
    await wsm.connect(websocket, room_id, username)
    try:
        while True:
            data = await websocket.receive_bytes()
            await wsm.broadcast_bytes(room_id, data, excluded_user=username)
    except websockets.WebSocketDisconnect:
        await wsm.disconnect(websocket, room_id)

@app.get("/{path:path}")
async def serve_file(path: str):
    if path == "":
        path = "index.html"

    file_path = os.path.join("static", path)
    if os.path.isfile(file_path) and os.path.exists(file_path):
        return FileResponse(file_path)
    else:
        return JSONResponse(content={"error": "File not found"}, status_code=404)


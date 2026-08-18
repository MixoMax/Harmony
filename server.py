import asyncio
import os
import uuid

from fastapi import FastAPI, websockets
from fastapi.responses import FileResponse, JSONResponse

app = FastAPI()


class User:
    def __init__(self, name: str, websocket: websockets.WebSocket, udpPort: int):
        self.name: str = name
        self.websocket: websockets.WebSocket = websocket
        self.udpPort: int = udpPort
        self.id: str = str(uuid.uuid4())

    def to_json(self):
        return {"id": self.id, "name": self.name, "ip": self.websocket.client.host, "port": self.udpPort}


class Room:
    def __init__(self, name):
        self.name = name
        self.users: list[User] = []

    def to_json(self):
        return {"name": self.name, "users": [user.to_json() for user in self.users]}


class WebSocketManager:
    rooms: dict[str, Room] = {}  # room_id: list of tuples (websocket, username)

    def __init__(self):
        self.rooms = {}

    async def connect(self, room_id: str, user: User) -> Room:
        print(f"User \"{user.name}\" connected to room \"{room_id}\"")
        if room_id not in self.rooms:
            self.rooms[room_id] = Room(room_id)
        room: Room = self.rooms[room_id]

        for other in room.users:
            await other.websocket.send_json({"type": "user_joined", "user": user.to_json()})

        await user.websocket.accept()
        await user.websocket.send_json({"type": "room_joined", "room": room.to_json()})

        room.users.append(user)
        return room

    async def disconnect(self, room: Room, user: User):
        print(f"User \"{user.name}\" disconnected from room \"{room.name}\"")
        room.users = [u for u in room.users if u != user]

        for other_user in room.users:
            asyncio.create_task(other_user.websocket.send_json({"type": "user_left", "user": user.to_json()}))

        if not room.users:  # If the room is empty, remove it
            del self.rooms[room.name]

    async def broadcast_bytes(self, room: Room, data: bytes, excluded_user: User | None = None):
        for user in room.users:
            if user != excluded_user:
                print(f"sending {len(data)} bytes to {user.name} in room {room.name}")
                asyncio.create_task(user.websocket.send_bytes(data))

    # HTTP Requests
    def get_room_data(self):
        print(self.rooms)
        return [room.to_json() for room in self.rooms.values()]


wsm = WebSocketManager()


@app.get("/api/v1/rooms")
async def get_rooms():
    return JSONResponse(content=wsm.get_room_data())


@app.websocket("/ws/{room_id}/{username}/{udpPort}")
async def websocket_endpoint(websocket: websockets.WebSocket, room_id: str, username: str, udpPort: int):
    user = User(username, websocket, udpPort)
    room: Room = await wsm.connect(room_id=room_id, user=user)
    try:
        while True:
            data = await websocket.receive_bytes()
            await wsm.broadcast_bytes(room=room, data=data, excluded_user=user)
    except websockets.WebSocketDisconnect:
        await wsm.disconnect(room=room, user=user)


@app.get("/{path:path}")
async def serve_file(path: str):
    if path == "":
        path = "index.html"

    file_path = os.path.join("static", path)
    if os.path.isfile(file_path) and os.path.exists(file_path):
        return FileResponse(file_path)
    else:
        return JSONResponse(content={"error": "File not found"}, status_code=404)

import websockets
import asyncio
from playsound3 import playsound

sound_file = "./beep.mp3"



async def main():
    with open(sound_file, "rb") as f:
        sound_bytes = f.read()

    async with websockets.connect("ws://188.245.231.101:9999/ws") as websocket:
        await websocket.send(sound_bytes)
        message = await websocket.recv()

    with open("./received_audio.mp3", "wb") as f:
        f.write(message)

    playsound("./received_audio.mp3")

if __name__ == "__main__":
    asyncio.run(main())
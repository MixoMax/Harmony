import websockets
import asyncio
from playsound3 import playsound
import time

sound_file = "./beep.mp3"



async def main():
    t_start = time.time()

    with open(sound_file, "rb") as f:
        sound_bytes = f.read()

    t_read_file = time.time()

    async with websockets.connect("ws://188.245.231.101:9999/ws") as websocket:
        await websocket.send(sound_bytes)
        message = await websocket.recv()

    t_upload_file = time.time()

    with open("./received_audio.mp3", "wb") as f:
        f.write(message)

    t_download_file = time.time()

    print(f"Loading file: {round((t_read_file - t_start) * 1000)} ms")
    print(f"Uploading file: {round((t_upload_file - t_read_file) * 1000)} ms")
    print(f"Downloading file: {round((t_download_file - t_upload_file) * 1000)} ms")
    print(f"Total time: {round((t_download_file - t_start) * 1000)} ms")

    playsound("./received_audio.mp3")

if __name__ == "__main__":
    asyncio.run(main())
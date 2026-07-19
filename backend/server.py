from fastapi import FastAPI, staticfiles

from routes import ROUTERS

app = FastAPI()

for router in ROUTERS:
    app.include_router(router)


app.mount("/static", staticfiles.StaticFiles(directory="static"), name="static")
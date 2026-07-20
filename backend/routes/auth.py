from fastapi import APIRouter, Depends
from dependencies import get_db


router = APIRouter(prefix="/api/v1/auth", tags=["auth"])

@router.post("/register")
async def register_user():
    pass
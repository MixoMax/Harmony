from fastapi import Depends, HTTPException, Header

from database import Database
from classes import Channel, Channel, Forum, User, UserForumRelationship



db = Database(db_path="data/database.db")

def get_db() -> Database:
    return db

def get_token_from_header(authorization: str = Header(...)) -> str:
    if not authorization.startswith("Bearer "):
        raise HTTPException(status_code=401, detail="Invalid authorization header")
    return authorization.removeprefix("Bearer ")
    
def get_requesting_user(
    token: str = Depends(get_token_from_header),
    db: Database = Depends(get_db),
) -> User:
    user = db.validate_token(token)
    if not user:
        raise HTTPException(status_code=401, detail="Invalid token")
    return user

def get_forum(
    forum_id: str,
    db: Database = Depends(get_db),
) -> Forum:
    forum = db.read_forum_by_id(forum_id)
    if not forum:
        raise HTTPException(status_code=404, detail="Forum not found")
    return forum

def require_forum_membership(
    forum: Forum = Depends(get_forum),
    user: User = Depends(get_requesting_user),
    db: Database = Depends(get_db),
) -> Forum:
    is_in_forum = db.is_user_in_forum(forum.id, user.id)
    if not is_in_forum:
        raise HTTPException(status_code=403, detail="User not a member of this forum")
    return forum

def get_forum_role
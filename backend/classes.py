from typing import Literal

from pydantic import BaseModel
import datetime

class User(BaseModel):
    id: str
    username: str
    email: str
    password_hash: str
    created_at: datetime.datetime

    @staticmethod
    def from_row(row: tuple):
        return User(
            id=row[0],
            username=row[1],
            email=row[2],
            password_hash=row[3],
            created_at=datetime.datetime.fromisoformat(row[4])
        )

    def to_row(self):
        return (self.id, self.username, self.email, self.password_hash, self.created_at.isoformat())

    def __hash__(self):
        return hash(self.id)

class Forum(BaseModel):
    id: str
    name: str
    description: str | None = None
    created_at: datetime.datetime
    owner_user_id: str #->User.id

    @staticmethod
    def from_row(row: tuple):
        return Forum(
            id=row[0],
            name=row[1],
            description=row[2],
            created_at=datetime.datetime.fromisoformat(row[3]),
            owner_user_id=row[4]
        )

    def to_row(self):
        return (self.id, self.name, self.description, self.created_at.isoformat(), self.owner_user_id)

    def __hash__(self):
        return hash(self.id)

class Channel(BaseModel):
    id: str
    type: Literal["text", "voice"]
    name: str | None = None
    description: str | None = None
    created_at: datetime.datetime
    forum_id: str #->Forum.id
    use_encryption: bool = False
    is_public: bool = True

    @staticmethod
    def from_row(row: tuple):
        return Channel(
            id=row[0],
            type=row[1],
            name=row[2],
            description=row[3],
            created_at=datetime.datetime.fromisoformat(row[4]),
            forum_id=row[5],
            use_encryption=row[6],
            is_public=row[7]
        )

    def to_row(self):
        return (self.id, self.type, self.name, self.description, self.created_at.isoformat(), self.forum_id, self.use_encryption, self.is_public)

    def __hash__(self):
        return hash(self.id)

class Message(BaseModel):
    id: str
    content: str
    created_at: datetime.datetime
    channel_id: str #->Channel.id
    user_id: str #->User.id

    @staticmethod
    def from_row(row: tuple):
        return Message(
            id=row[0],
            content=row[1],
            created_at=datetime.datetime.fromisoformat(row[2]),
            channel_id=row[3],
            user_id=row[4]
        )

    def to_row(self):
        return (self.id, self.content, self.created_at.isoformat(), self.channel_id, self.user_id)

    def __hash__(self):
        return hash(self.id)

class ForumRole(BaseModel):
    id: str
    name: str
    data: dict

    @staticmethod
    def from_row(row: tuple):
        return ForumRole(
            id=row[0],
            name=row[1],
            data=row[2]
        )

    def to_row(self):
        return (self.id, self.name, self.data)

    def __hash__(self):
        return hash(self.id)

class UserForumRelationship(BaseModel):
    user_id: str #->User.id
    forum_id: str #->Forum.id
    role_id: str #->ForumRole.id

    @staticmethod
    def from_row(row: tuple):
        return UserForumRelationship(
            user_id=row[0],
            forum_id=row[1],
            role_id=row[2]
        )

    def to_row(self):
        return (self.user_id, self.forum_id, self.role_id)

    def __hash__(self):
        return hash((self.user_id, self.forum_id, self.role_id))

class ChannelRoleRequirement(BaseModel):
    channel_id: str #->Channel.id
    role_id: str #->ForumRole.id

    @staticmethod
    def from_row(row: tuple):
        return ChannelRoleRequirement(
            channel_id=row[0],
            role_id=row[1]
        )

    def to_row(self):
        return (self.channel_id, self.role_id)

    def __hash__(self):
        return hash((self.channel_id, self.role_id))

class Token(BaseModel):
    id: str
    user_id: str #->User.id
    created_at: datetime.datetime
    expires_at: datetime.datetime

    @staticmethod
    def from_row(row: tuple):
        return Token(
            id=row[0],
            user_id=row[1],
            created_at=datetime.datetime.fromisoformat(row[2]),
            expires_at=datetime.datetime.fromisoformat(row[3])
        )

    def to_row(self):
        return (self.id, self.user_id, self.created_at.isoformat(), self.expires_at.isoformat())

    def __hash__(self):
        return hash(self.id)

class RSAPublicKey(BaseModel):
    id: str
    user_id: str #->User.id
    n: int
    e: int

    @staticmethod
    def from_row(row: tuple):
        return RSAPublicKey(
            id=row[0],
            user_id=row[1],
            n=row[2],
            e=row[3]
        )

    def to_row(self):
        return (self.id, self.user_id, self.n, self.e)

    def __hash__(self):
        return hash(self.id)

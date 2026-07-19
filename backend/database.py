import sqlite3
import threading
import uuid

from classes import User, Forum, Channel, Message, UserForumRelationship, Token
from config import TOKEN_EXPIRY_SECONDS
import datetime

class Database:
    def __init__(self, db_path: str):
        self._db_path = db_path
        self._local = threading.local()
        self._init_tables()

    @property
    def connection(self):
        if not hasattr(self._local, 'connection'):
            self._local.connection = sqlite3.connect(self._db_path, check_same_thread=False)
            self._local.connection.execute('PRAGMA journal_mode=WAL;')
            self._local.connection.execute('PRAGMA busy_timeout=5000;')
            self._local.connection.execute('PRAGMA foreign_keys = ON;')

        return self._local.connection

    def _execute(self, cmd: str, params: tuple = ()):
        cursor = self.connection.cursor()
        cursor.execute(cmd, params)
        self.connection.commit()
        cursor.close()

    def _execute_rowcount(self, cmd: str, params: tuple = ()):
        cursor = self.connection.cursor()
        cursor.execute(cmd, params)
        self.connection.commit()
        rowcount = cursor.rowcount
        cursor.close()

        return rowcount

    def _fetchone(self, cmd: str, params: tuple = ()):
        cursor = self.connection.cursor()
        cursor.execute(cmd, params)
        row = cursor.fetchone()
        cursor.close()

        return row

    def _fetchall(self, cmd: str, params: tuple = ()):
        cursor = self.connection.cursor()
        cursor.execute(cmd, params)
        rows = cursor.fetchall()
        cursor.close()

        return rows

    def _init_tables(self):
        table_cmds = []

        table_cmds.append("""
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            username TEXT NOT NULL,
            email TEXT NOT NULL,
            password_hash TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )""")

        table_cmds.append("""
        CREATE TABLE IF NOT EXISTS forums (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            owner_user_id TEXT NOT NULL,
            FOREIGN KEY (owner_user_id) REFERENCES users(id) ON DELETE CASCADE
        )""")

        table_cmds.append("""
        CREATE TABLE IF NOT EXISTS channels (
            id TEXT PRIMARY KEY,
            type TEXT NOT NULL,
            forum_id TEXT NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (forum_id) REFERENCES forums(id) ON DELETE CASCADE
        )""")

        table_cmds.append("""
        CREATE TABLE IF NOT EXISTS messages (
            id TEXT PRIMARY KEY,
            channel_id TEXT NOT NULL,
            user_id TEXT NOT NULL,
            content TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (channel_id) REFERENCES channels(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )""")

        table_cmds.append("""
        CREATE TABLE IF NOT EXISTS user_forum_relationships (
            user_id TEXT NOT NULL,
            forum_id TEXT NOT NULL,
            role TEXT NOT NULL,
            PRIMARY KEY (user_id, forum_id),
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (forum_id) REFERENCES forums(id) ON DELETE CASCADE
        )""")

        table_cmds.append("""
        CREATE TABLE IF NOT EXISTS tokens (
            token_id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            expires_at TIMESTAMP NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )""")

        # add indexes for performance

        table_cmds.append("""
        CREATE INDEX IF NOT EXISTS idx_user_roles ON user_forum_relationships (forum_id, role);
        """)

        table_cmds.append("""
        CREATE INDEX IF NOT EXISTS idx_tokens_user_id ON tokens (user_id);
        """)

        for cmd in table_cmds:
            self._execute(cmd)

    def close(self):
        if hasattr(self._local, 'connection'):
            self._local.connection.close()
            del self._local.connection

    def generate_uuid(self) -> str:
        return str(uuid.uuid4())

    #%% user crud
    def create_user(self, user: User):
        cmd = "INSERT INTO users (id, username, email, password_hash, created_at) VALUES (?, ?, ?, ?, ?)"
        self._execute(cmd, user.to_row())

    def create_user_if_not_exists(self, user: User):
        cmd = "INSERT OR IGNORE INTO users (id, username, email, password_hash, created_at) VALUES (?, ?, ?, ?, ?)"
        self._execute(cmd, user.to_row())

    def read_user_by_id(self, user_id: str) -> User | None:
        cmd = "SELECT * FROM users WHERE id = ?"
        row = self._fetchone(cmd, (user_id,))
        if row:
            return User.from_row(row)
        return None

    def read_users_by_forum_id(self, forum_id: str) -> list[User]:
        cmd = """
        SELECT u.* FROM users u
        JOIN user_forum_relationships r ON u.id = r.user_id
        WHERE r.forum_id = ?
        """
        rows = self._fetchall(cmd, (forum_id,))
        return [User.from_row(row) for row in rows]

    def read_user_by_channel_id(self, channel_id: str) -> list[User]:
        cmd = """
        SELECT u.* FROM users u
        JOIN messages m ON u.id = m.user_id
        WHERE m.channel_id = ?
        """
        rows = self._fetchall(cmd, (channel_id,))
        return [User.from_row(row) for row in rows]

    def update_user(self, user: User):
        cmd = "UPDATE users SET username = ?, email = ?, password_hash = ? WHERE id = ?"
        self._execute(cmd, (user.username, user.email, user.password_hash, user.id))

    def delete_user(self, user_id: str):
        cmd = "DELETE FROM users WHERE id = ?"
        self._execute(cmd, (user_id,))

    #%% forum crud

    def create_forum(self, forum: Forum):
        cmd = "INSERT INTO forums (id, name, description, created_at, owner_user_id) VALUES (?, ?, ?, ?, ?)"
        self._execute(cmd, forum.to_row())

    def read_forum_by_id(self, forum_id: str) -> Forum | None:
        cmd = "SELECT * FROM forums WHERE id = ?"
        row = self._fetchone(cmd, (forum_id,))
        if row:
            return Forum.from_row(row)
        return None

    def update_forum(self, forum: Forum):
        cmd = "UPDATE forums SET name = ?, description = ? WHERE id = ?"
        self._execute(cmd, (forum.name, forum.description, forum.id))

    def delete_forum(self, forum_id: str):
        cmd = "DELETE FROM forums WHERE id = ?"
        self._execute(cmd, (forum_id,))


    #%% channel crud

    def create_channel(self, channel: Channel):
        cmd = "INSERT INTO channels (id, type, name, description, created_at, forum_id) VALUES (?, ?, ?, ?, ?, ?)"
        self._execute(cmd, channel.to_row())

    def read_channel_by_id(self, channel_id: str) -> Channel | None:
        cmd = "SELECT * FROM channels WHERE id = ?"
        row = self._fetchone(cmd, (channel_id,))
        if row:
            return Channel.from_row(row)
        return None

    def read_channels_by_forum_id(self, forum_id: str) -> list[Channel]:
        cmd = "SELECT * FROM channels WHERE forum_id = ?"
        rows = self._fetchall(cmd, (forum_id,))
        return [Channel.from_row(row) for row in rows]

    def update_channel(self, channel: Channel):
        cmd = "UPDATE channels SET type = ?, name = ?, description = ? WHERE id = ?"
        self._execute(cmd, (channel.type, channel.name, channel.description, channel.id))

    def delete_channel(self, channel_id: str):
        cmd = "DELETE FROM channels WHERE id = ?"
        self._execute(cmd, (channel_id,))

    #%% message crud

    def create_message(self, message: Message):
        cmd = "INSERT INTO messages (id, channel_id, user_id, content, created_at) VALUES (?, ?, ?, ?, ?)"
        self._execute(cmd, message.to_row())

    def read_message_by_id(self, message_id: str) -> Message | None:
        cmd = "SELECT * FROM messages WHERE id = ?"
        row = self._fetchone(cmd, (message_id,))
        if row:
            return Message.from_row(row)
        return None

    def read_messages_by_channel_id(self, channel_id: str, limit: int = 100) -> list[Message]:
        cmd = "SELECT * FROM messages WHERE channel_id = ? ORDER BY created_at DESC LIMIT ?"
        rows = self._fetchall(cmd, (channel_id, limit))
        return [Message.from_row(row) for row in rows]

    def update_message(self, message: Message):
        cmd = "UPDATE messages SET content = ? WHERE id = ?"
        self._execute(cmd, (message.content, message.id))

    def delete_message(self, message_id: str):
        cmd = "DELETE FROM messages WHERE id = ?"
        self._execute(cmd, (message_id,))

    #%% user forum relationship crud
    def create_user_forum_relationship(self, relationship: UserForumRelationship):
        cmd = "INSERT INTO user_forum_relationships (user_id, forum_id, role) VALUES (?, ?, ?)"
        self._execute(cmd, relationship.to_row())

    def read_user_forum_relationship(self, user_id: str, forum_id: str) -> UserForumRelationship | None:
        cmd = "SELECT * FROM user_forum_relationships WHERE user_id = ? AND forum_id = ?"
        row = self._fetchone(cmd, (user_id, forum_id))
        if row:
            return UserForumRelationship.from_row(row)
        return None

    def update_user_forum_relationship(self, relationship: UserForumRelationship):
        cmd = "UPDATE user_forum_relationships SET role = ? WHERE user_id = ? AND forum_id = ?"
        self._execute(cmd, (relationship.role, relationship.user_id, relationship.forum_id))

    def delete_user_forum_relationship(self, user_id: str, forum_id: str):
        cmd = "DELETE FROM user_forum_relationships WHERE user_id = ? AND forum_id = ?"
        self._execute(cmd, (user_id, forum_id))

    #%% token crud
    def issue_token(self, user: User) -> Token:
        token_id = self.generate_uuid()
        created_at = datetime.datetime.now()
        expires_at = created_at + datetime.timedelta(seconds=TOKEN_EXPIRY_SECONDS)

        token = Token(id=token_id, user_id=user.id, created_at=created_at, expires_at=expires_at)
        cmd = "INSERT INTO tokens (token_id, user_id, created_at, expires_at) VALUES (?, ?, ?, ?)"
        self._execute(cmd, token.to_row())

        return token

    def validate_token(self, token_id: str) -> User | None:
        cmd = "SELECT * FROM tokens WHERE token_id = ? AND expires_at > CURRENT_TIMESTAMP"
        row = self._fetchone(cmd, (token_id,))
        if row:
            token = Token.from_row(row)
            return self.read_user_by_id(token.user_id)
        return None

    def revoke_token(self, token_id: str):
        cmd = "DELETE FROM tokens WHERE token_id = ?"
        self._execute(cmd, (token_id,))
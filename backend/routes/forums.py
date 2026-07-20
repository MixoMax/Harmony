from fastapi import APIRouter, Depends
from dependencies import get_forum, get_forum_membership
from classes import Forum, UserForumRelationship


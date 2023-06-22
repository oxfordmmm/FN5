'''Provides model defintiions for the DB tables
Also provides functions for self-queuing pods
'''
import os
import random
import time

import sqlalchemy as db
from dotenv import load_dotenv
from sqlalchemy import Boolean, Integer, String
from sqlalchemy.orm import (DeclarativeBase, Mapped, create_session,
                            mapped_column)
from sqlalchemy_utils import database_exists, create_database

load_dotenv(".db")


class Base(DeclarativeBase):
    pass

class Distance(Base):
    '''Store all distance comparisons <=20 SNPs away
    Don't store all or this grows very quickly
    '''
    __tablename__ = "distances"
    
    guid1: Mapped[str] = mapped_column(String(100), primary_key=True)
    guid2: Mapped[str] = mapped_column(String(100), primary_key=True)
    dist: Mapped[int] = mapped_column(Integer())

    def __init__(self, g1: str, g2: str, d: int):
        '''Constructor. Ensures that the GUIDs are sorted for easier checks

        Args:
            g1 (str): guid1
            g2 (str): guid2
            d (int): Distance between the guids
        '''
        g1, g2 = sorted([g1, g2])
        super().__init__(guid1=g1, guid2=g2, dist=d)

    def __repr__(self) -> str:
        return f"Distance between {self.guid1} and {self.guid2} is {self.dist}"

class Lock(Base):
    '''For locking the DB during execution
    '''
    __tablename__ = "lock"

    start: Mapped[float] = mapped_column(Integer())
    thread_id: Mapped[int] = mapped_column(Integer(), primary_key=True, autoincrement=True)

    def __repr__(self) -> str:
        return f"Lock(thread_id={self.thread_id}, start={self.start})"

class Batch(Base):
    '''For automatic batching of samples
    '''
    __tablename__ = "batch"

    guid: Mapped[str] = mapped_column(String(100), primary_key=True)

    def __repr(self) -> str:
        return f"Batched GUID {self.guid}"

def get_engine(species: str):
    '''Get the DB engine connection
    
    Args:
        species (str): Species table name
    Returns:
        SQLAlchemy connection, SQLAlchemy engine
    '''
    #Connect to DB
    db_path = os.getenv('DB_PATH') + "_" + species
    engine = db.create_engine(db_path, connect_args={'connect_timeout': 30})
    if not database_exists(engine.url):
        create_database(engine.url)
    #Create tables if don't already exist
    Base.metadata.create_all(engine)
    
    connection = engine.connect()
    return connection, engine


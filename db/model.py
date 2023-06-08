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

load_dotenv(".db")


class Base(DeclarativeBase):
    pass

class Distances(Base):
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
    __tablename__ = "snp_lock"

    id_: Mapped[int] = mapped_column(Integer(), primary_key=True, autoincrement=True)
    start: Mapped[float] = mapped_column(Integer())
    thread_id: Mapped[int] = mapped_column(Integer())
    done: Mapped[bool] = mapped_column(Boolean())

    def __repr__(self) -> str:
        return f"Lock(ID={self.id_}, start={self.start}, thread_id={self.thread_id}, done={self.done})"

class Batch(Base):
    '''For automatic batching of samples
    '''
    __tablename__ = "batch"

    guid: Mapped[str] = mapped_column(String(100), primary_key=True)

    def __repr(self) -> str:
        return f"Batched GUID {self.guid}"

def get_engine():
    '''Get the DB engine connection
    
    Returns:
        SQLAlchemy connection, SQLAlchemy engine
    '''
    #Connect to DB
    engine = db.create_engine(os.getenv('DB_PATH'), connect_args={'connect_timeout': 30})
    
    #Create tables if don't already exist
    Base.metadata.create_all(engine)
    
    connection = engine.connect()
    return connection, engine

def add_lock(engine):
    '''Add a unique lock to the table for self-queuing
    Args:
        engine (SQLAlchemy engine): DB engine
    Returns:
        Lock: Lock record
        sqlalchemy.session: Session instance which the lock belongs
    '''
    #Add a lock to the table
    session = create_session(engine)
    lock = Lock(start=time.time(), thread_id=random.randint(0,1000000000), done=False)
    session.add(lock)
    session.commit()
    return lock, session

def get_lock(engine, lock: Lock) -> None:
    '''Wait until the DB is available

    Args:
        engine (SQLAlchemy engine): DB engine
        lock (Lock): This thread's lock record
    '''
    #Wait for your turn
    locked = True
    while locked:
        #Get the next valid lock in the table
        session = create_session(engine)
        q = session.query(Lock).\
            filter(Lock.done == False).\
            filter(Lock.start + 3600 > time.time()).\
            order_by(Lock.id_.asc()).first()
        if q.thread_id == lock.thread_id:
            #We're next!
            locked = False
        else:
            time.sleep(1)
        session.close()

def get_lock_(engine, lock_id: int) -> None:
    '''Wait until the DB is available

    Args:
        engine (SQLAlchemy engine): DB engine
        lock_id (int): The corresponding thread_id for this record
    '''
    #Wait for your turn
    locked = True
    while locked:
        #Get the next valid lock in the table
        session = create_session(engine)
        q = session.query(Lock).\
            filter(Lock.done == False).\
            filter(Lock.start + 3600 > time.time()).\
            order_by(Lock.id_.asc()).first()
        if q.thread_id == lock_id:
            #We're next!
            locked = False
            return
        else:
            time.sleep(1)
        session.close()


def unlock(session, lock: Lock):
    '''Delete the lock record to denote that this thread is now finished

    Args:
        session (sqlalchemy.session): Session used to open the lock
        lock (Lock): Lock record for this thread
    '''
    #Delete from DB
    session.delete(lock)
    session.commit()
    session.close()

def unlock_(engine, lock_id: int):
    '''Find the record associated with this thread ID and delete it

    Args:
        engine (SQLAlchemy.engine): DB engine
        lock_id (int): Thread ID of this lock
    '''
    session = create_session(engine)
    q = session.query(Lock).filter(Lock.thread_id == lock_id).all()
    if len(q) == 0:
        #Not found so complain
        session.close()
        raise Exception(f"Lock {lock_id} not found in DB!")
    else:
        lock = q[0]
        session.delete(lock)
        session.commit()
        session.close()
import sqlalchemy as db
from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy import String
from sqlalchemy import Integer, Boolean
from sqlalchemy.orm import create_session
import time
import subprocess
import shlex
from dotenv import load_dotenv
import os
import random

load_dotenv(".db")

#Connect to DB
engine = db.create_engine(os.getenv('DB_PATH'))
connection = engine.connect()


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

class Closest(Base):
    '''Store the closest neighbour, regardless of cutoff
    '''
    __tablename__ = "closest"

    guid: Mapped[str] = mapped_column(String(100), primary_key=True)
    closest: Mapped[str] = mapped_column(String(100))
    dist: Mapped[int] = mapped_column(Integer())

    def __repr__(self) -> str:
        return f"Nearest neighbour to {self.guid} is {self.closest}. {self.dist} SNPs away"
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

#Create tables if don't already exist
Base.metadata.create_all(engine)

def add_lock() -> Lock:
    #Add a lock to the table
    session = create_session(engine)
    lock = Lock(start=time.time(), thread_id=random.randint(0,1000000000), done=False)
    session.add(lock)
    session.commit()
    return lock, session

def get_lock(lock):
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



def unlock(session, lock):
    #Delete from DB
    session.delete(lock)
    session.commit()
    session.close()

#~~~~~~~~~~~~~~~~~Get the lock from the DB~~~~~~~~~~~~~~~~~~~~~~~~
#Ensures sequential adding across nextflow

#Add row to DB
lock, s = add_lock()

try:
    #Wait your turn...
    get_lock(lock)
    print("Got lock, starting run...")


    #~~~~~~~~~~~~~~~~~Actually compute~~~~~~~~~~~~~~~~~~~~~~~~
    start = time.time()
    output = subprocess.check_output(shlex.split("./pipeline-script.sh")).decode("utf-8").strip()
    print(output)
    print("Computed in: ", time.time()-start)


    #~~~~~~~~~~~~~~~~~Adding new comparions~~~~~~~~~~~~~~~~~~~~~~~~
    start = time.time()
    #Read the comparisons file
    with open("comparisons.txt") as f:
        comparisons = [line.strip().split(" ") for line in f if line.strip() != ""]

    dists = []
    guids = set([g1 for g1, g2, d in comparisons] + [g2 for g1, g2, d in comparisons])
    nearest = {guid : (float('inf'), None) for guid in guids}
    for g1, g2, d in comparisons:
        d = int(d)
        if d <= 20:
            #Add all below cutoff
            dists.append(Distances(g1, g2, d))

        if d < nearest[g1][0]:
            #New closest to guid1
            nearest[g1] = (d, g2)

        if d < nearest[g2][0]:
            #New closest to guid2
            nearest[g2] = (d, g1)
        
    def add_dist_to_session(session, dist, seen):
        #Add to the DB if this doesn't already exist
        if (dist.guid1, dist.guid2) in seen:
            #Already seen so skip
            return
        else:
            session.add(dist)
            seen.add((dist.guid1, dist.guid2))

    def add_nearest_to_session(session, guid, nearest, dist, seen, seen_guids):
        #If this is closer than the DB value, update it
        if guid in seen_guids:
            #Already seen so check dist
            if dist < seen[guid]:
                #New closest so update
                q = session.query(Closest).filter(Closest.guid == guid).first()
                session.delete(q)
                c = Closest(guid=guid, closest=nearest, dist=dist)
                session.add(c)
        else:
            #Not seen yet to add
            c = Closest(guid=guid, closest=nearest, dist=dist)
            session.add(c)

    session = create_session(engine)

    #Loading this all into memory seems inefficient
    #But the alternative is to run a query on each addition which is significantly slower
    seen = session.query(Distances).all()
    seen = set([(s.guid1, s.guid2) for s in seen])

    seen_closest_ = session.query(Closest).all()
    seen_closest = {s.guid: s.dist for s in seen_closest_}
    seen_closest_guids = set([s.guid for s in seen_closest_])

    for d in dists:
        add_dist_to_session(session, d, seen)

    for guid in sorted(list(nearest.keys())):
        dist, near = nearest[guid]
        add_nearest_to_session(session, guid, near, dist, seen_closest, seen_closest_guids)

    session.commit()
    session.close()

    print("Added in: ", time.time() - start)

    #~~~~~~~~~~~~~~~~~Querying comparions~~~~~~~~~~~~~~~~~~~~~~~~
    start = time.time()
    session = create_session(engine)
    guid = "site.20.subj.SA00660055.lab.YA00135475.iso.1.v0.8.3.regenotyped"
    q = session.query(Distances).filter((Distances.guid1 == guid) | (Distances.guid2 == guid)).all()
    for res in q:
        if res.guid1 == guid:
            other = res.guid1
        else:
            other = res.guid2
        print(f"{other} --> {res.dist}")

    q = session.query(Closest).filter(Closest.guid == guid).first()
    session.close()
    print(f"Nearest neighbour: {q.closest} --> {q.dist}")
    print("Queried in: ", time.time() - start)

    #~~~~~~~~~~~~~~~~~Unlock the DB to allow others to run~~~~~~~~~~~~~~~~~~~~~~~~
    unlock(s, lock)
except:
    unlock(s, lock)

    
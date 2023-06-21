'''If the queue is not already full, add a lock to the table
Else, add the sample to the batch table
'''
import argparse
from model import *

def check_locks(engine) -> int:
    '''Check how many locks are in the table

    Args:
        engine (SQLAlchemy.engine): DB engine
    Returns:
        int: Number of valid locks in the table
    '''
    session = create_session(engine)
    q = session.query(Lock).\
        filter(Lock.done == False).\
        filter(Lock.start + 3600 > time.time()).all()
    session.close()
    return len(q)

def add_to_batch(engine, g) -> None:
    '''Add this sample to the batch table

    Args:
        engine (SQLAlchemy.engine): DB engine
        g (str): Guid to add
    '''
    session = create_session(engine)
    #Check there isn't already this sample in a batch
    q = session.query(Batch).filter(Batch.guid == g).all()
    if len(q) != 0:
        return

    b = Batch(guid=g)
    session.add(b)
    session.commit()
    session.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--guid", required=True, help="Guid of the sample to add")
    options = parser.parse_args()

    conn, engine = get_engine()

    if check_locks(engine) >= 3:
        #Queue is full so batch
        add_to_batch(engine, options.guid)
    else:
        lock, _ = add_lock(engine)
        print(lock.thread_id)



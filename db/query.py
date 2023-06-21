'''Very simple querying of the output matrix to find nearest neighbours
'''
from model import *
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--guid", required=True, help="GUID of the sample to query for")
    options = parser.parse_args()

    start = time.time()
    conn, engine = get_engine()
    session = create_session(engine)
    guid = options.guid
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

'''Parse `comparisons.txt` and enter into db
'''
import argparse
from model import *

def add_dist_to_session(session, dist, seen):
    '''Add a distance to the table if not already in there

    Args:
        session (SQLAlchemy.session): Session object
        dist (Distance): SQLAlchemy object for the row to add
        seen (set): Set of already seen guids
    '''
    #Add to the DB if this doesn't already exist
    if (dist.guid1, dist.guid2) in seen:
        #Already seen so skip
        return
    else:
        session.add(dist)
        seen.add((dist.guid1, dist.guid2))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--comparisons", required=True, help="Path to the comparisons file")
    options = parser.parse_args()

    conn, engine = get_engine()
    session = create_session(engine)

    with open(options.comparisons) as f:
        comparisons = [line.strip().split(" ") for line in f if line.strip() != ""]
    dists = []
    for g1, g2, d in comparisons:
        #Note that a sample with `guid1 guid1 -1` is an orphan
        d = int(d)
        if d <= 20:
            #Add all below cutoff
            dists.append(Distances(g1, g2, d))
    
    #Loading this all into memory seems inefficient
    #But the alternative is to run a query on each addition which is significantly slower
    seen = session.query(Distances).all()
    seen = set([(s.guid1, s.guid2) for s in seen])

    for d in dists:
        add_dist_to_session(session, d, seen)
    
    session.commit()
    session.close()


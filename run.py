from db.model import *
import subprocess
import shlex


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


if __name__ == "__main__":
    conn, engine = get_engine()

    #Add lock to DB
    lock, s = add_lock(engine)

    try:
        #Wait your turn...
        get_lock(engine, lock)
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
        #~~~~~~~~~~~~~~~~~Unlock the DB to allow others to run~~~~~~~~~~~~~~~~~~~~~~~~
        unlock(s, lock)
    except:
        #Any case this dies, clean up after yourself
        unlock(s, lock)


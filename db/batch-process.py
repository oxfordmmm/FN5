'''Handle getting GUIDs from batches, and deleting once completed
'''
import argparse
from model import *

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--get", default=False, action='store_true', help="Use to get contents of batch table. Produces a line separated file of GUIDs")
    parser.add_argument("--guids_to_clear", help="Path to a line separated file of GUIDs to delete from the batch table")
    parser.add_argument("--id", default="", help="Optional ID for annotating output files")
    options = parser.parse_args()
    
    conn, engine = get_engine()

    if options.get:
        #Pull out records
        session = create_session(engine)
        q = session.query(Batch).all()
        filename = "batch_guids.txt" if options.id == "" else f"{options.id}_batch_guids.txt"
        with open(filename, "w") as f:
            for row in q:
                f.write(row.guid)
                f.write("\n")
        session.close()
    else:
        #Delete these records
        session = create_session(engine)
        with open(options.guids_to_clear) as f:
            guids = [line.strip() for line in f]
        for g in guids:
            b = session.query(Batch).filter(Batch.guid == g).first()
            if b is not None:
                #Don't try and delete if it doesn't exist
                session.delete(b)
        session.commit()
        session.close()

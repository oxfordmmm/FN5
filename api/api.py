'''Use FASTAPI to construct an API for bucket and db operations

This should probably be split into separate files for DB and bucket operations
    but as this is due to be integrated into the `gpas` repo, it's not worthwhile
    figuring out how to join FastAPI apps/routers

All of these methods need auth setup. Most of these should be internal API calls
    so should only be accessible with appropriate access
'''
import time
from typing import Annotated

import oci

from fastapi import FastAPI, Response, File, UploadFile, HTTPException
from model import Batch, Distance, Lock, get_engine
from sqlalchemy.orm import create_session

app = FastAPI()

#==============================
# @@@@ External API calls @@@@
#==============================

@app.get("/api/relatedness/{species}/neighbours")
async def neighbours(species: str, guid: str):
    '''Get all of the neighbours of a given sample within the cutoff

    Args:
        species (str): Name of the species (for the db)
        guid (str): GUID of the sample to find neighbours for
    '''
    conn, engine = get_engine(species)
    session = create_session(engine)

    #TODO: Add access filtering
    q = session.query(Distance).\
        filter((Distance.guid1 == guid) | (Distance.guid2 == guid)).\
        all()

    if len(q) == 0:
        #We have nothing in the table so this hasn't been processed yet
        #In this case, return 404
        raise HTTPException(status_code=404, detail="GUID not found!")
    
    distances = {}
    orphan = False
    for record in q:
        if record.guid1 == guid:
            if record.guid1 == record.guid2:
                #Orphan record (but continue in case this is outdated)
                orphan = True
            elif record.guid2 == "||QC_FAIL||":
                #Sample had <50% ACGT so failed QC. Report as such
                return {"distances": None, "QC": "FAIL"}
            distances[record.guid2] = record.dist
        elif record.guid2 == guid:
            distances[record.guid1] = record.dist
    
    session.close()
    if orphan and len(distances) == 1:
        #We have an orphan with just the orphan record so return no distances
        return {"distances": {}, "QC": "PASS"}
    else:
        return {"distances": distances, "QC": "PASS"}

@app.get("/api/relatedness/{species}/recompute")
async def recompute(species: str, guid: str, cutoff: int=9999999):
    '''Recompute the distance of this sample from all existing samples.
    This will take a **long** time so will likely require extended timeout

    Args:
        species (str): Name of the species (for the db)
        guid (str): GUID of the sample to recompute distances for
        cutoff (int, optional): SNP cutoff for computation. Defaults to having no cutoff.
    '''
    #TODO: Implement this. Requires FN5 changes (and probably addition of Python bindings)
    #This is not required for MVP, but adding as a placeholder
    return {"distances": "NOT IMPLEMENTED"}

#==============================
# @@@@ Internal API calls @@@@
#==============================


#==============================
# @@@@ Database API calls @@@@
#==============================

@app.get("/api/relatedness/{species}/db/{guid}/check_lock")
async def check_lock(species: str, guid: str):
    '''Check the size of the lock table. 
        if bigger than an arbitrary cutoff (3)
            add to batch and return null
        else
            add new lock and return the thread_id
    
    Args:
        species (str): Name of the species (for the db)
        guid (str): This sample's guid. Used for adding to batch if appropriate

    '''
    conn, engine = get_engine(species)
    session = create_session(engine)
    q = session.query(Lock).\
        filter(Lock.start + 3600 > time.time()).all()
    session.close()
    
    queue_size = len(q)

    if queue_size >= 3:
        #Queue is full so batch
        session = create_session(engine)
        #Check there isn't already this sample in a batch
        q = session.query(Batch).filter(Batch.guid == guid).all()
        if len(q) != 0:
            return {"lock": ""}
        b = Batch(guid=guid)
        session.add(b)
        session.commit()
        session.close()
        return {"lock": ""}
    else:
        #Queue has room so add new lock
        session = create_session(engine)

        #Give it 15mins per item in the queue to avoid conflicts
        t = time.time() + 900 * queue_size
        
        lock = Lock(start=t)
        session.add(lock)
        session.commit()
        return {"lock": lock.thread_id}
    
@app.get("/api/relatedness/{species}/db/next_lock")
async def next_lock(species: str):
    '''Get the next lock from the DB. Allows waiting for a lock to be next

    Args:
        species (str): Name of the species (for the db)
    '''
    conn, engine = get_engine(species)
    session = create_session(engine)

    l = session.query(Lock).\
        filter(Lock.start + 3600 > time.time()).\
        order_by(Lock.thread_id.asc()).first()
    session.close()
    if l is not None:
        return {"lock": l.thread_id}
    else:
        return {"lock": ""}

@app.get("/api/relatedness/{species}/db/clear_lock")
async def clear_lock(species: str, lock: int):
    '''Given a lock's thread_id, delete it from the Lock table

    Args:
        species (str): Name of the species (for the db)
        lock (int): Thread ID of this lock
    '''
    conn, engine = get_engine(species)
    session = create_session(engine)

    l = session.query(Lock).filter(Lock.thread_id == lock).first()
    if l is not None:
        session.delete(l)
        session.commit()
        session.close()

@app.get("/api/relatedness/{species}/db/get_batch")
async def get_batch(species: str):
    '''Get all of the GUIDs in the batch table

    Args:
        species (str): Name of the species (for the db)
    '''
    conn, engine = get_engine(species)
    session = create_session(engine)
    q = session.query(Batch).all()
    guids = [batch.guid for batch in q]
    session.close()
    return {"batch": guids}

@app.post("/api/relatedness/{species}/db/add_distances")
async def add_distances(species:str, file: UploadFile):
    '''Given a line separated text file of distances, add to the database

    Args:
        species (str): Name of the species (for the db)
        file (UploadFile): Comparisons file. Each line should be "`guid1` `guid2` `dist`"
    '''
    distances = await file.read()
    distances = distances.decode("utf-8").strip()

    conn, engine = get_engine(species)
    session = create_session(engine)

    #Avoid duplicate entries by ensuring new records aren't already there
    #Single query and filtration is significantly faster than querying for each record
    dists_from_db = session.query(Distance).all()
    dists_from_db = set((d.guid1, d.guid2) for d in dists_from_db)

    for line in distances.split("\n"):
        g1, g2, d = line.split(" ")
        dist = Distance(g1, g2, d)
        d = int(d)
        if (dist.guid1, dist.guid2) in dists_from_db:
            #Already seen so skip
            continue

        session.add(dist)

    session.commit()
    session.close()

@app.post("/api/relatedness/{species}/db/clear_batch")
async def clear_batch(species: str, file: UploadFile):
    '''Given a line separated file of GUIDs, remove each from the Batch table

    Args:
        species (str): Name of the species (for the db)
        file (UploadFile): Line separated file of GUIDs
    '''
    guids = await file.read()
    guids = guids.decode("utf-8").strip()
    guids = set(guids.split("\n"))

    conn, engine = get_engine(species)
    session = create_session(engine)

    batch = session.query(Batch).all()
    for g in batch:
        if g.guid in guids:
            #We want to delete this record
            session.delete(g)
    session.commit()
    session.close()


#==============================
# @@@@@ Bucket API calls @@@@@
#==============================

@app.post("/api/relatedness/{species}/upload")
async def upload(species: str, file: UploadFile, path: str):
    '''Upload a given file to a specified path. Uploads `file` to `species/path` on the bucket

    Args:
        species (str): Name of the species (for the bucket)
        file (UploadFile): File to upload
        path (str): Desired path (including object name) within the species folder on the bucket
    '''
    #Get the object storage and namespace from OCI
    config = oci.config.from_file("~/.oci/config")
    object_storage = oci.object_storage.ObjectStorageClient(config)
    namespace =  object_storage.get_namespace().data

    #Read the incoming data as a binary string for upload
    data = await file.read()

    #Do the upload...
    object_storage.put_object(namespace,"JW-test", species + "/" + path, data)


#Based on https://stackoverflow.com/questions/55873174/how-do-i-return-an-image-in-fastapi
#Currently the only bucket operations should be with `*.tar.gz` files 
@app.get("/api/relatedness/{species}/download",
        responses = {
            200: {
                "content": {"application/gzip": {}}
            }
        },
        # Prevent FastAPI from adding "application/json" as an additional
        # response media type in the autogenerated OpenAPI specification.
        # https://github.com/tiangolo/fastapi/issues/3258
        response_class=Response
)
async def download(species: str, path: str):
    '''Download a specified object from the relatedness bucket

    Args:
        species (str): Name of the species (for the bucket)
        path (str): Path to the object within the bucket's species folder
    '''
    #The response for this doesn't work from the FastAPI docs page, but does via curl
    #Get the object storage and namespace from OCI
    config = oci.config.from_file("~/.oci/config")
    object_storage = oci.object_storage.ObjectStorageClient(config)
    namespace =  object_storage.get_namespace().data

    #Get the object from the bucket
    try:
        obj = object_storage.get_object(namespace, "JW-test", species + "/" + path)
        #Get the object's contents
        bin_obj = obj.data.content
        return Response(content=bin_obj, media_type="application/gzip")
    except:
        raise HTTPException(status_code=404, detail="Sample not found!")

@app.post("/api/relatedness/{species}/delete")
async def delete(species: str, file: UploadFile):
    '''Delete the specified files from the relatedness bucket.
    Takes a line separated file of bucket paths to delete

    Args:
        species (str): Name of the species (for the bucket)
        file (UploadFile): Line separated file of bucket paths to delete
    '''
    #Get the object storage and namespace from OCI
    config = oci.config.from_file("~/.oci/config")
    object_storage = oci.object_storage.ObjectStorageClient(config)
    namespace =  object_storage.get_namespace().data
    
    paths = await file.read()
    paths = paths.decode("utf-8").strip()
    for line in paths.split("\n"):
        object_storage.delete_object(namespace, "JW-test", species + "/" + line)




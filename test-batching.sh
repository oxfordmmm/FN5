#!/bin/bash

set -xe

lock_file=$(echo $1)_lock.txt

python add_lock.py --guid $1 > $lock_file


if [ -s $lock_file ]; then
        #Lock was added so continue
        lock=$(cat $lock_file)
        echo waiting for lock...
        python wait-for-lock.py --lock $lock

        #Lock attained so pull from batch
        python batch-process.py --get --id $1
        echo Got batch:
        cat $(echo $1)_batch_guids.txt
        echo $1

        #`do stuff` now we have the lock and batch
        sleep 20

        #Done comparing now so clean up batch and lock
        python batch-process.py --guids_to_clear $(echo $1)_batch_guids.txt
        echo cleared batch
        python release-lock.py --lock $lock

else
        #Queue was full, so added to batch and can now exit
        echo $1 was added to batch
        exit 0
fi



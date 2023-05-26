#!/bin/bash

set -e

#Use environment vars to load the secret bucket link
source .env

#Pull the existing saves from a bucket here
#   <bucket> --> saves 
#Get the latest saves from the bucket
latest=$(curl -SsL $bucket | jq -r '.objects[].name' | sort | tail -n 1)
echo Using $latest

echo Getting from bucket
curl -SsL $bucket/$latest > $latest
echo
echo untaring
tar --use-compress-program=pigz -xf $latest
echo
#<><><><><><>


#Get comparisons with the new ones
echo comparing
./fn5 --add_many $input_paths --cutoff 20000 > comparisons.txt
echo
#<><><><><><>

#Push new saves to a bucket here
#   saves/* --> <bucket>
output="$(date +%s).tar.gz"
echo compressing
tar --use-compress-program=pigz -cf $output saves
echo
echo uploading
curl -X PUT --data-binary "@$(pwd)/$output" $bucket/$output
#<><><><><><>

#Do something with comparions.txt here
#   Parse into DB? 
#   Upload to bucket?
#<><><><><><>

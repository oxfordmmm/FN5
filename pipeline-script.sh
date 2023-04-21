#!/bin/bash

#Use environment vars to load the secret bucket link
source .env

#Pull the existing saves from a bucket here
#   <bucket> --> saves 
#Get the latest saves from the bucket
latest=$(curl -SsL $bucket | jq -r '.objects[].name' | sort | head -n 1)
echo Using $latest
curl -SsL $bucket/$latest > $latest
tar xzf $latest
#<><><><><><>


#Get comparisons with the new ones
./fast-snp --add_many $input_paths 20 > comparisons.txt
#<><><><><><>

#Push new saves to a bucket here
#   saves/* --> <bucket>
output="$(date +%s).tar.gz"
tar czf $output saves
curl -X PUT --data-binary "@$(pwd)/$output" $bucket/$output
#<><><><><><>

#Do something with comparions.txt here
#   Parse into DB? 
#   Upload to bucket?
#<><><><><><>

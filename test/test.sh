#!/bin/bash

#Do the comparisons to file equality can be checked with pytest
#As most outputs are to stdout, and all we care about is equality, this seems
#   like an easier option than setting up and using gtest properly

#If it wasn't for the multithreaded nature of this, files would be reproducable
#   and could easily be checked directly with bash

#For the sake of speed and size, we use COVID-19 instead of TB
#All test cases have been manually edited to ensure known expected outputs

set -xe

mkdir -p test/saves
mkdir -p test/output

#Clear output dir
for f in $(ls test/output);
do
    rm test/output/$f
done


./fn5 --bulk_load test/all.txt --saves_dir test/saves --reference NC_045512.fasta --mask ignore
./fn5 --compute 20 --saves_dir test/saves > test/output/1.txt

./fn5 --add test/cases/4.fasta --saves_dir test/saves --output_file test/output/2.txt --reference NC_045512.fasta --mask ignore

./fn5 --add_many test/two_samples.txt --saves_dir test/saves --reference NC_045512.fasta --mask ignore > test/output/3.txt

./fn5 --compare_row test/cases/4.fasta --saves_dir test/saves --reference NC_045512.fasta --mask ignore > test/output/4.txt



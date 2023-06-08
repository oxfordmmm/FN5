# Find Neighbour 5
Spiritual successor to [Find Neighbour 4](https://github.com/davidhwyllie/findNeighbour4) - continuing the work of David Wyllie.

[![Tests](https://github.com/oxfordmmm/FN5/actions/workflows/test.yaml/badge.svg)](https://github.com/oxfordmmm/FN5/actions/workflows/test.yaml)
[![Build FN5 images](https://github.com/oxfordmmm/FN5/actions/workflows/build.yaml/badge.svg?branch=testing-docker)](https://github.com/oxfordmmm/FN5/actions/workflows/build.yaml)

SNP matrix generation with caching to disk to allow fast reloading.
A test set of 1286 cryptic samples was used. Once parsed and saved, these can be read into memory (on a single thread) in 0.5s - scaling linearly

Defaults to using 20 threads where multithreading is used. This can be updated through use of the `--threads` flag

Saves default to `./saves`. This can be updated through use of the `--saves_dir` flag

# Running

## Locally
Compile and run locally. Requires `git`, `bash`, `make`, `cmake`, and `g++` or `clang`. These should be preinstalled on most systems.
```
git clone git@github.com:oxfordmmm/FN5.git
cd FN5
./build.sh

./fn5 <do stuff>
```

## Docker
Docker image is available privately via OCI container registry. Requires authentication.
```
#Open a shell in the container
docker run -it lhr.ocir.io/lrbvkel2wjot/oxfordmmm/fn5:latest bash
```

## Nextflow
Requires OCI authorisation too.

https://github.com/oxfordmmm/fn5_pipeline


# Run tests
A unit test suite is provided, as well as a set of end-to-end tests
```
#Compile and run the unit tests
./unit-test.sh

#Run end-to-end tests and ensure expected output
./test/test.sh
python -m pytest -vv
```

# Load testing
Using the cryptic set of 15229 samples, on a VM with 64 cores (using max 250 threads):

* Time to parse and save to disk: 2min 33s
* Time to construct SNP matrix with cutoff 12: 3min 38s
* Time to construct SNP matrix with cutoff 20: 3min 38s
* Time to construct SNP matrix without cutoff (90000): 9min 22s
* Size of saves on disk: 305M. FASTA files ~65G

By not returning anything in cases where the comparison > cutoff, matrix size shows significant decrease, as does time taken.

From cold (i.e nothing in RAM) and with a single thread, pulling out a row of comparisons on this much data can be performed in ~10.5s - meaning that this is fast enough that we could containerise this to run as a service. This should be able to circumvent the need to have an instance running constantly. Also, the time taken for this should just scale linearly (we are doing N comparisons).
It also utilises < 0.5GiB RAM for this...
Most of this time taken is also just for loading the saves (this takes somewhere in the region of 7-8s), so extending to add samples in a batch like this would be slow, but could be containerised and orchestrated. This should allow ~1min turn around for small batches

By using a SNP cutoff, the amount of data produced becomes significantly more tractable - cuttof of 20 reduces the comparison matrix for the cryptic samples from ~14GB (>100,000,000 comparisons) to ~200MB

With binary saves:
* Time to parse and save to disk: 54s
* Time to construct SNP matrix with cutoff of 20: 3min 30s

## Parse some FASTA files
Parse some FASTA files into saves. Pass a path to a line separated file of FASTA paths. Currently only supporting upper case nucleotides. This is multithreaded, so can be performed efficiently.

```
./fn5 --bulk_load <path to list>
```

## Build a SNP matrix
Perform pairwise comparisons of all samples which have been saved already. Dumps to a txt file of format `<guid1> <guid2> <dist>`. Current path to this is `outputs/all.txt`. This can be changed with the `--output_file` flag
Cutoff is a mandatory parameter (set arbitrarily high to ignore). `12` is a good value for speed and use, but for several use cases, this cutoff may not be helpful
```
./fn5 --compute <cutoff>
```

## Add a single new file
From cold (i.e nothing in RAM), add a new sample to the matrix. This works fine for adding single samples, but is **very** slow for building a full matrix from scratch due to reading from disk for every sample
```
./fn5 --add <FASTA path>
```

## Add a batch of new files
From cold, add a list of samples to the matrix. As multiple comparisons occur without reading from disk each time, this is close to performance of `--bulk_load` then `--compute`. Takes a path to a line separated file of FASTA paths. Currently uses a 20 SNP threshold
```
./fn5 --add_many <path>
```

## Set SNP cutoff
In most cases, a cutoff of 20 makes sense, but to change this, use the `--cutoff` flag. To have no cutoff, just set arbirarily high

## Outputs
By default, most functions lead to outputs to `stdout`. This allows file redirection/piping to other programs. Querying this output should then be trivial

### Setup
As this uses Python for the results parsing and database, install all requirements (optionally in a virtualenv) with `pip install -r requirements.txt`.

1. Create a file called `.env` at the top level of this directory
2. Populate `.env` with
```
bucket="<bucket PAR URl here>"
input_paths="<path to a line separated file of FASTA paths to add>"
```
3. Setup an SQL database (MySQL works here)
4. Create a file called `.db` at the top level of this directory
5. Populate `.db` with
```
DB_PATH="<DB URL here>"
#Example
DB_PATH="mysql://<user>:<password>@<host>:<port>/<db name>"
```

### Calculate distances
Run `python run.py`

### Query database
Run `python query.py --guid <guid here>`

### Quality control
FN5 checks that samples are at least 50% ACGT before allowing them to be saved.
Any samples which produce `||QC_FAIL: <guid>||` have failed this check and will not be saved!

## TODO:
* Lower case FASTA support


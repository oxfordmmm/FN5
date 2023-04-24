# fast-snp
(Yes, I know this name has already been used for other projects. It isn't permanent)

SNP matrix generation with caching to disk to allow fast reloading.
A test set of 1286 cryptic samples was used. Once parsed and saved, these can be read into memory (on a single thread) in 0.5s - scaling linearly

Defaults to using 20 threads where multithreading is used. This can be updated through use of the `--threads` flag

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

## Compile
```
g++ src/fast-snp.cpp -Isrc -std=c++20 -w -pthread -O3 -o fast-snp
```

## Parse some FASTA files
Parse some FASTA files into saves. Pass a path to a line separated file of FASTA paths. Currently only supporting upper case nucleotides. This is multithreaded, so can be performed efficiently.

```
./fast-snp --bulk_load <path to list>
```

## Build a SNP matrix
Perform pairwise comparisons of all samples which have been saved already. Dumps to a txt file of format `<guid1> <guid2> <dist>`. Current path to this is `outputs/all.txt`
Cutoff is a mandatory parameter (set arbitrarily high to ignore). `12` is a good value for speed and use, but for several use cases, this cutoff may not be helpful
```
./fast-snp --compute <cutoff>
```

## Add a single new file
From cold (i.e nothing in RAM), add a new sample to the matrix. This works fine for adding single samples, but is **very** slow for building a full matrix from scratch due to reading from disk for every sample
```
./fast-snp --add <FASTA path>
```

## Add a batch of new files
From cold, add a list of samples to the matrix. As multiple comparisons occur without reading from disk each time, this is close to performance of `--bulk_load` then `--compute`. Takes a path to a line separated file of FASTA paths. Currently uses a 20 SNP threshold
```
./fast-snp --add_many <path>
```

## Querying data
Currently, pairwise distances are dumped to a plaintext file consisting of `<guid1> <guid2> <dist>`. This works fine for cases in which we actually only care about distances within cutoff. However, if we remove the cutoff, this file grows to be dangerously large, and **very** slow to query (1.5min+ with 15226 samples)

### Simple queries
Based on the data in `outputs/all.txt`, find samples which match. Note that this is reliant on distances being <= cutoff defined when this was populated
```
python3 query.py --guid <guid>
```

Optinally add a secondard cutoff. Useful for if you populated `outputs/all.txt` with a cutoff of say 100 but want to query under 20 for example.
```
python3 query.py --guid <guid> --snp <cutoff>
```
It is not wise to use this on large datasets without a cutoff in place though due to time and space complexity. Weirdly a simple C++ implementation is significantly slower...


### More complex
In cases where we are interested in finding arbitrarily high cutoffs, or just want the nearest neighbour of a single sample (even outside of cutoff distance), it is significantly quicker to just recompute this. This will find all neighbours <= `<cutoff>` away, and if there are none in this range, return the nearest one.
```
./fast-snp --compare_row <path to sample FASTA> <cutoff>
```
Note that this is significantly slower than querying the precomputed data in cases where a cutoff is used and we don't care about nearest past this. This is also entirely on a single thread, so could be used within a container or similar...
With 15226 samples, it can return in ~10.5s

## Nextflow-like
Testing some Nextflow-like behaviour, including fetching from and pushing to buckets. Currently using cutoff of 20
1. Create a file called `.env` at the top level of this directory
2. Populate with
```
bucket="<bucket URl here>"
input_paths="<path to a line separated file of FASTA paths to add>"
```
3. Run `./pipeline-script.sh`

Results will be populated to `comparisons.txt` currently, but this could be parsed into a DB or similar

## TODO:
* Replace hard coded values such as ref genome etc
* Multithread reading saves from disk? Possibly worse performance??
* Lower case FASTA support
* Proper DB? Currently this is based on the idea that the output file can be parsed as required
* Ref genome isn't strictly a FASTA file, but that is trivial to update
* Binary saves rather than ASCII


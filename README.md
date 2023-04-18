# fast-snp
SNP matrix generation with caching to disk to allow fast reloading.
A test set of 1286 cryptic samples was used. Once parsed and saved, these can be read into memory (on a single thread) in 0.5s - scaling linearly

# Note
Currently argument parsing isn't well implemented (this is proof of concept). In the below examples, ensure that the arguments are exactly this way around.

Also uses 20 threads where multithreaded currently.

# Load testing
Using the cryptic set of 15229 samples, on a VM with 64 cores (using max 250 threads):

* Time to parse and save to disk: 2min 33s
* Time to construct SNP matrix with cutoff 20: 3min 38s

By not returning anything in cases where the comparison > cutoff, matrix size shows significant decrease, as does time taken

## Compile
```
g++ fast-snp.cpp -std=c++20 -w -pthread -O3 -o fast-snp
```

## Parse some FASTA files
Parse some FASTA files into saves. Pass a path to a line separated file of FASTA paths. Currently only supporting upper case nucleotides. This isn't very fast due to single threaded, but is >=1 order of magnitude faster than Python

```
./fast-snp --bulk_load <path to list>
```

## Build a SNP matrix
Perform pairwise comparisons of all samples which have been saved already. Dumps to a txt file of format `<guid1> <guid2> <dist>`. Current path to this is `outputs/all.txt`
Cutoff is a mandatory parameter (set arbitrarily high to ignore). `12` is a good value for speed and use, but for several use cases, this cutoff may not be helpful
```
./fast-snp --compute <cutoff>
```

## TODO:
* Replace hard coded values such as save path, ref genome etc
* Multithread reading saves from disk? Possibly worse performance??
* Lower case FASTA support
* Clean up arg parsing to be less rigid
* Proper DB? Currently this is based on the idea that the output file can be parsed as required
* Ref genome isn't strictly a FASTA file, but that is trivial to update
* Binary saves rather than ASCII


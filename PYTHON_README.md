# Find Neighbour 5
Spiritual successor to [Find Neighbour 4](https://github.com/davidhwyllie/findNeighbour4) - continuing the work of David Wyllie.

[![Tests](https://github.com/oxfordmmm/FN5/actions/workflows/test.yaml/badge.svg)](https://github.com/oxfordmmm/FN5/actions/workflows/test.yaml)
[![Build FN5 images](https://github.com/oxfordmmm/FN5/actions/workflows/build.yaml/badge.svg?branch=testing-docker)](https://github.com/oxfordmmm/FN5/actions/workflows/build.yaml)

SNP matrix generation with caching to disk to allow fast reloading.

# Install
As this is compiled client side, ensure you have a functional C++ compiler such as GCC installed. Currently only tested on Linux
```
# Optional virtual environment
python -m virtualenv env
souce env/bin/activate

# Install via pip
pip install fn5
```

# Notes
This provides some of the bindings to the underlying FN5 library, but has some limitations. Most of the functionality should be exposed via the Python bindings though.

## Read FASTA files
Firstly, you'll need a reference genome, and a list of positions to mask within it. The mask is optional, but can be used to mask homoplastic or phylogenetic regions which aren't of epidemialogical interest. The mask should be line separated genome positions.
```
import fn5

reference = fn5.load_reference("<path to your reference>")
mask = fn5.load_mask("<path to your mask>")

# Alternatively ignore the mask
mask = set()
```
Then, samples can be parsed from FASTA files
```
sample1 = fn5.Sample("<path to sample1's FASTA>", reference, mask, "sample1")
sample2 = fn5.Sample("<path to sample2's FASTA>", reference, mask, "sample2")
...
```

## Save samples
For the sake of efficency, a sample can be written to (and subsequently loaded from) disk.

Note that the `fn5.save` method writes 5 files to the specified direction. `<path>/<sample ID>.A`, `<path>/<sample ID>.C`. ...
```
fn5.save("<some output directory>", sample1)
```

## Load samples
Load pre-saved samples from disk. This should be significantly faster than re-parsing the FASTA files.
```
sample1 = fn5.load("<some output directory>/sample1")
```

## Compute distances
Distances can be computed with arbitrary SNP cuttoffs.

### Single distance
If a returned distance == your cutoff + 1, the two samples are further away than the SNP cutoff.
```
sample1.dist(sample2, <some cutoff>)
```

### Distance matrix
By default this method uses 4 threads, and no cutoff. If a pair of samples is missing from the returned distance list, they are further away than the given cutoff.
```
samples = [fn5.load("<some directory>/"+f) for f in <existing filepaths>]
fn5.compute(samples)

# With more/less threads
fn5.compute(samples, thread_count=12)
fn5.compute(samples, thread_count=1)

# With a SNP cutoff
fn5.compute(samples, cutoff=12)
``` 

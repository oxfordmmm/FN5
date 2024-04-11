#pragma once
#include "sample.hpp"

#include <mutex>
#include <tuple>
#include <future>

/**
* @brief Code for performing comparisons, writing outputs etc
*/


using namespace std;

/**
* @brief A mutex lock used for multi-threaded behaviours
*/
extern mutex mutex_lock;

/**
* @brief Default save dir. Can be updated via args
*/
extern string save_dir;

/**
* @brief Default output file. Can be updated via args
*/
extern string output_file;

/**
* @brief Maximum number of threads to use. Can be updated via args
*/
extern int thread_count;

/**
* @brief Default reference genome. Note that changing this without deleting saves **WILL** cause issues
*/
extern string ref_genome_path;

/**
* @brief Default exclusion mask. Note that changing this without deleting saves **WILL** cause issues
            If this is set to "ignore", no mask will be used.
*/
extern string exclude_mask_path;

/**
* @brief Whether to print debug messages such as how many samples are loaded. Can be changed with the `--debug` flag
*/
extern bool debug;

/**
* @brief Load all saves from disk
*
* @returns Vector of samples
*/
vector<Sample*> load_saves();

/**
* @brief Load some saves. To be used by a thread
*
* @param filenames Vector of paths to saves
* @param acc Accumulator to be added to once all saves are loaded. Used for implicit return
*/
void load_save_thread(vector<string> filenames, vector<Sample*> *acc);

/**
* @brief Load all saves using multithreading
*
* @returns Vector of all loaded saves
*/
vector<Sample*> load_saves_multithreaded();


/**
* @brief Parse the FASTA files defined in `paths` and save to disk in a threadsafe manner
*
* @param paths Vector of FASTA paths to load
* @param reference Reference nucleotides
* @param mask Exclude mask
* @param acc Vector for accumulating samples from threads
*/
void parse_n(vector<string> paths, string reference, unordered_set<int> mask, vector<Sample*> *acc);

/**
* @brief Bulk load FASTA files and save to disk
*
* @param path Path to a line separated file of FASTA paths
* @param reference Reference nucleotides
* @param mask Exclude mask
* @return Vector of the samples loaded
*/
vector<Sample*> bulk_load(string path, string reference, unordered_set<int> mask);

/**
* @brief Save a list of comparisons to disk. Threadsafe
*
* @param comparisons List of precomputed comparisons. Tuples of (guid1, guid2, dist)
*/
void save_comparisons(vector<tuple<string, string, int>> comparisons);

/**
* @brief Print a list of comparisons to stdout. Threadsafe
*
* @param comparisons List of precomputed comparisons. Tuples of (guid1, guid2, dist)
*/
void print_comparisons(vector<tuple<string, string, int>> comparisons);


/**
* @brief Given a sample and a list of save paths, iteratively load a save & find the distance
*
* @param paths List of save paths
* @param sample Pre-loaded sample
* @param cutoff SNP cutoff 
*/
void do_comparisons_from_disk(vector<string> paths, Sample* sample, int cutoff);

/**
* @brief Add a single new FASTA to saved samples. Compute distances between this sample and all existing saves
*
* @param path Path to the FASTA file
* @param reference Reference nucleotides
* @param mask Exclude mask
* @param cutoff SNP cutoff
*/
void add_sample(string path, string reference, unordered_set<int> mask, int cutoff);

/**
* @brief Find distances between given sample pairs, printing results to stdout
*
* @param comparisons Pairs of samples to find distances between
* @param cutoff SNP cutoff
*/
void do_comparisons(vector<tuple<Sample*, Sample*>> comparisons, int cutoff);

/**
* @brief Similar to `add`, but loads to memory once to add multiple samples
*
* @param path Path to a line separated file of FASTA paths
* @param reference Reference nucleotides
* @param mask Exclude mask
* @param cutoff SNP threshold
*/
void add_many(string path, string reference, unordered_set<int> mask, int cutoff);

/**
* @brief Add a sample to existing saves. Prints results to stdout. Returns nearest if no samples within cutoff
*
* @param path Path to a FASTA file
* @param reference Reference nucleotides
* @param mask Exclusion mask
* @param cutoff SNP threshold
*/
void compare_row(string path, string reference, unordered_set<int> mask, int cutoff);

/**
* @brief Compute a pairwise matrix of specified samples already in memory
* 
* @param cutoff SNP threshold
* @param samples Loaded samples
*/
void compute_loaded(int cutoff, vector<Sample*> samples);

/**
* @brief Reference compress a single sample
*
* @param path Path to a FASTA file
* @param reference Reference nucleotides
* @param mask Exclusion mask
* @param guid GUID for the sample
*/
void reference_compress(string path, string reference, unordered_set<int> mask, string guid);

/**
* @brief Add a batch specified by sample saves in a given dir
*
* @param path Path to a directory of the saves to add
* @param cutoff Cutoff to use
*/
void add_batch(string path, int cutoff);

/**
* @brief Comptue a small matrix multi-threaded, returning distances. To be used by Python API
*
* @param samples Vector of samples to compute matrix for
* @param thread_count Number of threads to use. Defaults to 4
* @param cutoff SNP threshold to cutoff at. Defaults to arbirarily high (999999).
* @returns Vector of distances
*/
vector<tuple<string, string, int>> multi_matrix(vector<Sample*> samples, int thread_count = 4, int cutoff = 9999999);
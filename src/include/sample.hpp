#pragma once
#include <ios>
#include <ostream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <vector>
#include <unordered_set>
#include <string>
#include <string.h>
#include <chrono>
#include <filesystem>
#include <thread>
#include <tuple>
#include <stdexcept>

/**
* @brief Definition of the `Sample` class, and functions for saving and loading samples
*/

using namespace std;

class Sample{
    public:
        /**
        * @brief Set of indices which this sample is `A` and the reference is not
        */
        vector<int> A;

        /**
        * @brief Set of indices which this sample is `C` and the reference is not
        */
        vector<int> C;

        /**
        * @brief Set of indices which this sample is `G` and the reference is not
        */
        vector<int> G;

        /**
        * @brief Set of indices which this sample is `T` and the reference is not
        */
        vector<int> T;

        /**
        * @brief Set of indices which this sample is `N`
        */
        vector<int> N;

        /**
        * @brief This sample's UUID. Assumed to be `FASTA_header.split("|")[-1]`
        */
        string uuid;

        /**
        * @brief Whether a sample has passed QC (N% < 50%). If this is false, the sample
                is not saved, so only vald when instanciated from FASTA
        */
        bool qc_pass;


        /**
         * @brief Sample constructor. Reference compresses a given sample
         * 
         * @param filename FASTA filename
         * @param reference String of reference nucleotides
         * @param mask Genome indices to ignore (based on epidemialogical evidence)
         */
        Sample(string filename, string reference, unordered_set<int> mask, string guid="");

        /**
         * @brief Sample constructor. Used for instanciated a previously saved Sample
         * 
         * @param a Set of genome indices which this sample has an A, differing from the reference
         * @param c Set of genome indices which this sample has an C, differing from the reference
         * @param g Set of genome indices which this sample has an G, differing from the reference
         * @param t Set of genome indices which this sample has an T, differing from the reference
         * @param n Set of genome indices which this sample has an N, differing from the reference
         */
        Sample(vector<int> a, vector<int> c, vector<int> g, vector<int> t, vector<int> n);

        /**
        * @brief Provide a custom implementation of `==` to allow equality checking
        *
        * @param s2 The other sample
        * @return true when the samples hold the same data
        */
        bool operator== (const Sample &s2) const;

        /**
         * @brief Find the SNP distance between this sample and another
         * 
         * @param sample Sample to compare to
         * @param cutoff Distance to stop caring after (for speed)
         * @return int The distance between the two samples. If dist == cutoff + 1, the sample is further away and shouldn't be counted
         */
        int dist(Sample* sample, int cutoff);

    private:
        /**
         * @brief Private method for comparing arbitrary nucleotide positions. i.e find the difference of A's or C's etc
         * 
         * @param this_x Set of places this sample is nucleotide `x` different from the reference
         * @param this_n Set of places this sample has an N value
         * @param sample_x Set of places the other sample is nucleotide `x` different from the reference
         * @param sample_n Set of places the other sample has an N value
         * @param acc Accumulator set of the places which the samples differ
         * @param cutoff Distance to stop caring after (for speed)
         * @return int The distance between the two samples. If dist == cutoff + 1, the sample is further away and shouldn't be counted
         */
        unordered_set<int> dist_x(vector<int> this_x, vector<int> this_n, vector<int> sample_x, vector<int> sample_n, unordered_set<int> acc, unsigned int cutoff);
};

/**
* @brief Save the contents of an unordered set to disk using binary.
*
* @param to_save Set to save
* @param filename Output filename
*/
void save_n(vector<int> to_save, string filename);

/**
* @brief Load an unordered set from a binary save on disk
*
* @param filename File to load
* @returns Unordered set of the ints stored in the file
*/
vector<int> load_n(string filename);

/**
* @brief Save a sample to disk
* 
* @param filename Directory to save in. Actual saves will be [<filename>/<uuid>.A, <filename>/<uuid>.C, ...]
* @param sample Sample to save
*/
void save(string filename, Sample* sample);

/**
* @brief Load a sample from disk
* 
* @param filename Base filename to load from. Actual saves will be [<filename>.A, <filename>.C, ...]
* @returns Sample loaded from disk
*/
Sample* readSample(string filename);

/**
* @brief Load the reference from disk
* 
* @param filename Path to the reference genome FASTA
* @returns string of the reference nucleotides
*/
string load_reference(string filename);


/**
* @brief Load the exclusion mask from disk
* 
* @param filename Path to the exclude mask
* @returns unordered_set<int> positions in the mask
*/
unordered_set<int> load_mask(string filename);

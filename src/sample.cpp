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

/**
* @brief Definition of the `Sample` class, and functions for saving and loading samples
*/

using namespace std;

class Sample{
    public:
        /**
        * @brief Set of indices which this sample is `A` and the reference is not
        */
        unordered_set<int> A;

        /**
        * @brief Set of indices which this sample is `C` and the reference is not
        */
        unordered_set<int> C;

        /**
        * @brief Set of indices which this sample is `G` and the reference is not
        */
        unordered_set<int> G;

        /**
        * @brief Set of indices which this sample is `T` and the reference is not
        */
        unordered_set<int> T;

        /**
        * @brief Set of indices which this sample is `N`
        */
        unordered_set<int> N;

        /**
        * @brief This sample's UUID. Assumed to be `FASTA_header.split("|")[-1]`
        */
        string uuid;


        /**
         * @brief Sample constructor. Reference compresses a given sample
         * 
         * @param filename FASTA filename
         * @param reference String of reference nucleotides
         * @param mask Genome indices to ignore (based on epidemialogical evidence)
         */
        Sample(string filename, string reference, unordered_set<int> mask){
            char ch;
            fstream fin(filename, fstream::in);
            //Deal with the header first
            //Assume last pipe separated value in header is UUID (at least for now)
            while(fin >> noskipws >> ch){
                if(ch == '\n'){
                    //Line has ended
                    break;
                }
                if(ch == '|'){
                    //New pipe, so reset
                    uuid = "";
                }
                else{
                    //Add to the str
                    uuid += ch;
                }
            }

            int i = 0;
            while (fin >> noskipws >> ch) {
                if(ch == '\n'){ 
                    //We don't care about these endl chars
                    continue;
                }
                if(mask.contains(i)){
                    //Skip if the exclude mask is at this point
                    i++;
                    continue;
                }
                if(ch != reference[i]){
                    //We have a difference from reference, so add to appropriate set
                    switch (ch) {
                        case 'A':
                            A.insert(i);
                            break;
                        case 'C':
                            C.insert(i);
                            break;
                        case 'G':
                            G.insert(i);
                            break;
                        case 'T':
                            T.insert(i);
                            break;
                        case 'N':
                            N.insert(i);
                            break;
                        case '-':
                            N.insert(i);
                            break;
                    }
                }
                i++;
            }
            fin.close();
        }

        /**
         * @brief Sample constructor. Used for instanciated a previously saved Sample
         * 
         * @param a Set of genome indices which this sample has an A, differing from the reference
         * @param c Set of genome indices which this sample has an C, differing from the reference
         * @param g Set of genome indices which this sample has an G, differing from the reference
         * @param t Set of genome indices which this sample has an T, differing from the reference
         * @param n Set of genome indices which this sample has an N, differing from the reference
         */
        Sample(unordered_set<int> a, unordered_set<int> c, unordered_set<int> g, unordered_set<int> t, unordered_set<int> n){
            A = a;
            C = c;
            G = g;
            T = t;
            N = n;
        }

        /**
         * @brief Find the SNP distance between this sample and another
         * 
         * @param sample Sample to compare to
         * @param cutoff Distance to stop caring after (for speed)
         * @return int The distance between the two samples. If dist == cutoff + 1, the sample is further away and shouldn't be counted
         */
        virtual int dist(Sample* sample, int cutoff){
            unordered_set<int> acc;
            acc = dist_x(A, N, sample->A, sample->N, acc, cutoff);
            acc = dist_x(C, N, sample->C, sample->N, acc, cutoff);
            acc = dist_x(T, N, sample->T, sample->N, acc, cutoff);
            acc = dist_x(G, N, sample->G, sample->N, acc, cutoff);
            return acc.size();
        }

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
        unordered_set<int> dist_x(unordered_set<int> this_x, unordered_set<int> this_n, unordered_set<int> sample_x, unordered_set<int> sample_n, unordered_set<int> acc, int cutoff){
            //Increment cutoff so we can reject distances > cutoff entirely
            cutoff++; // FIXME
            for(const int& elem: this_x){
                if(acc.size() == cutoff){
                    return acc;
                }
                if(!sample_x.contains(elem)){ 
                    //Not a in sample
                    if(!sample_n.contains(elem)){
                        //Not an N comparison either, so add
                        acc.insert(elem);
                    }
                }
            }
            for(const int& elem: sample_x){
                if(acc.size() == cutoff){
                    return acc;
                }
                if(!this_x.contains(elem)){
                    //Not a in sample
                    if(!this_n.contains(elem)){
                        //Not an N comparison either, so add
                        acc.insert(elem);
                    }
                }
            }

            return acc;
        }
};


/**
* @brief Save a sample to disk
* 
* @param filename Directory to save in. Actual saves will be [<filename>/<uuid>.A, <filename>/<uuid>.C, ...]
* @param sample Sample to save
*/
void save(string filename, Sample* sample){
    //Append the sample UUID to the filename to save as such
    if(filename[filename.size()-1] != '/'){
        //No trailing / so add
        filename += '/';
    }
    filename += sample->uuid;
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    for(int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        //Find out which one we need to save
        unordered_set<int> toSave;
        switch (types.at(i)) {
            case 'A':
                toSave = sample->A;
                break;
            case 'C':
                toSave = sample->C;
                break;
            case 'G':
                toSave = sample->G;
                break;
            case 'T':
                toSave = sample->T;
                break;
            case 'N':
                toSave = sample->N;
                break;
        }
        ofstream out(f, ios::binary);
        for(const int elem: toSave){
            out << elem << endl;
        }
        out << EOF;
        out.flush();
        out.close();
    }
}

/**
* @brief Load a sample from disk
* 
* @param filename Base filename to load from. Actual saves will be [<filename>.A, <filename>.C, ...]
* @returns Sample loaded from disk
*/
Sample* readSample(string filename){
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    vector<unordered_set<int>> loading;
    for(int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        string acc;
        char ch;
        fstream fin(f, fstream::in | fstream::binary);
        unordered_set<int> toLoad;
        int counter = 0;
        while (fin >> noskipws >> ch) {
            if(ch == '\n'){
                //This is an endl, so store whats currently in acc as an int and reset
                toLoad.insert(stoi(acc));
                acc = "";
            }
            else{
                //Just add to acc
                acc += ch;
            }
        }
        fin.close();

        loading.push_back(toLoad);
    }
    Sample *s = new Sample(loading.at(0), loading.at(1), loading.at(2), loading.at(3), loading.at(4));
    
    //Get the UUID from the filename
    string uuid;
    if(filename.at(filename.size()-1) == '/'){
        //Remove trailing / if req
        filename.pop_back();
    }
    for(int i=0;i<filename.size();i++){
        if(filename.at(i) == '/'){
            //We only want the last part, so reset
            uuid = "";
        }
        else{
            uuid += filename.at(i);
        }
    }
    s->uuid = uuid;
    return s;
}

/**
* @brief Load the reference from disk
* 
* @param filename Path to the reference genome
* @returns string of the reference nucleotides
*/
string load_reference(string filename){
    //Load reference
    string reference;
    char ch;
    fstream fin(filename, fstream::in);
    while (fin >> noskipws >> ch) {
        reference += ch;
    }
    fin.close();

    return reference;
}


/**
* @brief Load the exclusion mask from disk
* 
* @param filename Path to the exclude mask
* @returns unordered_set<int> positions in the mask
*/
unordered_set<int> load_mask(string filename){
    //Load the exclude mask
    fstream fin2(filename, fstream::in);
    string acc;
    unordered_set<int> mask;
    char ch;
    while (fin2 >> noskipws >> ch) {
        if(ch == '\n'){
            mask.insert(stoi(acc));
            acc = "";
        }
        else{
            acc += ch;
        }
    }
    fin2.close();

    return mask;
}
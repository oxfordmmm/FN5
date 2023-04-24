#include "sample.cpp"
#include "argparse.cpp"
#include <mutex>
#include <tuple>

using namespace std;
namespace fs = std::filesystem;


/**
* @brief A mutex lock used for multi-threaded behaviours
*/
mutex mutex_lock;

/**
* @brief Default save dir. Can be updated via args
*/
string save_dir = "saves";

/**
* @brief Default output file. Can be updated via args
*/
string output_file = "outputs/all.txt";

/**
* @brief Maximum number of threads to use. Can be updated via args
*/
int thread_count = 20;


/**
* @brief Load all saves from disk
*
* @returns Vector of samples
*/
vector<Sample*> load_saves(){
    unordered_set<string> saves;
    for (const auto & entry : fs::directory_iterator(save_dir)){
        string p = entry.path();
        //Check for .gitkeep or nothing (we want to ignore this)
        if(p == save_dir+"/.gitkeep" || p == save_dir){
            continue;
        }
        
        //Remove `.A` etc
        p.pop_back();
        p.pop_back();

        //Only add if the path found is not empty
        if(p.find_first_not_of(' ') != string::npos){
            saves.insert(p);
        }
    }

    vector<Sample*> samples;
    for(const string elem: saves){
        Sample *s = readSample(elem);
        samples.push_back(s);
    }

    return samples;
}


/**
* @brief Parse the FASTA files defined in `paths` and save to disk in a threadsafe manner
*
* @param paths Vector of FASTA paths to load
* @param reference Reference nucleotides
* @param mask Exclude mask
* @param acc Vector for accumulating samples from threads
*/
void parse_n(vector<string> paths, string reference, unordered_set<int> mask, vector<Sample*> *acc){
    //Load all of the samples in `paths`
    for(const string path: paths){
        Sample *s = new Sample(path, reference, mask);
        
        mutex_lock.lock();
            acc->push_back(s);
            save(save_dir+"/", s);
        mutex_lock.unlock();
    }
}

/**
* @brief Bulk load FASTA files and save to disk
*
* @param path Path to a line separated file of FASTA paths
* @param reference Reference nucleotides
* @param mask Exclude mask
* @return Vector of the samples loaded
*/
vector<Sample*> bulk_load(string path, string reference, unordered_set<int> mask){
    //Take a path to a file specifying which files to load
    //Parse and save, no comparisons
    //Read the file given as an arg, treating each line as a new filepath
    vector<string> filepaths;
    char ch;
    fstream pathsFile(path, fstream::in);
    string f;
    while(pathsFile >> noskipws >> ch){
        if(ch == '\n'){
            //Only add if the path found is not empty
            if(f.find_first_not_of(' ') != string::npos){
                //Also check for .gitkeep
                if(f != ".gitkeep"){
                    filepaths.push_back(f);
                }
            }
            f = "";
        }
        else{
            f += ch;
        }
    }
    //Add the last one
    if(f != ""){
        filepaths.push_back(f);
    }
    pathsFile.close();

    cout << "Saving " << filepaths.size() << " new samples to " << save_dir << endl;

    //Do comparisons with multithreading
    int chunk_size = filepaths.size() / thread_count;
    vector<thread> threads;
    vector<Sample*> samples;
    for(int i=0;i<thread_count;i++){
        vector<string> these(filepaths.begin() + i*chunk_size, filepaths.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(parse_n, these, reference, mask, &samples));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    for(int i=chunk_size*thread_count;i<filepaths.size();i++){
        Sample *s = new Sample(filepaths.at(i), reference, mask);
        mutex_lock.lock();
            samples.push_back(s);
            save(save_dir+"/", s);
        mutex_lock.unlock();
    }

    //Join the threads
    for(int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    return samples;
}

/**
* @brief Save a list of comparisons to disk. Threadsafe
*
* @param comparisons List of precomputed comparisons. Tuples of (guid1, guid2, dist)
*/
void save_comparisons(vector<tuple<string, string, int>> comparisons){
    //Save some comparisons to disk in a threadsafe manner
    mutex_lock.lock();
        fstream output(output_file, fstream::app);
        for(const tuple<string, string, int> elem: comparisons){
            output << get<0>(elem) << " " << get<1>(elem) << " " << get<2>(elem) << endl;
        }
        output.close();
    mutex_lock.unlock();
}

/**
* @brief Print a list of comparisons to stdout. Threadsafe
*
* @param comparisons List of precomputed comparisons. Tuples of (guid1, guid2, dist)
*/
void print_comparisons(vector<tuple<string, string, int>> comparisons){
    //Save some comparisons to disk in a threadsafe manner
    mutex_lock.lock();
        fstream output(output_file, fstream::app);
        for(const tuple<string, string, int> elem: comparisons){
            output << get<0>(elem) << " " << get<1>(elem) << " " << get<2>(elem) << endl;
        }
        output.close();
    mutex_lock.unlock();
}


/**
* @brief Given a sample and a list of save paths, iteratively load a save & find the distance
*
* @param paths List of save paths
* @param sample Pre-loaded sample
* @param cutoff SNP cutoff 
*/
void do_comparisons_from_disk(vector<string> paths, Sample* sample, int cutoff){
    //To be used by Thread to do comparisons in parallel
    //Used by `add_sample` for multithreading adding a single sample
    vector<tuple<string, string, int>> distances;
    for(int i=0;i<paths.size();i++){
        Sample *s2 = readSample(paths.at(i));
        int dist = sample->dist(s2, cutoff);
        if(dist > cutoff){
            //Further than cutoff so ignore
            continue;
        }
        distances.push_back(make_tuple(sample->uuid, s2->uuid, dist));

        if(distances.size() == 1000){
            //We have a fair few comparisons now, so save
            save_comparisons(distances);
            //Clear for continuing
            distances = {};

        }
    }
    //And save the last few (if existing)
    save_comparisons(distances);
}

/**
* @brief Add a single new FASTA to saved samples. Compute distances between this sample and all existing saves
*
* @param path Path to the FASTA file
* @param reference Reference nucleotides
* @param mask Exclude mask
* @param cutoff SNP cutoff
*/
void add_sample(string path, string reference, unordered_set<int> mask, int cutoff){
    //Parse a new sample
    //Compare it to every saved sample, then save it too
    Sample *s = new Sample(path, reference, mask);

    //Find all saves
    unordered_set<string> saves_;
    for (const auto & entry : fs::directory_iterator(save_dir)){
        string p = entry.path();    
        //Check for .gitkeep or nothing (we want to ignore this)
        if(p == save_dir+"/.gitkeep" || p == save_dir){
            continue;
        }
        
        //Remove `.A` etc
        p.pop_back();
        p.pop_back();

        //Only add if the path found is not empty
        if(p.find_first_not_of(' ') != string::npos){
            saves_.insert(p);
        }
    }
    //Load them and perform comparisons
    vector<string> saves;
    for(const string elem: saves_){
        saves.push_back(elem);
    }

    //Do comparisons with multithreading
    int chunk_size = saves.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<string> these(saves.begin() + i*chunk_size, saves.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(do_comparisons_from_disk, these, s, cutoff));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    vector<string> remaining;
    for(int i=chunk_size*thread_count;i<saves.size();i++){
        remaining.push_back(saves.at(i));
    }
    do_comparisons_from_disk(remaining, s, cutoff);

    //Join the threads
    for(int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    //Save the new sample
    save(save_dir+"/", s);
}

/**
* @brief Find distances between given sample pairs, printing results to stdout
*
* @param comparisons Pairs of samples to find distances between
* @param cutoff SNP cutoff
*/
void do_comparisons(vector<tuple<Sample*, Sample*>> comparisons, int cutoff){
    //To be used by Thread to do comparisons in parallel
    vector<tuple<string, string, int>> distances;
    for(int i=0;i<comparisons.size();i++){
        Sample *s1 = get<0>(comparisons.at(i));
        Sample *s2 = get<1>(comparisons.at(i));
        if(s1->uuid == s2->uuid){
            continue;
        }
        int dist = s1->dist(s2, cutoff);
        if(dist > cutoff){
            //Further than cutoff so ignore
            continue;
        }
        
        distances.push_back(make_tuple(s1->uuid, s2->uuid, dist));
        if(distances.size() == 1000){
            print_comparisons(distances);
            distances = {};
        }
    }
    print_comparisons(distances);
}

/**
* @brief Similar to `add`, but loads to memory once to add multiple samples
*
* @param path Path to a line separated file of FASTA paths
* @param reference Reference nucleotides
* @param mask Exclude mask
* @param cutoff SNP threshold
*/
void add_many(string path, string reference, unordered_set<int> mask, int cutoff){
    // Like `add`, but handles adding >1 sample
    //Should be significantly faster by multithreading

    //Load existing samples
    vector<Sample*> existing = load_saves();

    //Open the path, and treat each line as a new FASTA file
    vector<Sample*> others;
    vector<string> other_paths;
    char ch;
    string acc;
    fstream fin(path, fstream::in);
    while (fin >> noskipws >> ch) {
        if(ch == '\n'){
            if(acc.find_first_not_of(' ') != string::npos){
                //Also check for .gitkeep
                if(acc != ".gitkeep"){
                    other_paths.push_back(acc); 
                }
            }
            acc = "";
        }
        else{
            acc += ch;
        }
    }
    if(acc.find_first_not_of(' ') != string::npos){
            other_paths.push_back(acc);
    }    
    fin.close();

    //Load the samples multithreaded
    //Do comparisons with multithreading
    int chunk_size = other_paths.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<string> these(other_paths.begin() + i*chunk_size, other_paths.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(parse_n, these, reference, mask, &others));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    for(int i=chunk_size*thread_count;i<other_paths.size();i++){
        Sample *s = new Sample(other_paths.at(i), reference, mask);
        mutex_lock.lock();
            others.push_back(s);
            save(save_dir+"/", s);
        mutex_lock.unlock();
    }

    //Join the threads
    for(int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    vector<tuple<Sample*, Sample*>> comparisons;
    //Compare each new one with an each existing comparison
    for(int i=0;i<existing.size();i++){
        for(int j=0;j<others.size();j++){
            comparisons.push_back(make_tuple(existing.at(i), others.at(j)));
        }
    }
    //And compare each new one against other new ones
    unordered_set<string> seen;
    for(int i=0;i<others.size();i++){
        Sample *s1 = others.at(i);
        seen.insert(s1->uuid);
        for(int j=0;j<others.size();j++){
            Sample *s2 = others.at(j);
            if(seen.contains(s2->uuid)){
                continue;
            }
            else{
                comparisons.push_back(make_tuple(s1, s2));
            }
        }
    }

    cout << "Adding " << others.size() << " new samples to an existing " << existing.size() <<  " with " << comparisons.size() << " comparisons" << endl;

    //Do comparisons with multithreading
    chunk_size = comparisons.size() / thread_count;
    vector<thread> threads2;
    for(int i=0;i<thread_count;i++){
        vector<tuple<Sample*, Sample*>> these(comparisons.begin() + i*chunk_size, comparisons.begin() + i*chunk_size + chunk_size);
        threads2.push_back(thread(do_comparisons, these, cutoff));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)

    for(int i=chunk_size*thread_count;i<comparisons.size();i++){
        tuple<Sample*, Sample*> val = comparisons.at(i);
        if(get<0>(val)->uuid == get<1>(val)->uuid){
        }
        int dist = get<0>(val)->dist(get<1>(val), cutoff);
        if(dist <= cutoff){
            mutex_lock.lock();
                cout << get<0>(val)->uuid << " " << get<1>(val)->uuid << " " << dist << endl;
            mutex_lock.unlock();
        }
    }

    //Join the threads
    for(int i=0;i<threads2.size();i++){
        threads2.at(i).join();
    }
}

/**
* @brief Add a sample to existing saves. Prints results to stdout. Returns nearest if no samples within cutoff
*
* @param path Path to a FASTA file
* @param reference Reference nucleotides
* @param mask Exclusion mask
* @param cutoff SNP threshold
*/
void compare_row(string path, string reference, unordered_set<int> mask, int cutoff){
    //Very similar to add_sample, but instead of saving to disk, print to stdout
    //This is because of how difficult it is to query the size of file created without cutoff
    Sample *s = new Sample(path, reference, mask);

    vector<Sample*> samples = load_saves();
    cout << "Comparing against " << samples.size() << endl;
    int closest_dist = 999999999;
    string closest_uuid = "";
    bool found_within_cutoff = false;
    for(int i=0;i<samples.size();i++){
        if(samples.at(i)->uuid == s->uuid){
            //Same sample
            continue;
        }
        int dist = s->dist(samples.at(i), 99999999);
        if(dist <= closest_dist){
            closest_dist = dist;
            closest_uuid = samples.at(i)->uuid;
        }
        if(dist > cutoff){
            continue;
        }
        found_within_cutoff = true;
        cout << s->uuid << " " << samples.at(i)->uuid << " " << dist << endl;
    }
    if(!found_within_cutoff){
        //Nothing found within the cutoff, so output nearest
        cout << "Nearest: " << closest_uuid << " " << closest_dist << endl;
    }

    //Save the sample in a new dir
    save(save_dir+"/", s);
}

/**
* @brief Compute a pairwise matrix of specified samples already in memory
* 
* @param cutoff SNP threshold
* @param samples Loaded samples
*/
void compute_loaded(int cutoff, vector<Sample*> samples){
    //Version of compute() without reading from disk
    //Utilise multithreading for speed

    //Construct list of comparisons (but don't actually do any of them yet)
    vector<tuple<Sample*, Sample*>> comparisons;
    unordered_set<string> seen;
    for(int i=0;i<samples.size();i++){
        Sample *s1 = samples.at(i);
        seen.insert(s1->uuid);
        for(int j=0;j<samples.size();j++){
            Sample *s2 = samples.at(j);
            if(seen.contains(s2->uuid)){
                //We've already done this comparison
                continue;
            }
            else{
                comparisons.push_back(make_tuple(s1, s2));
            }
        }
    }
    cout << "Comparing " << samples.size() << " for a total of " << comparisons.size() << " comparisons" << endl;

    //Clear output file ready for thread-by-thread appending
    fstream output(output_file, fstream::out);
    output.close();

    //Do comparisons with multithreading
    int chunk_size = comparisons.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<tuple<Sample*, Sample*>> these(comparisons.begin() + i*chunk_size, comparisons.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(do_comparisons, these, cutoff));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    vector<tuple<string, string, int>> distances;
    for(int i=chunk_size*thread_count;i<comparisons.size();i++){
        tuple<Sample*, Sample*> val = comparisons.at(i);
        int dist = get<0>(val)->dist(get<1>(val), cutoff);
        if(dist <= cutoff){
            distances.push_back(make_tuple(get<0>(val)->uuid, get<1>(val)->uuid, dist));
        }
    }
    print_comparisons(distances);

    //Join the threads
    for(int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

}

int main(int nargs, const char* args_[]){
    if(nargs == 2){
        string val = args_[1];
        if(val == "--version" || val == "-v"){
            cout << "v1.0.0" << endl;
            return 0;
        }
    }
    map<string, string> args = parse_args(nargs, args_);

    //Check to see if values are supplied or we need to use default
    if(check_flag(args, "--threads")){
        thread_count = stoi(args.at("--threads"));
    }
    if(check_flag(args, "--saves_dir")){
        save_dir = args.at("--saves_dir");
        //Ensure no trailing / for consistency
        if(save_dir[save_dir.size()-1] == '/'){
            save_dir.pop_back();
        }
    }
    if(check_flag(args, "--output_file")){
        output_file = args.at("--output_file");
    }
    
    int cutoff = 20;
    if(check_required(args, {"--cutoff"})){
        //Take the user's req of a cutoff if given
        cutoff = stoi(args.at("--cutoff"));
    }

    //Check for compute first as it doesn't need reference
    if(check_flag(args, "--compute")){
        vector<Sample*> samples = load_saves();
        compute_loaded(cutoff, samples);
        return 0;
    }

    string reference = load_reference("ref.upper.fasta");
    
    unordered_set<int> mask = load_mask("tb-exclude.txt");

    if(check_flag(args, "--bulk_load")){
        bulk_load(args.at("--bulk_load"), reference, mask);
    }

    if(check_flag(args, "--add")){
        add_sample(args.at("--add"), reference, mask, cutoff);
    }

    if(check_flag(args, "--add_many")){
        add_many(args.at("--add_many"), reference, mask, cutoff);
    }

    if(check_flag(args, "--compare_row")){
        compare_row(args.at("--compare_row"), reference, mask, cutoff);
    }

}
#include "include/comparisons.hpp"
#include <exception>

namespace fs = std::filesystem;

mutex mutex_lock;

string save_dir = "saves";

string output_file = "outputs/all.txt";

int thread_count = 20;

string ref_genome_path = "NC_000962.3.fasta";

string exclude_mask_path = "tb-exclude.txt";

bool debug = false;

vector<Sample*> load_saves(){
    unordered_set<string> saves;
    for (const auto &entry : fs::directory_iterator(save_dir)){
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
    for(const string &elem: saves){
        Sample *s = readSample(elem);
        samples.push_back(s);
    }

    return samples;
}

void load_save_thread(vector<string> filenames, vector<Sample*> *acc){
    vector<Sample*> samples;
    for(unsigned int i=0;i<filenames.size();i++){
        samples.push_back(readSample(filenames.at(i)));
    }

    mutex_lock.lock();
        for(unsigned int i=0;i<samples.size();i++){
            acc->push_back(samples.at(i));
        }
    mutex_lock.unlock();
}

vector<Sample*> load_saves_multithreaded(){
    unordered_set<string> saves;
    for (const auto &entry : fs::directory_iterator(save_dir)){
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

    vector<Sample*> acc;
    vector<string> filenames;
    for(const string &elem: saves){
        filenames.push_back(elem);
    }

    int chunk_size = filenames.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<string> these(filenames.begin() + i*chunk_size, filenames.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(load_save_thread, these, &acc));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    vector<string> missed;
    for(unsigned int i=chunk_size*thread_count;i<filenames.size();i++){
        missed.push_back(filenames.at(i));
    }
    load_save_thread(missed, &acc);

    for(unsigned int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    return acc;
}

void parse_n(vector<string> paths, string reference, unordered_set<int> mask, vector<Sample*> *acc){
    //Load all of the samples in `paths`
    for(const string &path: paths){
        Sample *s = new Sample(path, reference, mask);
        
        mutex_lock.lock();
            acc->push_back(s);
            save(save_dir+"/", s);
        mutex_lock.unlock();
    }
}

vector<Sample*> bulk_load(string path, string reference, unordered_set<int> mask){
    //Take a path to a file specifying which files to load
    //Parse and save, no comparisons
    //Read the file given as an arg, treating each line as a new filepath
    vector<string> filepaths;
    char ch;
    fstream pathsFile(path, fstream::in);
    if(!pathsFile.good()){
        throw invalid_argument("Invalid path to line separated paths: " + path);
    }
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

    if(debug){
        cout << "Saving " << filepaths.size() << " new samples to " << save_dir << endl;
    }

    //Do comparisons with multithreading
    int chunk_size = filepaths.size() / thread_count;
    vector<thread> threads;
    vector<Sample*> samples;
    for(int i=0;i<thread_count;i++){
        vector<string> these(filepaths.begin() + i*chunk_size, filepaths.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(parse_n, these, reference, mask, &samples));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    for(unsigned int i=chunk_size*thread_count;i<filepaths.size();i++){
        Sample *s = new Sample(filepaths.at(i), reference, mask);
        mutex_lock.lock();
            samples.push_back(s);
            save(save_dir+"/", s);
        mutex_lock.unlock();
    }

    //Join the threads
    for(unsigned int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    return samples;
}

void save_comparisons(vector<tuple<string, string, int>> comparisons){
    //Save some comparisons to disk in a threadsafe manner
    mutex_lock.lock();
        fstream output(output_file, fstream::app);
        for(const tuple<string, string, int> &elem: comparisons){
            output << get<0>(elem) << " " << get<1>(elem) << " " << get<2>(elem) << endl;
        }
        output.close();
    mutex_lock.unlock();
}

void print_comparisons(vector<tuple<string, string, int>> comparisons){
    //Save some comparisons to disk in a threadsafe manner
    mutex_lock.lock();
        for(const tuple<string, string, int> &elem: comparisons){
            cout << get<0>(elem) << " " << get<1>(elem) << " " << get<2>(elem) << endl;
        }
    mutex_lock.unlock();
}

void do_comparisons_from_disk(vector<string> paths, Sample* sample, int cutoff){
    //To be used by Thread to do comparisons in parallel
    //Used by `add_sample` for multithreading adding a single sample
    vector<tuple<string, string, int>> distances;
    for(unsigned int i=0;i<paths.size();i++){
        Sample *s2 = readSample(paths.at(i));
        if(sample->uuid == s2->uuid){
            //These are the same sample so skip...
            continue;
        }
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

void add_sample(string path, string reference, unordered_set<int> mask, int cutoff){
    //Parse a new sample
    //Compare it to every saved sample, then save it too
    Sample *s = new Sample(path, reference, mask);

    //Find all saves
    unordered_set<string> saves_;
    for (const auto &entry : fs::directory_iterator(save_dir)){
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
    for(const string &elem: saves_){
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
    for(unsigned int i=chunk_size*thread_count;i<saves.size();i++){
        remaining.push_back(saves.at(i));
    }
    do_comparisons_from_disk(remaining, s, cutoff);

    //Join the threads
    for(unsigned int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    //Save the new sample
    save(save_dir+"/", s);
}

void do_comparisons(vector<tuple<Sample*, Sample*>> comparisons, int cutoff){
    //To be used by Thread to do comparisons in parallel
    vector<tuple<string, string, int>> distances;
    for(unsigned int i=0;i<comparisons.size();i++){
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

void add_many(string path, string reference, unordered_set<int> mask, int cutoff){
    // Like `add`, but handles adding >1 sample
    //Should be significantly faster by multithreading

    //Load existing samples
    vector<Sample*> existing = load_saves_multithreaded();

    //Open the path, and treat each line as a new FASTA file
    vector<Sample*> others;
    vector<string> other_paths;
    char ch;
    string acc;
    fstream fin(path, fstream::in);
    if(!fin.good()){
        throw invalid_argument("Invalid path to line separated paths: " + path);
    }
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
    for(unsigned int i=chunk_size*thread_count;i<other_paths.size();i++){
        Sample *s = new Sample(other_paths.at(i), reference, mask);
        mutex_lock.lock();
            others.push_back(s);
            save(save_dir+"/", s);
        mutex_lock.unlock();
    }

    //Join the threads
    for(unsigned int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

    vector<tuple<Sample*, Sample*>> comparisons;
    //Compare each new one with an each existing comparison
    for(unsigned int i=0;i<existing.size();i++){
        for(unsigned int j=0;j<others.size();j++){
            if(existing.at(i)->uuid == others.at(j)->uuid){
                continue;
            }
            comparisons.push_back(make_tuple(existing.at(i), others.at(j)));
        }
    }
    //And compare each new one against other new ones
    unordered_set<string> seen;
    for(unsigned int i=0;i<others.size();i++){
        Sample *s1 = others.at(i);
        seen.insert(s1->uuid);
        for(unsigned int j=0;j<others.size();j++){
            Sample *s2 = others.at(j);
            if(seen.contains(s2->uuid)){
                continue;
            }
            else{
                comparisons.push_back(make_tuple(s1, s2));
            }
        }
    }
    if(debug){
        cout << "Adding " << others.size() << " new samples to an existing " << existing.size() <<  " with " << comparisons.size() << " comparisons" << endl;
    }

    //Do comparisons with multithreading
    chunk_size = comparisons.size() / thread_count;
    vector<thread> threads2;
    for(int i=0;i<thread_count;i++){
        vector<tuple<Sample*, Sample*>> these(comparisons.begin() + i*chunk_size, comparisons.begin() + i*chunk_size + chunk_size);
        threads2.push_back(thread(do_comparisons, these, cutoff));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)

    for(unsigned int i=chunk_size*thread_count;i<comparisons.size();i++){
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
    for(unsigned int i=0;i<threads2.size();i++){
        threads2.at(i).join();
    }
}

void compare_row(string path, string reference, unordered_set<int> mask, int cutoff){
    //Very similar to add_sample, but instead of saving to disk, print to stdout
    //This is because of how difficult it is to query the size of file created without cutoff
    Sample *s = new Sample(path, reference, mask);

    vector<Sample*> samples = load_saves();
    if(debug){
        cout << "Comparing against " << samples.size() << endl;
    }
    int closest_dist = 999999999;
    string closest_uuid = "";
    bool found_within_cutoff = false;
    for(unsigned int i=0;i<samples.size();i++){
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

void compute_loaded(int cutoff, vector<Sample*> samples){
    //Version of compute() without reading from disk
    //Utilise multithreading for speed

    //Construct list of comparisons (but don't actually do any of them yet)
    vector<tuple<Sample*, Sample*>> comparisons;
    unordered_set<string> seen;
    for(unsigned int i=0;i<samples.size();i++){
        Sample *s1 = samples.at(i);
        seen.insert(s1->uuid);
        for(unsigned int j=0;j<samples.size();j++){
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
    if(debug){
        cout << "Comparing " << samples.size() << " for a total of " << comparisons.size() << " comparisons" << endl;
    }

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
    for(unsigned int i=chunk_size*thread_count;i<comparisons.size();i++){
        tuple<Sample*, Sample*> val = comparisons.at(i);
        int dist = get<0>(val)->dist(get<1>(val), cutoff);
        if(dist <= cutoff){
            distances.push_back(make_tuple(get<0>(val)->uuid, get<1>(val)->uuid, dist));
        }
    }
    print_comparisons(distances);

    //Join the threads
    for(unsigned int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

}

void reference_compress(string path, string reference, unordered_set<int> mask, string guid){
    Sample *s = new Sample(path, reference, mask, guid);
    save(save_dir+"/", s);
    cout << s->uuid << endl;
}

void add_batch(string path, int cutoff){
    //**VERY** similar to `add_many`, but starting with reference compressed sequences
    vector<Sample*> existing = load_saves_multithreaded();
    
    //Use the same method to load the new saves
    //So change the save dir as appropriate
    // string __save_dir = save_dir;
    save_dir = path;
    vector<Sample*> to_add = load_saves_multithreaded();

    vector<tuple<Sample*, Sample*>> comparisons;
    //Compare each new one with an each existing comparison
    for(unsigned int i=0;i<existing.size();i++){
        for(unsigned int j=0;j<to_add.size();j++){
            if(existing.at(i)->uuid == to_add.at(j)->uuid){
                continue;
            }
            comparisons.push_back(make_tuple(existing.at(i), to_add.at(j)));
        }
    }
    //And compare each new one against other new ones
    unordered_set<string> seen;
    for(unsigned int i=0;i<to_add.size();i++){
        Sample *s1 = to_add.at(i);
        seen.insert(s1->uuid);
        for(unsigned int j=0;j<to_add.size();j++){
            Sample *s2 = to_add.at(j);
            if(seen.contains(s2->uuid)){
                continue;
            }
            else{
                comparisons.push_back(make_tuple(s1, s2));
            }
        }
    }

    //Do comparisons with multithreading
    int chunk_size = comparisons.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<tuple<Sample*, Sample*>> these(comparisons.begin() + i*chunk_size, comparisons.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(do_comparisons, these, cutoff));
    }

    //Catch ones missed at the end due to rounding (doing on main thread)
    vector<tuple<string, string, int>> distances;
    for(unsigned int i=chunk_size*thread_count;i<comparisons.size();i++){
        tuple<Sample*, Sample*> val = comparisons.at(i);
        if(get<0>(val)->uuid == get<1>(val)->uuid){
        }
        int dist = get<0>(val)->dist(get<1>(val), cutoff);
        if(dist <= cutoff){
            distances.push_back({get<0>(val)->uuid,  get<1>(val)->uuid, dist});
        }
        if(distances.size() == 1000){
            print_comparisons(distances);
            distances = {};
        }
    }
    print_comparisons(distances);

    //Join the threads
    for(unsigned int i=0;i<threads.size();i++){
        threads.at(i).join();
    }



}

vector<tuple<string, string, int>> ret_distances(vector<tuple<Sample*, Sample*>> comparisons, int cutoff){
    //To be used by Thread to do comparisons in parallel with no cutoff
    vector<tuple<string, string, int>> distances;
    for(unsigned int i=0;i<comparisons.size();i++){
        Sample *s1 = get<0>(comparisons.at(i));
        Sample *s2 = get<1>(comparisons.at(i));
        if(s1->uuid == s2->uuid){
            continue;
        }
        int dist = s1->dist(s2, cutoff);
        if(dist <= cutoff){
            distances.push_back(make_tuple(s1->uuid, s2->uuid, dist));
        }
    }
    // Future return
    return distances;
}

vector<tuple<string, string, int>> multi_matrix(vector<Sample *> samples, int thread_count, int cutoff){
    if(thread_count < 1){
        throw invalid_argument("Invalid thread_count. Should be > 0");
    }
    if(cutoff < 1){
        throw invalid_argument("Invalid cutoff. Should be > 0");
    }
    unordered_set<string> seen;
    vector<tuple<Sample*, Sample*>> comparisons;
    for(unsigned int i=0;i<samples.size();i++){
        Sample *s1 = samples.at(i);
        seen.insert(s1->uuid);
        for(unsigned int j=0;j<samples.size();j++){
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

    //Do comparisons with multithreading
    int chunk_size = comparisons.size() / thread_count;
    vector<future<vector<tuple<string, string, int>>>> promises;
    vector<tuple<string, string, int>> distances;

    for(int i=0;i<thread_count;i++){
        vector<tuple<Sample*, Sample*>> these(comparisons.begin() + i*chunk_size, comparisons.begin() + i*chunk_size + chunk_size);
        promises.push_back(async(&ret_distances, these, cutoff));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    for(unsigned int i=chunk_size*thread_count;i<comparisons.size();i++){
        tuple<Sample*, Sample*> val = comparisons.at(i);
        int dist = get<0>(val)->dist(get<1>(val), cutoff);
        if(dist <= cutoff){
            distances.push_back(make_tuple(get<0>(val)->uuid, get<1>(val)->uuid, dist));
        }
    }

    //Join the threads
    for(unsigned int i=0;i<promises.size();i++){
        vector<tuple<string, string, int>> dists = promises.at(i).get();
        for(unsigned int j=0;j<dists.size();j++){
            distances.push_back(dists.at(j));
        }
    }
    return distances;
}
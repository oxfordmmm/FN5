#include "include/sample.hpp"
#include <cstddef>
#include <stdexcept>
#include <vector>

/**
* @brief Definition of the `Sample` class, and functions for saving and loading samples
*/

using namespace std;


Sample::Sample(string filename, string reference, unordered_set<int> mask, string guid){
    char ch;
    fstream fin(filename, fstream::in);
    if(!fin.good()){
        throw invalid_argument("No such FASTA file " + filename);
    }
    //Deal with the header first
    //Assume last pipe separated value in header is UUID (at least for now)
    string uuid_;
    while(fin >> noskipws >> ch){
        if(ch == '\n'){
            //Line has ended
            break;
        }
        if(ch == '|' || ch == '>'){
            //New pipe, so reset
            uuid_ = "";
        }
        else{
            //Add to the str
            uuid_ += ch;
        }
    }

    //Check if we've been given a GUID instead
    if(guid == ""){
        //Nothing given, so use value from header
        uuid = uuid_;
    }
    else{
        //Given a GUID, so use it
        uuid = guid;
    }

    int i = 0;
    while (fin >> noskipws >> ch) {
        if(ch == '\n' || ch == '\r'){ 
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
                    A.push_back(i);
                    break;
                case 'C':
                    C.push_back(i);
                    break;
                case 'G':
                    G.push_back(i);
                    break;
                case 'T':
                    T.push_back(i);
                    break;
                default:
                    N.push_back(i);
                    break;
            }
        }
        i++;
    }
    fin.close();

    // Check that this file is the same length as the reference
    int ref_size = reference.size(); // Cast to int to avoid warnings
    if(i != ref_size){
        throw invalid_argument("File " + filename + " is not the same length as the reference genome");
    }

    //For a basic QC check we want to ensure that <20% of the sample is N
    //This should infer that the sample is >=80% ACGT
    //Inherently ref has no Ns, so total Ns == this->N.size()
    float total_size = reference.size();
    qc_pass = N.size() / total_size < 0.2;
}

Sample::Sample(vector<int> a, vector<int> c, vector<int> g, vector<int> t, vector<int> n){
    A = a;
    C = c;
    G = g;
    T = t;
    N = n;
    //As samples are not saved if they don't pass QC, this is implicitly true
    qc_pass = true;
}

bool Sample::operator== (const Sample &s2) const{
    bool check =  (uuid == s2.uuid);
    check = check && (A == s2.A);
    check = check && (C == s2.C);
    check = check && (G == s2.G);
    check = check && (T == s2.T);
    check = check && (N == s2.N);
    return check;
}

int Sample::dist(Sample* sample, int cutoff_){
    unordered_set<int> acc;
    unsigned int cutoff = cutoff_;
    acc = dist_x(A, N, sample->A, sample->N, acc, cutoff);
    acc = dist_x(C, N, sample->C, sample->N, acc, cutoff);
    acc = dist_x(T, N, sample->T, sample->N, acc, cutoff);
    acc = dist_x(G, N, sample->G, sample->N, acc, cutoff);
    return acc.size();
}

unordered_set<int> Sample::dist_x(vector<int> this_x, vector<int> this_n, vector<int> sample_x, vector<int> sample_n, unordered_set<int> acc, unsigned int cutoff){
    //Increment cutoff so we can reject distances > cutoff entirely
    cutoff++;
    for(const int &elem: this_x){
        if(acc.size() == cutoff){
            return acc;
        }
        if(!binary_search(sample_x.begin(), sample_x.end(), elem)){
            //Not a in sample
            if(!binary_search(sample_n.begin(), sample_n.end(), elem)){
                //Not an N comparison either, so add
                acc.insert(elem);
            }
        }
    }
    for(const int &elem: sample_x){
        if(acc.size() == cutoff){
            return acc;
        }
        if(!binary_search(this_x.begin(), this_x.end(), elem)){
            //Not a in sample
            if(!binary_search(this_n.begin(), this_n.end(), elem)){
                //Not an N comparison either, so add
                acc.insert(elem);
            }
        }
    }

    return acc;
}

void save_n(vector<int> to_save, string filename){
    fstream out(filename, fstream::binary | fstream::out);
    if(!out.good()){
        throw invalid_argument("Error writing save file: " + filename);
    }

    for(const int &elem: to_save){
        const int *p = &elem;
        const char *p_ = (char *) p;
        out.write(p_, 4);
    }

    out.close();
}

// DEPRECIATED
vector<int> load_n(string filename){
    //ate flag seeks to the end of the file
    fstream in(filename, fstream::binary | fstream::in | fstream::ate);
    if(!in.good()){
        throw invalid_argument("Invalid save path: " + filename);
    }
    //Get the size of the file in bytes (we have to convert this as 1 int == 4 bytes)
    int size = in.tellg();
    in.seekg(0); //Go back to the start so we can read

    vector<int> output;

    for(int j=0;j<size/4;j++){
        //Read 4 characters from the file into a char array
        char i[4];
        in.read(i, 4);

        //Convert the char array into an int and insert
        int *p;
        p = (int *) i;
        output.push_back(*p);
    }

    in.close();

    //Make sure this is sorted or binary search breaks
    sort(output.begin(), output.end());

    return output;
}

// For backwards compatibility
// Loads the old-style saves, saves to new format then clean up old save files
Sample* load_old(string filename){
    if(filename.at(filename.size()-1) == '/'){
        //Remove trailing / if req
        filename.pop_back();
    }
    const vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    vector<vector<int>> loading;
    for(unsigned int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        loading.push_back(load_n(f));
    }
    Sample *s = new Sample(loading.at(0), loading.at(1), loading.at(2), loading.at(3), loading.at(4));
    
    //Get the UUID from the filename
    string uuid;
    for(unsigned int i=0;i<filename.size();i++){
        if(filename.at(i) == '/'){
            //We only want the last part, so reset
            uuid = "";
        }
        else{
            uuid += filename.at(i);
        }
    }
    s->uuid = uuid;

    // We now have instanciated the old save, so write the new-style version and clean up
    string base_filename = filename.substr(0, filename.size()-uuid.size());
    save(base_filename, s);

    for(unsigned int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        std::filesystem::remove(f);
    }

    return s;
}

void save(string filename, Sample* sample){
    /**
    File format:
    Integers are written as binary integers (i.e 4 chars per int)
    Each set of `<>` below is a single integer
    Whitespace is for ease of reading, and is not included in the file

    ```
    <number of As in this file><value of A[0]><value of A[1]>...
    <number of Cs in this file><value of C[0]><value of C[1]>...
    <number of Gs in this file><value of G[0]><value of G[1]>...
    <number of Ts in this file><value of T[0]><value of T[1]>...
    <number of Ns in this file><value of N[0]><value of N[1]>...
    ```
    */
    if(!sample->qc_pass){
        //This sample has not passed QC, so don't save it
        cout << "||QC_FAIL: " << sample->uuid << "||" << endl;
        return;
    }
    //Append the sample UUID to the filename to save as such
    if(filename[filename.size()-1] != '/'){
        //No trailing / so add
        filename += '/';
    }
    filename += sample->uuid;
    filename = filename + ".fn5";

    const vector<char> types = {'A', 'C', 'G', 'T', 'N'};

    fstream out(filename, fstream::binary | fstream::out);
    if(!out.good()){
        throw invalid_argument("Error writing save file: " + filename);
    }

    for(unsigned int i=0; i<types.size(); i++){
        //Find out which one we need to save
        vector<int> toSave;
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
        // Ideally, these should already be sorted, but sort on save for clarity
        sort(toSave.begin(), toSave.end());

        // First, write the number of ints we are going to write to make reading possible
        // Annoying conversion as vector.size() is unsigned long
        int nc_size = (toSave.size() & INT_MAX);
        const int *p = &nc_size;
        const char *p_ = (char *) p;
        out.write(p_, 4);
        // Then, iterate over elements, writing to the file
        for(const int &elem: toSave){
            const int *p = &elem;
            const char *p_ = (char *) p;
            out.write(p_, 4);
        }
    }
    out.close();
}

Sample* readSample(string filename){
    // Check for old-style saves to update if required
    string ext = filename.substr(filename.find_last_of(".")+1);
    if(ext != "fn5" && ext != "FN5"){
        //  Not valid file extension for new saves so try load_old
        try{
            return load_old(filename);
        }
        catch (invalid_argument &err) {
            // Not valid so try version with added `.fn5`
            // Mostly covering the gpas API use case
            filename += ".fn5";
        }
    }
    // filename is <uuid>.fn5
    const vector<string> supported_extensions = {".fn5", ".FN5"};
    const vector<char> types = {'A', 'C', 'G', 'T', 'N'};

    // Keep track of the values we read
    vector<vector<int>> loading;

    // Index of loading and types this is looking at
    unsigned long int loading_idx = 0;

    // Number of ints to read from the file for this type
    int nc_size = -1;
    // Counter of number of ints already read for this type
    int current_size = 0;

    //ate flag seeks to the end of the file
    fstream in(filename, fstream::binary | fstream::in | fstream::ate);
    if(!in.good()){
        throw invalid_argument("Invalid save path: " + filename);
    }
    //Get the size of the file in bytes (we have to convert this as 1 int == 4 bytes)
    int size = in.tellg();
    in.seekg(0); //Go back to the start so we can read

    if(size % 4 != 0){
        // Should only be mutliples of 4 in the file
        throw invalid_argument("Malformed save file: " + filename);
    }

    // This nucleotide's values
    vector<int> acc;
    for(int j=0;j<size/4;j++){
        //Read 4 characters from the file into a char array
        char i[4];
        in.read(i, 4);

        //Convert the char array into an int and insert
        int *p;
        p = (int *) i;

        if(nc_size == -1){
            // Size of the nucleotide to fetch has not been set, so this must be it
            nc_size = *p;
            current_size = 0;
        }
        else{
            if(current_size == nc_size){
                // Matched required number of ints, so add save and treat this int as the next nc_size
                nc_size = *p;
                current_size = 0;

                loading.push_back(acc);
                acc.clear();

                loading_idx++;
                if(loading_idx > types.size()){
                    // Complain if this has happened incorrectly
                    throw invalid_argument("Malformed save file: " + filename);
                }
            }
            else{
                acc.push_back(*p);
                current_size++;
            }
        }
    }
    // Catch the last item
    loading.push_back(acc);

    in.close();

    Sample *s = new Sample(loading.at(0), loading.at(1), loading.at(2), loading.at(3), loading.at(4));
    
    //Get the UUID from the filename
    string uuid;
    if(filename.at(filename.size()-1) == '/'){
        //Remove trailing / if req
        filename.pop_back();
    }
    // Remove file extension if existing
    for(const string &ext: supported_extensions){
        std::size_t startpos = filename.find(ext);
        if(startpos != string::npos){
            // Extension found, so remove
            filename = filename.erase(filename.find(ext), ext.size());
        }
    }
    for(unsigned int i=0;i<filename.size();i++){
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

string load_reference(string filename){
    //Load reference
    string reference = "";
    char ch;
    fstream fin(filename, fstream::in);
    if(!fin.good()){
        throw invalid_argument("Invalid reference genome: " + filename);
    }

    //First line is the header, but for this we don't care about it
    while(fin >> noskipws >> ch){
        if(ch == '\n' || ch == '\r'){
            break;
        }
    }
    //Now just parse the file
    while (fin >> noskipws >> ch) {
        if(ch == '\n' || ch =='\r'){
            //We don't want these
            continue;
        }
        reference += ch;
    }
    
    fin.close();

    return reference;
}

unordered_set<int> load_mask(string filename){
    //Load the exclude mask
    unordered_set<int> mask;
    if(filename == "ignore"){
        //If the user has requested to ignore this, then do.
        return mask;
    }
    fstream fin2(filename, fstream::in);
    if(!fin2.good()){
        throw invalid_argument("Invalid mask path: " + filename);
    }
    string acc;
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

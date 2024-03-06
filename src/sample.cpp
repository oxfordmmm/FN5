#include "include/sample.hpp"
#include <stdexcept>

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
        if(ch == '|'){
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
                case 'N':
                    N.push_back(i);
                    break;
                case '-':
                    N.push_back(i);
                    break;
                case 'X':
                    N.push_back(i);
                    break;
                case 'O':
                    N.push_back(i);
                    break;
                case 'Z':
                    N.push_back(i);
                    break;
            }
        }
        i++;
    }
    fin.close();

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

void save(string filename, Sample* sample){
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
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    for(unsigned int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
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
        save_n(toSave, f);
    }
}

Sample* readSample(string filename){
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    vector<vector<int>> loading;
    for(unsigned int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        loading.push_back(load_n(f));
    }
    Sample *s = new Sample(loading.at(0), loading.at(1), loading.at(2), loading.at(3), loading.at(4));
    
    //Get the UUID from the filename
    string uuid;
    if(filename.at(filename.size()-1) == '/'){
        //Remove trailing / if req
        filename.pop_back();
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

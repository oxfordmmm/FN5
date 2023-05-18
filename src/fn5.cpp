#include "include/sample.hpp"
#include "include/argparse.hpp"
#include "include/comparisons.hpp"

using namespace std;


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
    if(check_flag(args, "--reference")){
        ref_genome_path = args.at("--reference");
    }
    if(check_flag(args, "--mask")){
        exclude_mask_path = args.at("--mask");
    }
    if(check_flag(args, "--debug")){
        if(args.at("--debug") != "0"){
            debug = true;
        }
    }


    int cutoff = 20;
    if(check_required(args, {"--cutoff"})){
        //Take the user's req of a cutoff if given
        cutoff = stoi(args.at("--cutoff"));
    }

    //Check for compute first as it doesn't need reference
    if(check_flag(args, "--compute")){
        vector<Sample*> samples = load_saves_multithreaded();
        compute_loaded(cutoff, samples);
        return 0;
    }

    string reference = load_reference(ref_genome_path);
    
    unordered_set<int> mask = load_mask(exclude_mask_path);

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
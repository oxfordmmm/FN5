#include "include/sample.hpp"
#include "include/argparse.hpp"
#include "include/comparisons.hpp"

using namespace std;


int main(int nargs, const char* args_[]){
    if(nargs == 2){
        string val = args_[1];
        if(val == "--version" || val == "-v"){
            cout << "v2.0.6" << endl;
            return 0;
        }
    }
    map<string, string> args = parse_args(nargs, args_);

    //Check to see if values are supplied or we need to use default
    if(check_flag(args, "--threads")){
        thread_count = stoi(args.at("--threads"));
        //Make sure we have >=1 thread to avoid issues
        if(thread_count < 1){
            thread_count = 1;
        }
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

    if(check_flag(args, "--add_batch")){
        add_batch(args.at("--add_batch"), cutoff);
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

    if(check_flag(args, "--reference_compress")){
        string guid = "";
        if(check_flag(args, "--guid")){
            //If guid is given as an arg, use it
            //If not, just use whatever can be found from the header
            guid = args.at("--guid");
        }
        reference_compress(args.at("--reference_compress"), reference, mask, guid);
    }

}
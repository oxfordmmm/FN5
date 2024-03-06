#include "include/argparse.hpp"


using namespace std;

map<string, string> parse_args(int nargs, const char* args[]){
    map<string, string> arguments;
    string current_flag;
    for(int i=1;i<nargs;i++){
        if(current_flag == ""){
            //This item should be a new flag
            current_flag = args[i];
        }
        else{
            //We already have a flag loaded so get the value
            string val = args[i];
            arguments.insert({current_flag, val});
            current_flag = "";
        }
    }
    return arguments;
}

bool check_flag(map<string, string> args, string flag){
    for (auto const& [key, val] : args){
        if(key == flag){
            return true;
        }
    }
    return false;
}

bool check_required(map<string, string> args, vector<string> required){
    for(unsigned int i=0;i<required.size();i++){
        if(!check_flag(args, required.at(i))){
            //Doesn't have a required flag, so return
            return false;
        }
    }
    return true;
}

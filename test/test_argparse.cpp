#include <gtest/gtest.h>
#include "../src/include/argparse.hpp"
#include <vector>

using namespace std;

// This is more difficult than it seems as constructing `const char* []` isn't trivial...
// Not the most important part though, so TODO


// const char** to_args(vector<string> s){
//     // Convert vector of strings to char** for use with arg functions
//     char** out;
//     for(int i=0;i<s.size();i++){
//         char* chars = s.at(i).c_str();
//         out[i] = chars;
//     }
//     return out;
// }

// TEST(argparse, parse_args){
//     vector<string> args = {"--flag_1", "arg_1", "--flag_2", "arg_2", "--flag_3", "arg_3"};
//     map<string, string> expected_args = {{"--flag_1", "arg_1"}, {"--flag_2", "arg_2"}, {"--flag_3", "arg_3"}};
//     map<string, string> actual_args = parse_args(args.size(), to_args(args));
//     ASSERT_EQ(expected_args, actual_args);
// }
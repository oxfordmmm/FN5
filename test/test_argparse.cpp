#include <gtest/gtest.h>
#include "../src/include/argparse.hpp"
#include <vector>

using namespace std;

/**
* @brief Wrapper for `parse_args` to allow vector<string> instead of const char*[]
*
* @param strings Vector of strings to treat as arguments
* @return Parsed arguments
*/
map<string, string> to_args(vector<string> strings){
    // Convert vector of strings to char** for use with parse_args
    vector<char*> cstrings;
    cstrings.reserve(strings.size());

    for(size_t i = 0; i < strings.size(); ++i)
        cstrings.push_back(const_cast<char*>(strings[i].c_str()));
    
    const char** out = const_cast<const char**>(&cstrings[0]);

    return parse_args(strings.size(), out);
}

/**
* @brief Test `parse_args`
*/
TEST(argparse, parse_args){
    vector<string> args = {"./fn5", "--flag_1", "arg_1", "--flag_2", "arg_2", "--flag_3", "arg_3"};
    map<string, string> expected_args = {{"--flag_1", "arg_1"}, {"--flag_2", "arg_2"}, {"--flag_3", "arg_3"}};
    map<string, string> actual_args = to_args(args);
    ASSERT_EQ(expected_args, actual_args);
}

/**
* @brief Test `check_flag`
*/
TEST(argparse, check_flag){
    map<string, string> args = {{"--flag_1", "arg_1"}, {"--flag_2", "arg_2"}, {"--flag_3", "arg_3"}};
    ASSERT_TRUE(check_flag(args, "--flag_1"));
    ASSERT_TRUE(check_flag(args, "--flag_2"));
    ASSERT_TRUE(check_flag(args, "--flag_3"));

    ASSERT_FALSE(check_flag(args, "--flag_4"));
    ASSERT_FALSE(check_flag(args, "--not_a_flag"));
    ASSERT_FALSE(check_flag(args, "aaaaaaaaaaaaa"));
    ASSERT_FALSE(check_flag(args, "17"));
    ASSERT_FALSE(check_flag(args, "arg1"));
    ASSERT_FALSE(check_flag(args, "--flag1"));
}

/**
* @brief Test `check_required`
*/
TEST(argparse, check_required){
    map<string, string> args = {{"--flag_1", "arg_1"}, {"--flag_2", "arg_2"}, {"--flag_3", "arg_3"}};

    ASSERT_TRUE(check_required(args, {"--flag_1"}));
    ASSERT_TRUE(check_required(args, {"--flag_2"}));
    ASSERT_TRUE(check_required(args, {"--flag_3"}));
    ASSERT_TRUE(check_required(args, {"--flag_1", "--flag_2"}));
    ASSERT_TRUE(check_required(args, {"--flag_1", "--flag_3"}));
    ASSERT_TRUE(check_required(args, {"--flag_2", "--flag_3"}));
    ASSERT_TRUE(check_required(args, {"--flag_1", "--flag_2", "--flag_3"}));
    ASSERT_TRUE(check_required(args, {"--flag_3", "--flag_2", "--flag_1"}));
    ASSERT_TRUE(check_required(args, {"--flag_2", "--flag_1", "--flag_3"}));
    ASSERT_TRUE(check_required(args, {"--flag_1", "--flag_1", "--flag_1"}));

    ASSERT_FALSE(check_required(args, {"--flag_1", "not_a_flag"}));
    ASSERT_FALSE(check_required(args, {"--flag1", "not_a_flag"}));
    ASSERT_FALSE(check_required(args, {"1", "aaaaa"}));
    ASSERT_FALSE(check_required(args, {"--", "lorem ipsum"}));
    ASSERT_FALSE(check_required(args, {"421", "k", "l", "mnpo"}));

}

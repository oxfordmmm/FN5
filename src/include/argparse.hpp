#pragma once
#include <map>
#include <string>
#include <vector>

using namespace std;


/**
* @brief Parse arguments into a map of strings. Based on assumption of 1 arg per flag
*
* @param nargs Number of arguments
* @param args Arguments
* @returns map of arguments by flags. Both flag and arg as strings
*/
map<string, string> parse_args(int nargs, const char* args[]);

/**
* @brief Check if parsed arguments contain a given flag
*
* @param args Arguments
* @param flag Flag to check for
* @returns bool True if the flag exists
*/
bool check_flag(map<string, string> args, string flag);

/**
* @brief Check if parsed arguments contain all of the required flags
*
* @param args Arguments
* @param required Vector of flags to check for
* @returns bool True if all flags exist
*/
bool check_required(map<string, string> args, vector<string> required);

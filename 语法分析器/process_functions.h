#pragma once

#include "structs.h"

using namespace std;

vector<string> operator+(const vector<string>& a, const vector<string>& b);

vector<string> split(string s, string delimiter);

vector<Token> read_tokens_from_file(string filename);

bool operator==(const item& a, const item& b);

bool operator!=(const item& a, const item& b);

bool operator == (const item_cluster& a, const item_cluster& b);

vector<string> remove_duplicates(vector<string> input);

read_result read_grammar_and_first(string filename);

bool null_exist(first s);

map<string, first> process_first_map(map<string, first> init_first_map, map<string, symbol> symbol_map, map<string, set<string>> first_deduced);

map<string, set<string>> first_deduced_map(vector<grammar> grammar_vec, map<string, first> init_first_map);
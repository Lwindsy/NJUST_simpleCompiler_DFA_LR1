#pragma once

#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>

#include "structs.h"

//find the No.n P in T
int pos(string T, string P, int n);

//well done
vector<Grammar> load_grammar(string filename);

vector<DFA> get_dfa_list();

Token DFA_read(DFA dfa, string line, int& index);

list<Token> read_code(vector<DFA> dfa_list, string filename);

void search_anno(int& index,string line);
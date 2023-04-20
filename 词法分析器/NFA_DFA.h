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

using namespace std;

class NFA {
public:
	// type defines what type of word the NFA can identify.And type also is the starting node's value of each NFA.
	string type;
	set<string> letter_set;
	unordered_map<string, NFA_node*> status_set;
	//unordered_set<NFA_node*> NFA_node_set;// maybe useless

	NFA(string grammar_filename);

	// Function to transform the Grammar_list into the member NFA_node_set
	void transform_grammar_to_NFA(vector<Grammar> grammar_list);
};

class DFA {
public:
	string type;
	// node's name to a node
	map<string, DFA_node> node_set;

	DFA(NFA nfa);

	bool if_terminal(vector<string> subset);

	vector<string> find_unmarked(map<vector<string>, bool> C);

	vector<string> closure(vector<string> input_nodes, NFA nfa);

	vector<string> move(vector<string> T, NFA nfa, string letter);
};
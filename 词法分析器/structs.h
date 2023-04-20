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

using namespace std;

struct Grammar {
	string left;
	string right_lower;
	string right_upper;
	bool include_upper;
};

struct NFA_node {
	string value = "";
	vector<pair<string, NFA_node*>> edge_list = {};
	bool if_terminal = false;
	bool if_inception = false;
};

struct DFA_node {
	string value = "";
	// first weight then second node's name.
	vector<pair<string, string>> edge_list = {};
	bool if_terminal = false;
};

struct Token {
	// false means the token is empty and invalid
	//bool valid = true;
	int line;
	string type;
	string value;
};
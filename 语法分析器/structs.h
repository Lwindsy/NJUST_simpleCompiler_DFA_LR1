#pragma once

#include <string>

//error occurs when omitting "extern".
extern int item_id;
extern int item_cluster_id;

using namespace std;

struct Token {
	int line = -1;
	string type;
	string value;
};

struct symbol {
	string value;
	bool if_not_terminator = false;
};

struct grammar {
	string left;
	vector<symbol> right;
};

struct first {
	string value;
	set<string> first_set;
};

struct item {
	int id;
	int pos;
	int length;
	vector<symbol> right;
	string left;
	vector<string> search;

	item() {
		length = 0;
		pos = -1;
		id = item_id++;
		left = "";
		right = {};
		search = {};
	}
};

struct item_cluster {
	int id;
	// int is item's id
	map<int, item> item_map;
	vector<item> item_vec;
	item_cluster() {
		id = item_cluster_id++;
	}
};

struct read_result {
	vector<grammar> grammar_vec;
	map<string, first> first_map;
	map<string, symbol> symbol_map;
};

struct action {
	string type = "error";
	int to = -1;
	item reduce = item();
};

struct ic_node
{
	int ic_id;
	//weight and end's id
	map<string, int> edge_list;
	vector<item> reduction_list;
	ic_node() {
		ic_id = -1;
	}
	ic_node(item_cluster ic) {
		ic_id = ic.id;
		for (auto i : ic.item_vec) {
			if (i.pos == i.length) {
				reduction_list.push_back(i);
			}
		}
	}
};
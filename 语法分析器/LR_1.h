#pragma once

#include "structs.h"
#include "process_functions.h"

class LR_1 {
public:
	map<int, item_cluster> ic_cluster;//build in get_ic_cluster
	set<int> ic_Id;//build in get_ic_cluster (maybe useless)
	vector<item_cluster> ic_vec; //build in get_ic_cluster
	map<string, symbol> symbol_map;//bulid in constructor
	map<string, vector<item>> sameleft_item_set;//bulid in constructor
	map<string, first> first_map;  //(maybe useless)
	map<string, bool> if_deduct_null;
	//node corresponded to a ic
	map<int, ic_node> node_list;// build in get_new_ic
	map<string, set<string>> first_deduced;//  (maybe useless)

	map<string, action> ACTION;//bulid in constructor
	map<string, int> GOTO;//bulid in constructor

	// id and name to create key
	string action_or_goto_key(int i, const string& s);

	LR_1(map<string, symbol> sb_map, vector<grammar> grammar_list, map<string, first> f_map, map<string, set<string>> fs_deduced);

	vector<string> get_first_set(vector<string> a);

	void get_ic_cluster();

	inline int get_ic_id(item_cluster a);

	vector<item_cluster> get_new_ic(item_cluster& origin, map<int, int>& ic_done);

	item_cluster get_ic(vector<item> start);

	item grammar_to_item(grammar g);

	string error_check();

	string Syntax_Analysis(string filename);
};
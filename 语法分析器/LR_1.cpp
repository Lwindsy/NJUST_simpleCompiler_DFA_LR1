#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <stack>
#include <queue>

#include "LR_1.h"

// id and name to create key
string LR_1::action_or_goto_key(int i, const string& s) {
	return to_string(i) + "," + s;
}

LR_1::LR_1(map<string, symbol> sb_map, vector<grammar> grammar_list, map<string, first> f_map, map<string, set<string>> fs_deduced) {
	symbol_map = sb_map;
	first_map = f_map;
	first_deduced = fs_deduced;
	for (grammar g : grammar_list) {
		// without search!!!
		sameleft_item_set[g.left].push_back(grammar_to_item(g));
		if_deduct_null[g.left] = false;
	}
	bool changed = false;
	do {
		changed = false;
		for (auto& m : sameleft_item_set) {
			for (auto& i : m.second) {
				if (i.right[0].value == "$") {

					if (!if_deduct_null[i.left])
						changed = true;
					if_deduct_null[i.left] = true;
					continue;
				}
				for (int z = 0; z < i.length; z++) {
					symbol r = i.right[z];

					if (!r.if_not_terminator || if_deduct_null[r.value]) {
						break;
					}
					if (!if_deduct_null[i.left])
						changed = true;
					if_deduct_null[i.left] = true;
				}
			}
		}
	} while (changed);

	get_ic_cluster();

	for (auto m : node_list) {
		ic_node node = m.second;
		for (auto edge : node.edge_list) {
			string key = action_or_goto_key(node.ic_id, edge.first);
			action a;
			if (symbol_map[edge.first].if_not_terminator) {
				GOTO[key] = edge.second;
			}
			else {
				a.type = "S";
				a.to = edge.second;
				ACTION[key] = a;
			}
		}
		for (auto reduce : node.reduction_list) {
			action a;
			for (auto r : reduce.search) {
				string key = action_or_goto_key(node.ic_id, r);
				a.type = "r";
				a.reduce = reduce;
				ACTION[key] = a;
			}
			if (reduce.left == "Inception" && reduce.pos == reduce.length) {
				string key = action_or_goto_key(node.ic_id, "#");
				ACTION[key].type = "acc";
			}
		}
	}
}

vector<string> LR_1::get_first_set(vector<string> a) {
	vector<string> result;
	for (auto& s : a) {
		symbol symbol = symbol_map[s];
		if (!symbol.if_not_terminator) {
			result.push_back(s);
			break;
		}
		else {
			if (!if_deduct_null[symbol.value]) {
				break;
			}
			else {
				first f = first_map[symbol.value];
				for (auto f_f : f.first_set) {
					result.push_back(f_f);
				}
			}
		}
	}
	return result;
}

void LR_1::get_ic_cluster() {
	//first process the Inception
	item_cluster Inception = get_ic(sameleft_item_set["Inception"]);
	ic_cluster[Inception.id] = Inception;
	ic_Id.insert(Inception.id);
	ic_vec.push_back(Inception);
	node_list[Inception.id] = ic_node(Inception);

	queue<item_cluster> q;
	q.push(Inception);

	map<int, int> ic_done;
	ic_done[Inception.id] = 0;

	while (!q.empty()) {
		item_cluster cur_i = q.front();
		q.pop();
		if (ic_done[cur_i.id]) {
			continue;
		}
		vector<item_cluster> new_ic_vec = get_new_ic(cur_i, ic_done);
		for (auto new_ic : new_ic_vec) {
			ic_cluster[new_ic.id] = new_ic;
			ic_Id.insert(new_ic.id);
			//ic_vec.push_back(new_ic);
			//node_list[new_ic.id] = ic_node(new_ic);
			if (ic_done[new_ic.id] == 0) {
				q.push(new_ic);
			}
		}
		ic_done[cur_i.id] = 1;
	}
}

inline int LR_1::get_ic_id(item_cluster a) {
	for (auto i : ic_vec) {
		if (a == i) {
			return i.id;
		}
	}
	return a.id;
}

vector<item_cluster> LR_1::get_new_ic(item_cluster& origin, map<int, int>& ic_done) {

	vector<item_cluster> result;
	set<string> cur_symbol_str_set;
	for (int i = 0; i < origin.item_vec.size(); i++) {
		if (origin.item_vec[i].pos == origin.item_vec[i].length) {
			continue;
		}
		cur_symbol_str_set.insert(origin.item_vec[i].right[origin.item_vec[i].pos].value);
	}
	if (!cur_symbol_str_set.empty()) {
		for (auto s : cur_symbol_str_set) {
			vector<item> new_ic_start;
			for (int i = 0; i < origin.item_vec.size(); i++) {
				item cur = origin.item_vec[i];
				if (cur.pos < cur.length && cur.right[cur.pos].value == s) {
					cur.pos += 1;
					new_ic_start.push_back(cur);
				}
			}
			item_cluster new_ic = get_ic(new_ic_start);

			if (new_ic.id == get_ic_id(new_ic)) {
				ic_done[new_ic.id] = 0;
				ic_vec.push_back(new_ic);
				node_list[new_ic.id] = ic_node(new_ic);
			}
			new_ic.id = get_ic_id(new_ic);

			node_list[origin.id].edge_list[s] = new_ic.id;

			result.push_back(new_ic);
		}
	}
	return result;
}

item_cluster LR_1::get_ic(vector<item> start) {
	item_cluster result;
	queue<item> q;
	set<string> already_added;
	for (auto i : start) {
		q.push(i);
	}
	while (!q.empty()) {
		item i = q.front();
		q.pop();

		if (i.right[0].value == "$") {
			i.pos = 0;
			i.right.clear();
			i.length = 0;
		}
		// expand
		if (i.pos < i.length && i.right[i.pos].if_not_terminator && (already_added.find(i.right[i.pos].value) == already_added.end())) {
			for (item j : sameleft_item_set[i.right[i.pos].value]) {
				j.id = i.id;
				vector<string> beita;
				for (int index = i.pos + 1; index < i.length; index++) {
					beita.push_back(i.right[index].value);
				}

				j.search = get_first_set(beita);


				bool if_beita_deduct_null = false;
				for (int i = 0; i < beita.size(); i++) {
					string b = beita[i];
					if (!symbol_map[b].if_not_terminator || !if_deduct_null[b]) {
						break;
					}
					if_beita_deduct_null = true;
				}

				if (if_beita_deduct_null || beita.size() == 0) {
					for (int z = 0; z < i.search.size(); z++) {
						j.search.push_back(i.search[z]);
					}
				}

				j.search = remove_duplicates(j.search);
				q.push(j);
			}
			already_added.insert(i.right[i.pos].value);
		}
		if (i.pos == i.length) {
			for (auto& nt_deduced : first_deduced[i.left]) {
				i.search.push_back(nt_deduced);
			}
		}
		result.item_vec.push_back(i);
		result.item_map[i.id] = i;
	}
	return result;
}

item LR_1::grammar_to_item(grammar g) {
	item i = item();
	i.left = g.left;
	if (i.left == "Inception") {
		i.search.push_back("#");
	}
	i.length = g.right.size();
	i.pos = 0;
	i.right = g.right;
	return i;
}

string LR_1::error_check() {
	return "error!";
}

string LR_1::Syntax_Analysis(string filename) {

	string result;

	vector<Token> tokens = read_tokens_from_file(filename);
	Token final;
	final.value = "#";
	tokens.push_back(final);
	stack<int> status;
	stack<string> sym;
	sym.push("#");
	status.push(0);

	bool if_goto = false;
	for (int i = 0; i < tokens.size(); i++) {
		Token cur_token = tokens[i];
		int cur_status = status.top();
		string cur_symbol;
		if (symbol_map.find(cur_token.type) != symbol_map.end()) {
			cur_symbol = cur_token.type;
		}
		else
			if (symbol_map.find(cur_token.value) != symbol_map.end()) {
				cur_symbol = cur_token.value;
			}
			else
				if (cur_token.value == "#") {
					cur_symbol = "#";
				}
		string cur_key = action_or_goto_key(cur_status, cur_symbol);
		// move in
		if (ACTION[cur_key].to != -1) {
			sym.push(cur_symbol);
			status.push(ACTION[cur_key].to);

		}
		else
		{
			// reduce
			if (ACTION[cur_key].reduce.pos != -1) {
				item reduce = ACTION[cur_key].reduce;

				if (ACTION[cur_key].type == "acc") {
					result = "Success!";
					return result;
				}
				else {
					for (int i = 0; i < reduce.length; i++) {
						sym.pop();
						status.pop();
					}
					string after_reduce_key = action_or_goto_key(status.top(), reduce.left);
					if (GOTO.find(after_reduce_key) != GOTO.end()) {
						sym.push(reduce.left);
						status.push(GOTO[after_reduce_key]);
						i--;
					}
					else {
						result = error_check();
						return result;
					}
				}
			}
			else {
				//if (GOTO[cur_symbol]) {

				//}
				////empty
				//else
				//{
				result = error_check();
				return result;
				//}
			}

		}
	}

	return result;
}
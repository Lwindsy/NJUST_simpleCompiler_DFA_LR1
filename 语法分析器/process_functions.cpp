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

#include "process_functions.h"

using namespace std;

vector<string> operator+(const vector<string>& a, const vector<string>& b) {
	vector<string> result = a;
	for (auto s : b) {
		result.push_back(s);
	}
	return result;
}

vector<string> split(string s, string delimiter) {
	vector<string> result;
	size_t pos = 0;
	while ((pos = s.find(delimiter)) != string::npos) {
		result.push_back(s.substr(0, pos));
		s.erase(0, pos + delimiter.length());
	}
	result.push_back(s);
	return result;
}

vector<Token> read_tokens_from_file(string filename) {
	vector<Token> result;
	ifstream file(filename);
	string line;
	while (getline(file, line)) {
		Token token;
		vector<string> line_parts = split(line, ", ");
		token.line = stoi(split(line_parts[0], ": ")[1]);
		token.type = split(line_parts[1], ": ")[1];
		token.value = split(split(line_parts[2], ": ")[1], "]")[0];
		result.push_back(token);
	}
	return result;
}

bool operator==(const item& a, const item& b) {
	if (a.pos == b.pos && a.length == b.length && a.left == b.left && a.search == b.search && a.right.size() == b.right.size()) {
		for (int i = 0; i < a.right.size(); i++) {
			if (a.right[i].value != b.right[i].value || a.right[i].if_not_terminator != b.right[i].if_not_terminator) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool operator!=(const item& a, const item& b) {
	return !(a == b);
}

bool operator == (const item_cluster& a, const item_cluster& b) {
	if (a.item_vec.size() == b.item_vec.size()) {
		for (int i = 0; i < a.item_vec.size(); i++) {
			if (!(a.item_vec[i] == b.item_vec[i])) {
				return false;
			}
		}
		return true;
	}
	return false;
}

vector<string> remove_duplicates(vector<string> input) {
	set<string> unique_elements;
	for (string element : input) {
		unique_elements.insert(element);
	}
	vector<string> result;
	for (string element : unique_elements) {
		result.push_back(element);
	}
	return result;
}

read_result read_grammar_and_first(string filename) {

	read_result result;

	map<string, symbol> symbol_map;
	vector<grammar> grammar_vec;
	map<string, first> first_map;
	ifstream file(filename);
	string line;
	while (getline(file, line)) {
		// pass empty line
		if (line.empty()) {
			continue;
		}
		grammar g;
		int arrow_pos = line.find("->");
		g.left = line.substr(1, arrow_pos - 3);
		string right_str = line.substr(arrow_pos + 3);
		int pos = 0;
		while (pos < right_str.length()) {
			symbol s;
			if (right_str[pos] == '<') {
				int end_pos = right_str.find('>', pos);
				s.value = right_str.substr(pos + 1, end_pos - pos - 1);
				s.if_not_terminator = (s.value.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != string::npos);
				pos = end_pos + 1;
			}
			// defaultly read one letter as non-terminal.
			else {
				int end_pos = pos + 1;
				// specially analyzes ++ and -- and += and -=
				if (end_pos < right_str.length() && ((right_str[end_pos] == '=' && right_str[end_pos - 1] == '-') || (right_str[end_pos] == '=' && right_str[end_pos - 1] == '+') || (right_str[end_pos] == '+' && right_str[end_pos - 1] == '+') || (right_str[end_pos] == '-' && right_str[end_pos - 1] == '-'))) {
					end_pos++;
				}
				s.value = right_str.substr(pos, end_pos - pos);
				s.if_not_terminator = false;
				pos = end_pos;
			}
			g.right.push_back(s);
			symbol_map[s.value] = s;
		}
		grammar_vec.push_back(g);

		for (auto& s : grammar_vec) {
			for (auto& a : s.right) {
				if (first_map.find(a.value) == first_map.end()) {
					first f;
					f.value = a.value;
					first_map[a.value] = f;
				}
				for (int i = 0; i < s.right.size(); i++) {
					if (s.right[i].value == a.value && i + 1 < s.right.size()) {
						if (first_map[a.value].first_set.find(s.right[i + 1].value) == first_map[a.value].first_set.end()) {
							first_map[a.value].first_set.insert(s.right[i + 1].value);
						}
					}
				}
			}
		}
	}
	// remove all none_terminal
	for (auto& m : first_map) {
		vector<string> to_erase;
		for (auto& f : m.second.first_set) {
			if (symbol_map[f].if_not_terminator) {
				to_erase.push_back(f);
			}
		}
		for (string s : to_erase) {
			m.second.first_set.erase(s);
		}
	}
	file.close();
	result.grammar_vec = grammar_vec;
	result.first_map = first_map;
	result.symbol_map = symbol_map;
	return result;
}

bool null_exist(first s) {
	for (auto& a : s.first_set) {
		if (a == "$")
			return true;
	}
	return false;
}

map<string, first> process_first_map(map<string, first> init_first_map, map<string, symbol> symbol_map, map<string, set<string>> first_deduced) {
	map<string, first> original = init_first_map;
	for (int i = 0; ; i++) {
		bool done = true;
		if (i == 0) {
			for (auto& f : init_first_map) {
				queue<string> q;
				for (auto& f_fs : f.second.first_set) {
					if (symbol_map[f_fs].if_not_terminator) {
						q.push(f_fs);
					}
				}
				if (symbol_map[f.first].if_not_terminator) {
					for (auto& nt_deduced : first_deduced[f.first])
						f.second.first_set.insert(nt_deduced);
				}
				while (!q.empty()) {
					string nt = q.front();
					q.pop();
					for (auto& q_nt_fs : original[nt].first_set) {
						f.second.first_set.insert(q_nt_fs);
					}
					if (symbol_map[nt].if_not_terminator) {
						for (auto& nt_deduced : first_deduced[nt])
							f.second.first_set.insert(nt_deduced);
					}
				}
			}
		}
		// consider $
		else {
			done = true;
			for (auto index = init_first_map.begin(); index != init_first_map.end(); index++) {
				for (auto& f_fs : index->second.first_set) {
					if (null_exist(init_first_map[index->first]) && symbol_map[f_fs].if_not_terminator) {
						for (auto& f_null_fs : first_deduced[f_fs]) {
							if (index->second.first_set.find(f_null_fs) == index->second.first_set.end()) {
								done = false;
							}
							index->second.first_set.insert(f_null_fs);
						}
					}
				}
			}
			if (done) break;
		}
	}
	/*for (auto& f : init_first_map) {
		vector<string> wait_for_erase;
		for (auto& f_fs : f.second.first_set) {
			if (symbol_map[f_fs].if_not_terminator) {
				wait_for_erase.push_back(f_fs);
			}
		}
		for (string s : wait_for_erase)
			f.second.first_set.erase(s);
	}*/
	return init_first_map;
}

map<string, set<string>> first_deduced_map(vector<grammar> grammar_vec, map<string, first> init_first_map) {
	map<string, set<string>> result;
	for (auto& g : grammar_vec) {
		if (!g.right.empty()) {
			result[g.left].insert(g.right[0].value);
			for (int i = 1; i < g.right.size(); i++) {
				if (null_exist(init_first_map[g.right[i].value])) {
					result[g.left].insert(g.right[i].value);
				}
			}
		}
	}
	return result;
}
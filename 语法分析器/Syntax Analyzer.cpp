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

using namespace std;

int item_id = 0;
int item_cluster_id = 0;

vector<string> operator+(const vector<string>& a, const vector<string>& b) {
	vector<string> result = a;
	for (auto s : b) {
		result.push_back(s);
	}
	return result;
}

struct Token {
	int line;
	string type;
	string value;
};

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

struct symbol {
	string value;
	bool if_not_terminator;
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

struct item_cluster {
	int id;
	// int is item's id
	map<int, item> item_map;
	vector<item> item_vec;
	item_cluster() {
		id = item_cluster_id++;
	}
};

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

struct action {
	string type = "error";
	int to = -1;
	item reduce = item();
};

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

	// id and name to create key
	string action_or_goto_key(int i, const string& s) {
		return to_string(i) + "," + s;
	}

	map<string, action> ACTION;//bulid in constructor
	map<string, int> GOTO;//bulid in constructor

	LR_1(map<string, symbol> sb_map, vector<grammar> grammar_list, map<string, first> f_map, map<string, set<string>> fs_deduced) {
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

	vector<string> get_first_set(vector<string> a) {
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

	void get_ic_cluster() {
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

	inline int get_ic_id(item_cluster a) {
		for (auto i : ic_vec) {
			if (a == i) {
				return i.id;
			}
		}
		return a.id;
	}

	vector<item_cluster> get_new_ic(item_cluster& origin, map<int, int>& ic_done) {

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

	item_cluster get_ic(vector<item> start) {
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

	item grammar_to_item(grammar g) {
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

	string error_check() {
		return "error!";
	}

	string Syntax_Analysis(string filename) {

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
};

struct read_result {
	vector<grammar> grammar_vec;
	map<string, first> first_map;
	map<string, symbol> symbol_map;
};

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

int main() {
	auto read_result = read_grammar_and_first("syntactic_grammar.txt");
	map<string, first> init_first_map = read_result.first_map;
	vector<grammar> grammar_vec = read_result.grammar_vec;
	map<string, symbol> symbol_map = read_result.symbol_map;

	map<string, set<string>> first_deduced = first_deduced_map(grammar_vec, init_first_map);

	map<string, first> first_map = process_first_map(init_first_map, symbol_map, first_deduced);



	LR_1 lr1_analyzor = LR_1(symbol_map, grammar_vec, first_map, first_deduced);

	cout << lr1_analyzor.Syntax_Analysis("Token_list.txt") << endl;

	bool cout_ic = false;
	bool cout_ic_node = false;
	bool cout_ACTION = false;
	bool cout_GOTO = false;
	bool cout_first_deduced = false;
	bool cout_init_first_map = false;
	bool cout_first_map = false;
	bool cout_grammar_vec = false;
	bool cout_symbol_map = false;

	if (cout_ic)
	{
		cout << "--------------------------------------------" << endl;

		for (auto ic : lr1_analyzor.ic_vec) {
			cout << ic.id << " : " << endl;
			for (auto item : ic.item_vec) {
				cout << "        " << item.left << " -> ";
				for (auto r : item.right) {
					if (r.if_not_terminator) {
						cout << " <" << r.value << ">";
					}
					else {
						cout << " " << r.value;
					}
				}
				cout << "  pos: " << item.pos << "  len: " << item.length << " search: ";
				for (auto r : item.search) {
					cout << r << " ";
				}
				cout << endl;
			}
		}
	}
	if (cout_ic_node)
	{
		cout << "--------------------------------------------" << endl;

		for (auto m : lr1_analyzor.node_list) {
			cout << "ID: " << m.first << " / " << m.second.ic_id << endl;
			cout << " GO: " << endl;
			for (auto go : m.second.edge_list) {
				cout << "     " << m.first << " --`" << go.first << "`--> " << go.second << endl;
			}
			cout << " REDUCE: " << endl;
			for (auto reduce : m.second.reduction_list) {
				cout << "        " << reduce.left << " -> ";
				for (auto r : reduce.right) {
					if (r.if_not_terminator) {
						cout << " <" << r.value << ">";
					}
					else {
						cout << " " << r.value;
					}
				}
				cout << "  pos: " << reduce.pos << "  len: " << reduce.length << " search: ";
				for (auto r : reduce.search) {
					cout << r << " ";
				}
				cout << endl;
			}
			cout << endl;
		}
	}
	if (cout_ACTION) {
		cout << "--------------------------------------------" << endl;

		for (auto a : lr1_analyzor.ACTION) {
			cout << a.first << "  type: " << a.second.type << endl;
			if (a.second.type == "r" || a.second.type == "acc") {
				item reduce = a.second.reduce;
				cout << "        " << reduce.left << " -> ";
				for (auto r : reduce.right) {
					if (r.if_not_terminator) {
						cout << " <" << r.value << ">";
					}
					else {
						cout << " " << r.value;
					}
				}
				cout << "  pos: " << reduce.pos << "  len: " << reduce.length << " search: ";
				for (auto r : reduce.search) {
					cout << r << " ";
				}
				cout << endl;
			}
			else {
				cout << a.second.to << endl;
			}
		}
	}
	if (cout_GOTO) {
		cout << "--------------------------------------------" << endl;

		for (auto a : lr1_analyzor.GOTO) {
			cout << a.first << "   " << a.second << endl;
		}
	}
	if (cout_first_deduced) {
		cout << "--------------------------------------------" << endl;

		for (auto& s : first_deduced) {
			cout << "key: " << s.first << " value: " << endl;
			for (auto& z : s.second) {
				cout << "                  " << z << endl;
			}
		}
	}
	if (cout_init_first_map) {
		cout << endl << "-------------------------------------------------------" << endl;

		for (auto& s : init_first_map) {
			cout << "key: " << s.first << " value: " << s.second.value << endl;
			for (auto& z : s.second.first_set) {
				cout << "                  " << z << endl;
			}
		}
	}
	if (cout_first_map) {
		cout << endl << "-------------------------------------------------------" << endl;

		for (auto& s : first_map) {
			cout << "key: " << s.first << " value: " << s.second.value << endl;
			for (auto& z : s.second.first_set) {
				cout << "                  " << z << endl;
			}
		}
	}
	if (cout_grammar_vec) {
		cout << endl << "-------------------------------------------------------" << endl;

		for (auto& s : grammar_vec) {
			cout << s.left << " -> ";
			for (auto& a : s.right) {
				cout << a.value << "(" << a.if_not_terminator << ") ";
			}
			cout << endl;
		}
	}
	if (cout_symbol_map) {
		cout << endl << "-------------------------------------------------------" << endl;

		for (auto& f : symbol_map) {
			cout << f.first << " : " << f.second.value << endl;
		}
	}
}
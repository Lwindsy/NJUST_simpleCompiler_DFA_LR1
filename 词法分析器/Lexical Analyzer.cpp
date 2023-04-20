#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>

constexpr auto grammar_number = 17;

using namespace std;

struct Grammar {
	string left;
	string right_lower;
	string right_upper;
	bool include_upper;
};

//find the No.n P in T
int pos(string T, string P, int n) {
	if (n == 0)return -1;
	int count = 0;
	unsigned begined = 0;
	while ((begined = T.find(P, begined)) != string::npos) {
		count++;
		begined += P.length();
		if (n == count) {
			return begined - 1;
		}
	}
	return -2;
}

//well done
vector<Grammar> load_grammar(string filename) {
	vector<Grammar> grammar_list;
	ifstream file(filename);
	string line;
	while (getline(file, line)) {
		Grammar grammar;
		grammar.left = line.substr(1, line.find(">") - 1);
		grammar.right_lower = line.substr(pos(line, ">", 2) + 2, line.size() - 3 - pos(line, ">", 2) - 2);
		// if there is no upper then right_upper would be a mess but it's ok cause there is a include_upper to point it out.
		grammar.right_upper = line.substr(line.size() - 2, 1);
		grammar.include_upper = (line.find("<", 1) != string::npos);
		grammar_list.push_back(grammar);
	}
	return grammar_list;
}

struct NFA_node {
	string value = "";
	vector<pair<string, NFA_node*>> edge_list = {};
	bool if_terminal = false;
	bool if_inception = false;
};

class NFA {
public:
	// type defines what type of word the NFA can identify.And type also is the starting node's value of each NFA.
	string type;
	set<string> letter_set;
	unordered_map<string, NFA_node*> status_set;
	//unordered_set<NFA_node*> NFA_node_set;// maybe useless

	NFA(string grammar_filename) {
		vector<Grammar> grammar_list = load_grammar(grammar_filename);
		transform_grammar_to_NFA(grammar_list);
	}

	// Function to transform the Grammar_list into the member NFA_node_set
	void transform_grammar_to_NFA(vector<Grammar> grammar_list) {
		//Create the terminal node
		NFA_node* node = new NFA_node();
		node->value = "terminal";
		node->if_terminal = true;
		//NFA_node_set.insert(node);
		status_set["terminal"] = node;

		for (auto& grammar : grammar_list) {
			letter_set.insert(grammar.right_lower);
			// set up the NFA's type like 'constant' or 'identifier'
			if (grammar.left.size() > 1) {
				type = grammar.left;
			}
			// If the left of the grammar is not already in the status_set, create a new NFA_node for it and add it to the NFA_node_set
			if (status_set.find(grammar.left) == status_set.end()) {
				NFA_node* node = new NFA_node();
				node->value = grammar.left;
				node->if_terminal = false;
				//NFA_node_set.insert(node);
				status_set[grammar.left] = node;
			}
			// If the right_upper of the grammar is not already in the status_set, create a new NFA_node for it and add it to the NFA_node_set
			if (status_set.find(grammar.right_upper) == status_set.end() && grammar.include_upper == true) {
				NFA_node* node_upper = new NFA_node;
				node_upper->value = grammar.right_upper;
				node_upper->if_terminal = false;
				//NFA_node_set.insert(node_upper
				status_set[grammar.right_upper] = node_upper;
			}
			// If the right_upper is Null, then the edge should go to the terminal.
			if (grammar.include_upper == true)
			{
				// Add the right_lower side of the grammar and the NFA_node whose value is the same as the right upper side of the grammar to the edge_list of the NFA_node whose value is the same as the left side of the grammar
				status_set[grammar.left]->edge_list.push_back(make_pair(grammar.right_lower, status_set[grammar.right_upper]));
			}
			else
			{
				status_set[grammar.left]->edge_list.push_back(make_pair(grammar.right_lower, status_set["terminal"]));
			}
		}
	}

};

struct DFA_node {
	string value = "";
	// first weight then second node's name.
	vector<pair<string, string>> edge_list = {};
	bool if_terminal = false;
};

class DFA {
public:
	string type;
	// node's name to a node
	map<string, DFA_node> node_set;


	DFA(NFA nfa) {
		type = nfa.type;

		// subset is given a node
		map<vector<string>, string> subset_cluster;

		map<vector<string>, bool> C;
		/*vector<vector<string>> all_subset;*/
		// name each subset's node
		char name = 'A';

		// add start to C and status_set
		vector<string> start = { type };
		/*all_subset.push_back(start);*/
		C[start] = false;

		DFA_node start_node = DFA_node();
		start_node.value = "inception";
		start_node.if_terminal = if_terminal(start);
		node_set["inception"] = start_node;
		subset_cluster[start] = "inception";

		// the unmarked set in C
		vector<string> subset = find_unmarked(C);
		do {
			// mark
			C[subset] = true;
			string node_name = subset_cluster[subset];

			/*if (node_set[node_name].if_terminal) {
				subset = find_unmarked(C);
				continue;
			}*/

			for (auto& letter : nfa.letter_set) {
				vector<string> new_subset = closure(move(subset, nfa, letter), nfa);

				// if the subset is not already in the status_set and it's not empty then add it.
				if (C.find(new_subset) == C.end() && !new_subset.empty()) {
					// add it to C
					C[new_subset] = false;
					//create a DFA_node of new_subset
					DFA_node new_node = DFA_node();
					string string_name(1, name);
					new_node.value = string_name;
					name += 1;
					new_node.if_terminal = if_terminal(new_subset);
					subset_cluster[new_subset] = string_name;
					node_set[string_name] = new_node;
				}
				//add edge
				if (!new_subset.empty()) {
					node_set[node_name].edge_list.push_back(make_pair(letter, subset_cluster[new_subset]));
				}

			}
			subset = find_unmarked(C);
		} while (!subset.empty());
	}

	bool if_terminal(vector<string> subset) {
		for (auto& s : subset) {
			if (s == "terminal")
				return true;
		}
		return false;
	}

	vector<string> find_unmarked(map<vector<string>, bool> C) {
		for (auto& a : C) {
			if (a.second == false) {
				return a.first;
			}
		}
		return vector<string>();
	}

	vector<string> closure(vector<string> input_nodes, NFA nfa) {
		vector<string> closure_list = input_nodes;
		for (auto& node_value : input_nodes) {
			NFA_node* node = nfa.status_set[node_value];
			for (auto& edge : node->edge_list) {
				if (edge.first == "$") {
					closure_list.push_back(edge.second->value);
				}
			}
		}
		return closure_list;
	}

	vector<string> move(vector<string> T, NFA nfa, string letter) {
		vector<string> move_list;
		for (auto& s : T) {
			NFA_node* node = nfa.status_set[s];
			for (auto& edge : node->edge_list) {
				if (edge.first == letter) {
					move_list.push_back(edge.second->value);
				}
			}
		}
		return move_list;
	}
};

// sort by priority from top to bottom 
string grammar_file_list[grammar_number] = {
	"grammar_delimiter.txt",
	"grammar_string.txt",
	"grammar_number.txt",
	"grammar_bi_op_compare.txt",
	"grammar_unary_op.txt",
	"grammar_bi_op.txt",
	"grammar_constant.txt",
	"grammar_keyword_if.txt",
	"grammar_keyword_for.txt",
	"grammar_keyword_while.txt",
	"grammar_keyword_else.txt",
	"grammar_keyword_statement.txt",
	"grammar_keyword_return.txt",
	"grammar_qualifier.txt",
	"grammar_type.txt",
	"grammar_identifier.txt",
	"grammar_error.txt"
};

// if a word's type is one of the following 4 types then it should be analyzed if it's an identifier in case of some identifier's value is ambiguous such as "noner" and "forwhile" 
string ambiguous_identifier[9] = {
	"constant",
	"keyword_if",
	"keyword_for",
	"keyword_while",
	"keyword_else",
	"keyword_statement",
	"keyword_return",
	"qualifier",
	"type",
};

string ambiguous_error = "number";

vector<DFA> get_dfa_list() {
	vector<DFA> dfa_list;

	for (int i = 0; i < grammar_number; i++) {
		string filename = grammar_file_list[i];
		NFA nfa = NFA(filename);
		DFA dfa = DFA(nfa);
		dfa_list.push_back(dfa);
	}
	return dfa_list;
}

struct Token {
	// false means the token is empty and invalid
	//bool valid = true;
	int line;
	string type;
	string value;
};

Token DFA_read(DFA dfa, string line, int& index) {
	Token t;
	t.value = "";
	string value = "";
	string status = "inception";
	bool success = false;
	bool match_not_null = false;
	bool match_null = false;
	DFA_node node = dfa.node_set[status];

	for (; index < line.size(); index++) {
		string input_letter = line.substr(index, 1);

		success = false;
		match_not_null = false;
		match_null = false;

		if (input_letter == " " && dfa.type != "string") {
			index++;
			break;
		}
		for (auto& edge : node.edge_list) {
			// make it easy to read
			if (true)
			{
				// number
				if (edge.first == "^d" && (char)input_letter[0] >= '0' && (char)input_letter[0] <= '9') {
					status = edge.second;
					match_not_null = true;
					break;
				}
				else
					// lowercase letter
					if (edge.first == "^c" && (char)input_letter[0] >= 'a' && (char)input_letter[0] <= 'z') {
						status = edge.second;
						match_not_null = true;
						break;
					}
					else
						// uppercase letter
						if (edge.first == "^C" && (char)input_letter[0] >= 'A' && (char)input_letter[0] <= 'Z') {
							status = edge.second;
							match_not_null = true;							
							break;
						}
						else
							// any letter
							if (edge.first == "^~" && (char)input_letter[0] != '"') {

								status = edge.second;
								match_not_null = true;
								break;
							}
							else
								// current letter
								if (edge.first == input_letter) {
									status = edge.second;
									match_not_null = true;
									break;
								}
								else
									// $
									if (edge.first == "$") {
										status = edge.second;
										match_null = true;
									}
			}
		}
		node = dfa.node_set[status];
		if (match_not_null) {
			// prevent add the inception of next word to the word being processed.
			value += input_letter;
		}
		// reach terminal
		if (node.if_terminal && match_not_null) {
			t.type = dfa.type;
			t.value = value;
			success = true;
		}
		// only match $ then stop
		if (match_null && !match_not_null) {
			break;
		}
		// match nothing or already success
		if (!match_not_null && (success || !match_null)) {
			break;
		}
	}
	return t;
}

list<Token> read_file(vector<DFA> dfa_list, string filename) {
	list<Token> token_list;
	ifstream file(filename);
	string line;
	int index = 0;
	int line_num = 1;
	while (getline(file, line)) {
		// pass empty line
		if (line.empty()) {
			line_num++;
			continue;
		}
		// recognize the annotation
		if (line.size() > 2 && line[0] == line[1] == '/') {
			Token t;
			t.line = line_num;
			t.type = "annotation";
			t.value = line.substr(2);
			token_list.push_back(t);
			continue;
		}
		line.erase(0, line.find_first_not_of(" "));

		while (index <= line.size() - 1) {
			int start_index = index;
			for (int i = 0; i < grammar_number; i++) {
				DFA dfa = dfa_list[i];
				index = start_index;
				Token t = DFA_read(dfa, line, index);
				//not this dfa
				if (t.value == "") {
					continue;
				}
				// check if token is an identifier but it was analyzed to be of another type wrongly like 'forwhile' incorrectly being took as 2 keyword 'for' and 'while' 
				for (int i = 0; i < ambiguous_identifier->size(); i++) {
					string a = ambiguous_identifier[i];
					if (t.type == a) {
						index = start_index;
						Token identifier = DFA_read(dfa_list[15], line, index);
						//it should be an identifier
						if (identifier.value.size() > t.value.size()) {
							t = identifier;
						}
					}
				}
				
				// check if a invalid word was analyzed to be a number and a identifier like '4abc'
				if (t.type == ambiguous_error) {
					int save_index = index;
					index = start_index;
					Token error = DFA_read(dfa_list[16], line, index);
					//it should be an error
					if (error.value.size() > t.value.size()) {
						t = error;
					}
					else
					{
						index = save_index;
					}
				}
				t.line = line_num;
				token_list.push_back(t);
				break;
			}
		}
		line_num++;
		index = 0;
	}
	return token_list;
}

int main() {
	vector<DFA> dfa_list = get_dfa_list();

	for (auto dfa : dfa_list) {
		cout << endl << dfa.type << endl;
		for (auto a : dfa.node_set) {
			if (a.second.if_terminal) {
				cout << "     " << a.first << " is terminal" << endl;
			}
		}
		cout << endl;
		for (auto a : dfa.node_set) {
			if (a.second.edge_list.empty()) {
				continue;
			}
			for (auto b : a.second.edge_list) {
				cout << "     "  <<a.second.value << " -- " << b.first << " --> " << b.second << endl;
			}
			cout << endl;
		}
	}

	list<Token> token_list = read_file(dfa_list, "code.txt");
	ofstream outfile;
	outfile.open("C:\\Users\\ASUS\\source\\repos\\±‡“Î∆˜\\”Ô∑®∑÷Œˆ∆˜\\Token_list.txt");
	int len = token_list.size();
	int i = 0;
	for (auto token : token_list) {
		i++;
		outfile << "[ line: " << token.line << ", type: " << token.type << ", value: " << token.value << "]";
		if (i != len) outfile << endl;
	}
	outfile.close();
}


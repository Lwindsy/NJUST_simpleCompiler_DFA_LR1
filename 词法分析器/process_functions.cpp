#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>

#include "NFA_DFA.h"
#include "process_functions.h"
#include "const.h"


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

vector<DFA> get_dfa_list() {
	vector<DFA> dfa_list;

	for (int i = 0; i < grammar_number; i++) {
		string filename = "grammar/" + grammar_file_list[i];
		NFA nfa = NFA(filename);
		DFA dfa = DFA(nfa);
		dfa_list.push_back(dfa);
	}
	return dfa_list;
}

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
		line.erase(0, line.find_first_not_of(" "));
		cout << line_num << endl;

		cout << line << endl;
		// pass empty line
		if (line.empty()) {
			line_num++;
			continue;
		}
		// recognize the annotation
		if (line.size() >= 2 && line[0] == line[1] && line[0] == '/') {
			Token t;
			t.line = line_num;
			t.type = "annotation";
			t.value = line.substr(2);
			token_list.push_back(t);

			line_num++;
			continue;
		}

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
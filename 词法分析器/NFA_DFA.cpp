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


NFA::NFA(string grammar_filename) {
	vector<Grammar> grammar_list = load_grammar(grammar_filename);
	transform_grammar_to_NFA(grammar_list);
}

// Function to transform the Grammar_list into the member NFA_node_set
void NFA::transform_grammar_to_NFA(vector<Grammar> grammar_list) {
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

DFA::DFA(NFA nfa) {
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

bool DFA::if_terminal(vector<string> subset) {
	for (auto& s : subset) {
		if (s == "terminal")
			return true;
	}
	return false;
}

vector<string> DFA::find_unmarked(map<vector<string>, bool> C) {
	for (auto& a : C) {
		if (a.second == false) {
			return a.first;
		}
	}
	return vector<string>();
}

vector<string> DFA::closure(vector<string> input_nodes, NFA nfa) {
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

vector<string> DFA::move(vector<string> T, NFA nfa, string letter) {
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
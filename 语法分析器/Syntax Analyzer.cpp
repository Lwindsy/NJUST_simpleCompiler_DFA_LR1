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
#include "LR_1.h"

using namespace std;

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
	bool cout_ic_node = true;
	bool cout_ACTION = true;
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
				reduce.print();
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
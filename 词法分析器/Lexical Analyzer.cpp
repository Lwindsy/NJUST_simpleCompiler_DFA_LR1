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
#include "structs.h"

using namespace std;

int main() {
	vector<DFA> dfa_list = get_dfa_list();

	list<Token> token_list = read_code(dfa_list, "code.txt");

	ofstream outfile;
	outfile.open("C:\\Users\\ASUS\\source\\repos\\±àÒëÆ÷\\Óï·¨·ÖÎöÆ÷\\Token_list.txt");
	int len = token_list.size();
	int i = 0;
	for (auto token : token_list) {
		i++;
		outfile << "[ line: " << token.line << ", type: " << token.type << ", value: " << token.value << "]";
		if (i != len) outfile << endl;
	}
	outfile.close();
}


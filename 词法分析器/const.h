#pragma once

#include <string>

using namespace std;

constexpr auto grammar_number = 17;

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
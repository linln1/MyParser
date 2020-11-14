#pragma once
#ifndef LL1_H
#define LL1_H

#include "lexer.h"

using namespace std;

extern string start_symbol;
extern vector<string> tokens;

//extern uint unordered_map(const string &v);
extern unordered_set<string> non_terminals;
extern unordered_set<string> terminals;
extern unordered_map<string, vector<string>> productions;
extern unordered_set<string> V_T;
extern unordered_set<string> V_N;
extern unordered_map<string, set<string>> first;
extern unordered_map<string, set<string>> follow;
extern unordered_map<string, set<string>> first_of_prod;
extern unordered_map<string, bool> first_constructed;
extern unordered_map<string, bool> follow_constructed;
extern unordered_map<string, unordered_map<string, bool>> follow_contains;
extern unordered_map<string, unordered_map<string, string>> LL1_table;

extern void generate_LL1_table();
extern void calculate_all_first();
extern void calculate_all_follow();
extern void extract_left_common_factor();
extern void calculate_first();
extern void calculate_follow();
extern void init();
extern void parser();
extern void print_symbol(const string& s);
extern std::vector<std::string> split(const char *s, const char *delim);

#endif // LL1_H

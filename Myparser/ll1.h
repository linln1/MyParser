#pragma once
#ifndef LL1_H
#define LL1_H

#include "lexer.h"

using namespace std;

extern void eliminate_left_recurrent(QJsonObject grammar);
extern void generate_LL1_table();
extern void calculate_all_first();
extern void calculate_all_follow();
extern void extract_left_common_factor();
extern void calculate_first();
extern void calculate_follow();
extern void init();
extern void ll1_parser();
extern void print_symbol(const string& s);
extern std::vector<std::string> split(const char *s, const char *delim);

#endif // LL1_H

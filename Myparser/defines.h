#pragma once
#ifndef DEFINES_H
#define DEFINES_H

#include <stdio.h>
#include <queue>
#include <stack>
#include <set>
#include <string>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <Windows.h>
#include <malloc.h>
#include <stdint.h>
#include <handleapi.h>
#include <map>
#include <utility>
#include <string.h>
#include <vector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#define MAXKEY 1024
#define INITSIZE 8
#define BUFFER_MAX 256

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

typedef struct TkWord {
    int code;
    struct TkWord* next;
    char* str;
    struct Symbol* symStruct;
    struct Symbol* symIdentifier;
}tkWord;

typedef struct DynString {
    int len;        //length of the string
    int capacity;   //buffer length
    char* data;     //the pointer to this string
}dynString;

typedef struct DynArray {
    int len;    //len of array;
    int capacity;   //capacity of the buffer
    void** data;    //pointer to array
}dynArray;

typedef struct BUFFER {
    char data[BUFFER_MAX];
    int len;
    char* startPtr = (char*)malloc(sizeof(char));
    char* cur = (char*)malloc(sizeof(char));//forwardPtr;
}BUFFER;

typedef struct ErrorInfo {
    int line;
    int col;
    string errorInfo;
}errorInfo;

typedef struct StaticOnLine {
    int charaCount = 0;
    int definatorCount = 0;
    int identificatorCount = 0;
    int boundaryCount = 0;
    int operatorCount = 0;
    int constCount = 0;
    vector<errorInfo> staticInfo;
}staticOnLine;

// some error handler structure
enum errorLevel {
    CPP_WARNING,
    CPP_ERROR
};

enum workStage {
    COMPLIE,
    LINK,
};

enum LEX_STATE {
    LEX_NORMAL,
    LEX_SEP,
};


#endif // DEFINES_H

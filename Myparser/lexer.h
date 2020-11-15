#pragma once
#ifndef LEXER_H
#define LEXER_H

//标识符， 定义符， 界符， 常数， 运算符

#include "defines.h"

//some structure
enum eTokenCode {

    //定义符
    CPP_NAME,           //definator
    KW_CHAR,            //char keyword
    KW_DOUBLE,          //double keyword
    KW_FLOAT,           //float keyword
    KW_INT,             //int keyword
    KW_LONG,            //long keyword
    KW_BOOL,            //bool keyword
    KW_REGISTER,        //register keyword
    KW_SHORT,           //short keyword
    KW_SIGNED,          //signed keyword
    KW_STATIC,          //static keyword
    KW_STRUCT,          //struct keyword
    KW_UNSIGNED,        //unsigned keyword
    KW_VOID,            //void keyword
    KW_CHAR_STAR,       //char* keyword
    KW_DOUBLE_STAR,         //double* keyword
    KW_FLOAT_STAR,          //float* keyword
    KW_INT_STAR,                //int* keyword
    KW_LONG_STAR,           //long* keyword
    KW_BOOL_STAR,           //bool* keyword
    KW_SHORT_STAR,          //short* keyword
    KW_VOID_STAR,       //void* keyword
    KW_VOLATILE,        //volatile keyword

    KW_IF,              //if keyword
    KW_ELSE,            //else keyword
    KW_FOR,             //for keyword
    KW_CONTINUE,        //continue keyword
    KW_BREAK,           //break keyword
    KW_RETURN,          //return keyword
    KW_SIZEOF,          //sizeof keyword
    KW_WHILE,           //while keyword
    KW_AUTO,            //auto keyword
    KW_CASE,            //case keyword
    KW_CONST,           //const keyword
    KW_DEFAULT,         //default keyword
    KW_DO,              //do keyword
    KW_ENUM,            //enum keyword
    KW_EXTERN,          //extern keyword
    KW_GOTO,            //goto keyword
    KW_SWITCH,          //switch keyword
    KW_TYPEDEF,         //typedef keyword,
    KW_UNION,           //union keyword
    KW_NULL,            //null keyword
    kW_B_TRUE,          //True keyword
    KW_B_FALSE,         //False keyword
    KW_L_TRUE,          //true keyword
    KW_L_FALSE,         //false keyword

    PREV_WHITE,         //white space
    CPP_ELLIPSIS,       //...
    CPP_AT,             //@
    CPP_DOLLAR,         //$

    CPP_EOF,            //end of file

    //运算符
    CPP_OPERA,          //operator
    CPP_NOT,    //!
    CPP_PLUS,   //+
    CPP_MINUS,  //-
    CPP_MULT,   //*
    CPP_DIV,    // /
    CPP_MOD,    //%
    CPP_PLUS_EQ,    //+=
    CPP_MINUS_EQ,   //-=
    CPP_DIV_EQ, ///=
    CPP_MULT_EQ,    //*=
    CPP_PLUSPLUS,   //++
    CPP_MINUSMINUS, //--
    CPP_MOD_EQ, //%=
    CPP_EQ,     //==
    CPP_NOT_EQ, //!=
    CPP_LESS,   //<
    CPP_GREATER,//>
    CPP_LESS_EQ,    //<=
    CPP_GREATER_EQ, //>=
    CPP_LSHIFT,     //<<
    CPP_RSHIFT,     //>>
    CPP_LSHIFT_EQ,  //<<=
    CPP_RSHIFT_EQ,  //>>=
    CPP_ASSIGN, //=
    CPP_PTR,    //->
    CPP_PTR_STAR,   //->*
    CPP_DOT,    //.
    CPP_LOGIC_AND,  //&&
    CPP_ARITH_AND,  //&
    CPP_AND_EQ,     //&=
    CPP_LOGIC_OR,   //||
    CPP_ARITH_OR,   //|
    CPP_OR_EQ,      //|=
    CPP_XOR,        //^
    CPP_XOR_EQ,     //^=

    //界符
    CPP_BOUNDARY,       //boundary
    CPP_OPEN_PAREN, //(
    CPP_CLOSE_PAREN,    //)
    CPP_OPEN_SQUARE,    //[
    CPP_CLOSE_SQUARE,   //]
    CPP_OPEN_BRACE,     //{
    CPP_CLOSE_BRACE,    //}
    CPP_QUERY,          //
    CPP_COMPL,          //~
    CPP_COMMA,          //,
    CPP_SEMICOLON,      //;
    CPP_HASH,           //#
    CPP_PASTE,          //##
    CPP_COLON,          //:
    CPP_COLON_COLON,    //::

    //常量: 数值常量和字符常量
    CPP_NUMBER,         //number
    CPP_CINT,           //const int
    CPP_CFLOAT,         //const float
    CPP_CCHAR,          //const char
    CPP_CSTR,           //const char*

    NOT_FLOAT,          //not a float number
    AFTER_POINT,        //already find the mark of the float number
    AFTER_EXPON,        //scientific count mark

    CPP_N_DECIMAL,      //decimal number
    CPP_N_HEX,          //hex number
    CPP_N_BINARY,       //binary number
    CPP_N_OCTAL,        //octal number

    //标识符
    CPP_IDENT           //identifier
};             //WordTable

extern dynArray tkTable;

extern int wordCount; // 源代码单词数
extern int charCount; // 源代码字符数
extern int lineCount; // 源代码行数
extern int curLineSize; //当前行字符个数
extern int identifierCount; //源代码标识符个数
extern int definatioinCount; //源代码定义符个数
extern int boundaryCount; //源代码界符个数
extern int constCount; //源代码常数个数
extern int operatorCount; //源代码运算符个数
extern int tokenCount;//源代码token数
extern staticOnLine curLine;

extern int token;
extern bool over; //标志文件是否读完了

//
extern int symMatching;
extern int parenMatching;
extern int squareMatching;
extern int braceMatching;
extern int angleMatching;

extern char* filename;
extern char* temp;
extern char ch;
extern string debuglevel;

extern BUFFER* buffer;
extern vector<int> tokens_type;
extern FILE* fin;
extern std::map<int, std::pair<char*, string>> tokenStream;
extern std::map<string, int> CountToken;

//elfHash(str->key) *tkHashTable[key] 是hash值一样的所有tkWord的链表
//tkWord* tkHashTable[]  {code, *next, *str, symStruct, symIdentifier}
//dynArray tkTable 本身是一个 ‘每一项存的是tkWord的指针’ 的数组，len是tkTable的长度， capacity是tkTable的最大容量

extern tkWord* tkHashTable[MAXKEY];        //HashTable

extern dynString tkstr;
extern dynString sourcestr;

extern int errorCount;
extern vector<staticOnLine> lexError;

////some error handler function
extern void exceptionHandle(int stage, int level, const char* fmt, va_list ap);
extern void error(const char* fmt, ...);
extern void noMatchingHandler();
extern char getChar();
//dynString func
extern char* getTkstr(int v);
extern void myDynStringInit(dynString* strP, int initSize);
extern void myDynStringFree(dynString* strP);
extern void myDynStringReset(dynString* strP);
extern void myDynStringRealloc(dynString* strP, int newSize);
extern void myDynStringChcat(dynString* strP, int ch);
//dynArray func
extern void myDynArrayInit(dynArray* arrP, int initSize);
extern void myDynArrayRealloc(dynArray* arrP, int newSize);
extern void myDynArrayAdd(dynArray* arrP, void* data);
extern void myDynArrayFree(dynArray* arrP);
extern int myDynArraySearch(dynArray* arrP, int key);
//colorazation

extern void colorToken(int state);
//preprocess func
extern void skipWhiteSpace();
extern void parseComment();
extern void preprocess();
//malloc func
extern void* mallocz(int size);
//hash func and insert func

extern int elfHash(char* key);
extern tkWord* tkInsertKeyWord(tkWord* tp);
extern tkWord* tkFindWord(char* p, int key);
extern tkWord* tkInsertIdentifier(char* p) ;
//parse func
extern void linker(tkWord* ref) ;//将来调用链接器，连接#include 的头文件

extern int isDigit(char c);
extern int isXDigit(char c);
extern int notDigit(char c) ;
extern int ifNexIs(char c, eTokenCode A, eTokenCode B);
extern void parseIdentifier(tkWord* result);
extern void parseString(tkWord* result);
//增加了解析数字的处理
extern void parseNumber(tkWord* result);
//main lexer func

extern void lexerDirect();

extern void lexerInit();
extern void lexerCleanup() ;
extern void warning(char* fmt, ...) ;
extern void except(char* msg) ;
extern void skip(int c);
extern void linkError(char* fmt, ...) ;
extern void infoDisplay();
extern int lexerinit(int, char**);
extern void endoflexer();
extern int lexer();

#endif // LEXER_H

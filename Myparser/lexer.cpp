#define _CRT_SECURE_NO_WARNINGS

#include "lexer.h"
#include <iostream>

dynArray tkTable;

int wordCount = 0; // 源代码单词数
int charCount = 0; // 源代码字符数
int lineCount = 0; // 源代码行数
int curLineSize = 0; //当前行字符个数
int identifierCount = 0; //源代码标识符个数
int definatioinCount = 0; //源代码定义符个数
int boundaryCount = 0; //源代码界符个数
int constCount = 0; //源代码常数个数
int operatorCount = 0; //源代码运算符个数
int tokenCount = 0;//源代码token数
staticOnLine curLine;

int token;
bool over = false; //标志文件是否读完了

//
int symMatching = 0;
int parenMatching = 0;
int squareMatching = 0;
int braceMatching = 0;
int angleMatching = 0;

char* filename;
char* temp = (char*)malloc(sizeof(char) * 256);
char ch;
string debuglevel;
vector<int> tokens_type;
BUFFER* buffer = (BUFFER*)malloc(sizeof(BUFFER));

FILE* fin;
std::map<int, std::pair<char*, string>> tokenStream;
std::map<string, int> CountToken;

//elfHash(str->key) *tkHashTable[key] 是hash值一样的所有tkWord的链表
//tkWord* tkHashTable[]  {code, *next, *str, symStruct, symIdentifier}
//dynArray tkTable 本身是一个 ‘每一项存的是tkWord的指针’ 的数组，len是tkTable的长度， capacity是tkTable的最大容量

tkWord* tkHashTable[MAXKEY];        //HashTable

dynString tkstr;
dynString sourcestr;

int errorCount = 0;
vector<staticOnLine> lexError;

//some error handler function
void exceptionHandle(int stage, int level, const char* fmt, va_list ap) {
    char buf[1024];
    vsprintf(buf, fmt, ap);
    if (stage == COMPLIE) {
        if (level == CPP_WARNING)
            printf("%s(%d th row): Compile Warning : %s ! \n", filename, lineCount, buf);
        else
        {
            printf("%s(%d th row): Compile Error : %s ! \n", filename, lineCount, buf);
            exit(-1);
        }
    }
    else {
        printf("LINK Error : %s ! \n", buf);
        exit(-1);
    }
}

void error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    exceptionHandle(COMPLIE, CPP_ERROR, fmt, ap);
    va_end(ap);
}

void noMatchingHandler() {
    if (parenMatching > 0) {
        //need more '(' or less ')' in line %dth, Col %dth, lineCount, charaCount
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "need more ')' or less '(' in line";
        curLine.staticInfo.push_back(curError);
    }
    else if (parenMatching < 0) {
        //need more ')' or less '(' in line %dth, Col %dth, lineCount, charaCount
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "need more '(' or less ')' in line";
        curLine.staticInfo.push_back(curError);
    }

    if (squareMatching > 0) {
        //need more '[' or less ']' in line %dth, Col %dth, lineCount, charaCount
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "need more ']' or less '[' in line";
        curLine.staticInfo.push_back(curError);
    }
    else if (squareMatching < 0) {
        //need more ']' or less '[' in line %dth, Col %dth, lineCount, charaCount
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "need more '[' or less ']' in line";
        curLine.staticInfo.push_back(curError);
    }

    /*  //关键是{和}一般都不在一行,所以还需要单独处理
        if (braceMatching > 0) {
            //need more '{' or less '}' in line %dth, Col %dth, lineCount, charaCount
        }
        else if (braceMatching < 0) {
            //need more '}' or less '{' in line %dth, Col %dth, lineCount, charaCount
        }
    */

    if (angleMatching > 0) {
        //nedd more '<' or less '>' in line %dth, Col %dth, lineCount, charaCount
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "nedd more '>' or less '<' in line";
        curLine.staticInfo.push_back(curError);
    }
    else if (angleMatching < 0) {
        //need more '>' or less '<' in line %dth, Col %dth, lineCount, charaCount
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "need more '<' or less '>' in line";
        curLine.staticInfo.push_back(curError);
    }
}

char getChar() {
    ch = *(buffer->cur);
    //cur指针指向缓冲区末尾 或者 cur指针指向的内容是'\n'，buffer要读下一行或者读当前行下一部分
    if (ch != '\0') {
        charCount++;
        curLine.charaCount++;
    }
    //buffer要读下一行(部分)
    if (ch == '\n') {
        //printf("\n");
        noMatchingHandler();
        staticOnLine temp;
        temp.boundaryCount = curLine.boundaryCount;
        temp.charaCount = curLine.charaCount;
        temp.definatorCount = curLine.definatorCount;
        temp.identificatorCount = curLine.identificatorCount;
        temp.operatorCount = curLine.operatorCount;
        temp.staticInfo = curLine.staticInfo;

        lexError.push_back(temp);
        if (!fgets(buffer->data, BUFFER_MAX, fin)) {
            over = true;
            buffer->data[0] = '\0';
        }//读取文件下一行,读取BUFFER_MAX个字符或者读取到'\n'时停止
        buffer->startPtr = &buffer->data[0];//当前单词已经结束，新的单词开始
        buffer->cur = &buffer->data[0];

        curLine.boundaryCount = 0;
        curLine.charaCount = 0;
        curLine.definatorCount = 0;
        curLine.identificatorCount;
        curLine.operatorCount = 0;
        curLine.staticInfo = vector<errorInfo>{};
    }
    else if (buffer->cur == &buffer->data[BUFFER_MAX]) {//当前指针已经读完该缓冲区里面的全部内容，但是仍未遇到换行符，所以应该接着读这一行
        fgets(buffer->data, BUFFER_MAX, fin);
        buffer->cur = &buffer->data[0];//上一个单词还没结束，但是缓冲区已经用完了
    }
    else if (ch == ' ') {
        buffer->cur++;
        buffer->startPtr = buffer->cur;
    }
    else {
        buffer->cur++;//指针向前移动一格
    }

    return ch;
}

//dynString func
char* getTkstr(int v) {
    if (v > tkTable.len)
        return NULL;
//    if (v >= CPP_OPERA && v < CPP_NUMBER){
//        return (char*)tkstr.data;
//    }
    if (v >= CPP_NUMBER && v< CPP_IDENT) {
        return "num";
        //return (char*)tkstr.data;
    }
    return (char*)((tkWord*)tkTable.data[v])->str;
}

void myDynStringInit(dynString* strP, int initSize) {

    if (strP != NULL) {
        strP->data = (char*)malloc(sizeof(char) * initSize);
        strP->len = 0;
        strP->capacity = initSize;
    }
}

void myDynStringFree(dynString* strP) {
    if (strP != NULL) {
        if (strP->data) {
            free(strP->data);
        }
        strP->len = 0;
        strP->capacity = 0;
    }
}

void myDynStringReset(dynString* strP) {
    myDynStringFree(strP);
    myDynStringInit(strP, INITSIZE);
}

void myDynStringRealloc(dynString* strP, int newSize) {
    int Cap = 1;
    char* data;
    Cap = strP->capacity;
    while (Cap < newSize) {
        Cap = Cap << 1;
    }
    data = (char*)realloc(strP->data, Cap);
    if (!data) {
        error("Error to allocate the memory!\n");
    }
    strP->capacity = Cap;
    strP->data = data;
}

void myDynStringChcat(dynString* strP, int ch) {
    int len = 0;
    len = strP->len + 1;
    if (len > strP->capacity) {
        myDynStringRealloc(strP, len);
    }
    ((char*)strP->data)[len - 1] = ch;
    strP->len = len;
}

//dynArray func
void myDynArrayInit(dynArray* arrP, int initSize) {
    if (arrP != NULL) {
        arrP->data = (void**)malloc(sizeof(char) * initSize);
        arrP->len = 0;
        arrP->capacity = initSize;
    }
}

void myDynArrayRealloc(dynArray* arrP, int newSize) {
    int Cap = 0;
    void** data;

    Cap = arrP->capacity;
    while (Cap < newSize) {
        Cap = Cap << 1;
    }
    //有问题
    data = (void**)realloc(arrP->data, Cap);
    if (!data)
        error("failuer to allocate the memory!\n");
    arrP->capacity = Cap;
    arrP->data = data;

}

void myDynArrayAdd(dynArray* arrP, void* data) {
    int len = 0;
    len = arrP->len + 1;
    if (len * sizeof(void*) > (unsigned int)arrP->capacity) {
        myDynArrayRealloc(arrP, len * sizeof(void*));
    }
    arrP->data[len - 1] = data;
    arrP->len = len;
}

void myDynArrayFree(dynArray* arrP) {
    void** p;
    for (p = arrP->data; arrP->len; ++p, --arrP->len) {
        if (*p)
            free(*p);
    }
    free(arrP->data);
    arrP->data = NULL;
}

int myDynArraySearch(dynArray* arrP, int key) {
    int** p;
    p = (int**)arrP->data;
    for (int i = 0; i < arrP->len; ++i, p++) {
        if (key == **p)
            return i;
    }
    return  -1;
}

//colorazation

void colorToken(int state) {

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    char* p;
    switch (state) {
    case LEX_NORMAL:
    {
        if (token >= CPP_IDENT) //标识符
            SetConsoleTextAttribute(h, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        else if (token >= CPP_NUMBER) //常量
            SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        else if (token >= CPP_BOUNDARY) //界符
            SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
        else if (token >= CPP_OPERA)//运算符
            SetConsoleTextAttribute(h, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
        else //定义符
            SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        p = (char*)malloc(sizeof(char) * 256);
        p = getTkstr(token);
        string a = p;
        CountToken[a]++;
        if (token < CPP_OPERA) {
            printf("<定义符, %s>", p);
            tokenStream[tokenCount++] = std::make_pair((char*)"定义符", a);
            definatioinCount++;
        }
        else if (token < CPP_BOUNDARY) {
            printf("<运算符, %s>", p);
            tokenStream.insert(make_pair(tokenCount++, make_pair((char*)("运算符"), a)));
        }
        else if (token < CPP_NUMBER) {
            printf("<界符, %s>", p);
            tokenStream.insert(make_pair(tokenCount++, make_pair((char*)("界符"), a)));
        }
        else if (token < CPP_IDENT) {
            printf("<常量, %s>", p);
            tokenStream.insert(make_pair(tokenCount++, make_pair((char*)("常量"), a)));
        }
        else {
            printf("<标识符, %s>", p);
            tokenStream.insert(make_pair(tokenCount++, make_pair((char*)("标识符"), a)));
        }
        break;
    }
    case LEX_SEP:
        printf("%c", ch);
        break;
    }
}

//preprocess func
void skipWhiteSpace() {
    while (ch == ' ' || ch == '\t' || ch == '\r') {
        if (ch == '\r') {
            ch = getChar();
            if (ch != '\n')
                return;
            lineCount++;
        }
        printf("%c", ch);
        ch = getChar();//这个getChar要改成从buffer里面读取字符，如果ch读到buffer的结束的地方，那就要让buffer自动读取文件的下一部分或者下一行

    }
}

void parseComment() {
    ch = getChar();
    int end = 0;
    do {
        while (ch != '*') {
            if (ch == '\n') {
                printf("\n");
                lineCount++;
            }
            if (ch == '\0') {
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "Block Comment not completed";
                curLine.staticInfo.push_back(curError);
                return;
            }
            ch = getChar();
        }

        while (ch == '*') {
            ch = getChar();
            if (ch == '/') {
                end = 1;
                return ;
            }
            else if(ch!='*'){
                ch = getChar();
            }
        }
    } while (end == 0);
}

void preprocess() {
    ch = getChar();
    while (1) {
        if (ch == ' ' || ch == '\t' || ch == '\r')
            skipWhiteSpace();
        else if (ch == '/') {
            ch = getChar();
            if (ch == '*') {
                parseComment();
                ch = getChar();
                break;
            }
            else if (ch == '/') {
                char* find;
                find = (char*)malloc(sizeof(char) * BUFFER_MAX);
                find = strchr(buffer->data, '\\');
                while (find) {
                    if ((*(find + 1) == '\r') && (*(find + 2) == '\n')) {
                        do {
                            ch = getChar();
                        } while (ch != '\n');
                        find = strchr(buffer->data, '\\');
                    }
                    else {
                        find = strchr(find + 1, '\\');
                    }
                    //*(find + 1) == '\n'   nextLine() : [&]()->void{find = strchr(find, '\\');}
                }
                if (ch == '\n') {
                    ch = getChar();
                }
                while ((!find) && ch != '\n' && ch != '\0') {
                    ch = getChar();
                }
                if (ch == '\n') {
                    token = -1;
                }
                if (ch == '\0') {
                    buffer->cur--;
                    charCount--;
                    curLine.charaCount--;
                    token = -1;
                }
            }
            else {
                ungetc(ch, fin);
                ch = '/';
                break;
            }
        }
        else
            break;
    }
    //ch = getChar();
}

//malloc func
void* mallocz(int size) {
    void* ptr;
    ptr = malloc(size);
    if (!ptr && size) {
        error("failure to allocate memory!\n");
    }
    memset(ptr, 0, size);
    return ptr;
}

//hash func and insert func

int elfHash(char* key) {

    int h = 0, g;
    while (*key) {
        h = (h << 4) + *key++;
        g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h % MAXKEY;

}

tkWord* tkInsertKeyWord(tkWord* tp) {
    int key;
    myDynArrayAdd(&tkTable, tp);
    key = elfHash((char*)tp->str);
    tp->next = tkHashTable[key];//第key项是一个 tkWord列表
    tkHashTable[key] = tp; //然后把这一项加到链表最前头，最后放到tkHashTable的第key项
    return tp;
}

tkWord* tkFindWord(char* p, int key) {//hash键值是key，字符串是p，看看是否已经存在于哈希表中了
    tkWord* temp;
    tkWord* res = NULL;
    temp = tkHashTable[key];//hashtable ( str -> key)   tkTable(key -> tkWord)
    while (temp != NULL) {
        if (!strcmp(p, (char*)temp->str)) {
            token = temp->code;
            res = temp;
            break;
        }
        temp = temp->next;
    }
    return res;
}

tkWord* tkInsertIdentifier(char* p) {
    tkWord* temp;
    int key = 0;
    char* s;
    char* end;
    int len = 0;

    key = elfHash(p);
    temp = tkFindWord(p, key);
    if (!temp) {//找不到对应的单词，说明她还没有被加入到tkHashTable中去，然后接下来就可往里插入tkWord了
        len = strlen(p);
        temp = (tkWord*)malloc(sizeof(tkWord) + len + 1);
        temp->next = tkHashTable[key];//tkHashTable中一个项中，同一个列表里的词code可能不同！
        tkHashTable[key] = temp;
        myDynArrayAdd(&tkTable, temp);
        temp->code = tkTable.len - 1;//tkTable 是用来存 code<->str的,符号表,同一个符号只能出现一次,标识符除外
        token = tkTable.len - 1;
        s = (char*)temp + sizeof(tkWord);
        temp->str = (char*)s;
        for (end = p + len; p < end; ) {
            *s++ = *p++;
        }
        *s = (char)'\0';
    }
    return temp;
}

//parse func
void linker(tkWord* ref) {

}//将来调用链接器，连接#include 的头文件

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

int isXDigit(char c) {
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
}

int notDigit(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c == '*');
}

int ifNexIs(char c, eTokenCode A, eTokenCode B) {
    if (*buffer->cur == c) {
        ch = getChar();
        return A;
    }
    return B;
}

void parseIdentifier(tkWord* result) {//
    myDynStringReset(&tkstr);
    myDynStringChcat(&tkstr, ch);
    ch = getChar();
    while (notDigit(ch) || isDigit(ch)) {
        myDynStringChcat(&tkstr, ch);
        ch = getChar();
    }
    if (ch == '-') {
        if (*buffer->cur == '>') {
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
            do {
                myDynStringChcat(&tkstr, ch);
                getChar();
            } while (notDigit(ch));
        }
    }
    if (ch == '.' || ch == '+' || ch == '-' || ch == ':') {
        myDynStringChcat(&tkstr, '\0');
        result->str = (char*)malloc(sizeof(char) * (tkstr.len + 1));
        memcpy(result->str, tkstr.data, tkstr.len + 1);
        tkInsertIdentifier((char*)result->str);
        result->code = token;
        return;
    }
    if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' && ch != ',' && ch != ';' && ch != '{' && ch != '[' && ch != ']' && ch != '(' && ch != ')' && ch != '>' && ch != '=' && ch != '<') {
        //标识符错误
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "invalid identifier defination";
        curLine.staticInfo.push_back(curError);
    }

    myDynStringChcat(&tkstr, '\0');
    result->str = (char*)malloc(sizeof(char) * (tkstr.len + 1));
    memcpy(result->str, tkstr.data, tkstr.len + 1);
    tkInsertIdentifier((char*)result->str);
    result->code = token;
}

void parseString(tkWord* result) {//常量不用加入符号表
    char c;
    token = CPP_CCHAR;
    myDynStringReset(&tkstr);
    myDynStringChcat(&tkstr, ch);
    char begin = ch;
    ch = getChar();
    int errorFlag = 0;
    for (;;) {
        if (ch != '\\') {
            if (ch == begin || (begin == '<' && ch == '>')) {
                if (ch == '>')
                    angleMatching--;
                myDynStringChcat(&tkstr, ch);
                break;
            }
            else if (ch == '\r') {
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "can't find \" or \' to end a const string";
                curLine.staticInfo.push_back(curError);
                errorFlag = 1;
                break;
            }
            //          else if (ch == '\n') {
                //          errorFlag = 1;
                    //      break;
                    //  }
            else {
                myDynStringChcat(&tkstr, ch);
                ch = getChar();
            }
        }
        else {
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
        }

        /*
                else if (ch == ' ') {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();
                }
                else if (ch == '\\') {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();
                    myDynStringChcat(&tkstr, ch);

                    switch (ch) {
                    case '0':
                        c = '\0';
                        break;
                    case 'a':
                        c = '\a';
                        break;
                    case 'b':
                        c = '\b';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case 'v':
                        c = '\v';
                        break;
                    case 'f':
                        c = '\f';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case '\"':
                        c = '\"';
                        break;
                    case '\'':
                        c = '\'';
                        break;
                    case '\\':
                        c = '\\';
                        break;
                    case 'n':
                        c = '\n';
                        break;
                    default:
                        break;

                    }

                    ch = getChar();
                }
                else {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();
                }
        */
    }
    if (errorFlag) {
        myDynStringChcat(&tkstr, begin);
    }
    myDynStringChcat(&tkstr, '\0');
    result->str = (char*)malloc(sizeof(char) * (tkstr.len + 1));
    memcpy(result->str, tkstr.data, tkstr.len + 1);
    token = result->code = CPP_CCHAR;
}

//增加了解析数字的处理
void parseNumber(tkWord* result) {
    /*parse number maybe integet maybe float or in scientific notation
    * maybe in decimal , hex, oct, bin
    */
    int valid = 1;
    int radix = 10; //默认是10进制
    myDynStringReset(&tkstr);//tokenstr 里面记录这个数字对应的字符串
    if (ch != '0') {
        do {
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
        } while (isDigit(ch));
    }
    else if (ch == '0') {//0开头
        radix = 8;
        myDynStringChcat(&tkstr, ch);
        ch = getChar();
        if (ch == '.') {
            radix = 10;//应该是10进制
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
        }
        else if (ch == 'x' || ch == 'X') {
            radix = 16;//是16
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
        }
        else if (isDigit(ch)) {
            do {
                myDynStringChcat(&tkstr, ch);
                ch = getChar();
            } while (isDigit(ch));
        }

        if (radix == 16) {
            if (!isXDigit(ch)) {
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "invalid number defination, digits should behind the . or X";
                curLine.staticInfo.push_back(curError);
            }
            else if (isXDigit(ch)) {
                do {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();
                } while (isXDigit(ch));
            }
        }
        else if (radix == 10) {
            if (isDigit(ch)) {
                do {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();
                } while (isXDigit(ch));
            }
        }
    }

    if (ch == '.') {
        myDynStringChcat(&tkstr, ch);
        ch = getChar();
        if (!isDigit(ch)) {
            errorInfo curError;
            curError.line = lineCount;
            curError.col = curLine.charaCount;
            curError.errorInfo = "invalid Float number defination ,digits should behind the .";
            curLine.staticInfo.push_back(curError);
        }
        else {
            do {
                myDynStringChcat(&tkstr, ch);
                ch = getChar();
            } while (isDigit(ch));
        }

        if (ch == 'E' || ch == 'e') {//是否是用科学计数法
            myDynStringChcat(&tkstr, ch);
            ch = getChar();
            if (ch == '+' || ch == '-') {
                char t = *buffer->cur;
                if (isDigit(t)) {
                    myDynStringChcat(&tkstr, ch);
                    myDynStringChcat(&tkstr, t);
                    buffer->cur++;
                    ch = getChar();
                    while (isDigit(ch)) {
                        myDynStringChcat(&tkstr, ch);
                        ch = getChar();//这里可能多读了一个字符
                    }
                }
                else {
                    //非法指数 +/- 后面应该是数字
                    errorInfo curError;
                    curError.line = lineCount;
                    curError.col = curLine.charaCount;
                    curError.errorInfo = "invalid Exponent defination, digit should follow behind the (+|-)";
                    curLine.staticInfo.push_back(curError);
                    valid = 0;
                }
            }
            else if (isDigit(ch)) {
                do {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();
                } while (isDigit(ch));
                if (notDigit(ch) && ch != '*') {
                    //非法标识符，数字后面不应该跟字母或下划线
                    errorInfo curError;
                    curError.line = lineCount;
                    curError.col = curLine.charaCount;
                    curError.errorInfo = "invalid identifier defination, should not start with digit";
                    curLine.staticInfo.push_back(curError);
                    valid = 0;
                }
            }
            else {
                //记录第几行，第几列出现什么样的错误
                //这里是E后面 非法指数 invalid expo, E后面应该(+|-)digits
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "invalid exponent defination, (+|-)digits should follow behind E";
                curLine.staticInfo.push_back(curError);
                valid = 0;
            }
        }
        else if ((notDigit(ch) && ch != '*') || ch == '.') {
            //非法数字常量，数字后面不应该跟字母或下划线
            errorInfo curError;
            curError.line = lineCount;
            curError.col = curLine.charaCount;
            curError.errorInfo = "invalid const number defination, should not follow alpha or underline";
            curLine.staticInfo.push_back(curError);
        }
    }
    else if (ch == 'E' || ch == 'e') {
        myDynStringChcat(&tkstr, ch);
        ch = getChar();
        if (ch == '+' || ch == '-') {
            char t = *buffer->cur;
            if (isDigit(t)) {
                myDynStringChcat(&tkstr, ch);
                myDynStringChcat(&tkstr, t);
                buffer->cur++;
                ch = getChar();
                while (isDigit(ch)) {
                    myDynStringChcat(&tkstr, ch);
                    ch = getChar();//这里可能多读了一个字符
                }
            }
            else {
                //非法指数 +/- 后面应该是数字
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "invalid Exponent defination, digit should follow behind the (+|-)";
                curLine.staticInfo.push_back(curError);
                valid = 0;
            }
        }
        else if (isDigit(ch)) {
            do {
                myDynStringChcat(&tkstr, ch);
                ch = getChar();
            } while (isDigit(ch));
            if (notDigit(ch) && ch != '*') {
                //非法标识符，数字后面不应该跟字母或下划线
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "invalid identifier defination, should not start with digit";
                curLine.staticInfo.push_back(curError);
                valid = 0;
            }
        }
        else {
            //记录第几行，第几列出现什么样的错误
            //这里是E后面 非法指数 invalid expo, E后面应该(+|-)digits
            errorInfo curError;
            curError.line = lineCount;
            curError.col = curLine.charaCount;
            curError.errorInfo = "invalid exponent defination, (+|-)digits should follow behind E";
            curLine.staticInfo.push_back(curError);
            valid = 0;
        }
    }
    else if (ch == ',') {

    }
    else if (notDigit(ch)) {
        errorInfo curError;
        curError.line = lineCount;
        curError.col = curLine.charaCount;
        curError.errorInfo = "invalid const number defination, should not follow alpha or underline";
        curLine.staticInfo.push_back(curError);
    }

    myDynStringChcat(&tkstr, '\0');
    result->str = (char*)malloc(sizeof(char) * tkstr.len + 1);
    memcpy(result->str, tkstr.data, tkstr.len + 1);

    if (!valid) return;

    //接下来是对数字进行分类
    if (tkstr.len == 2)
        token = result->code = CPP_CINT;  //const int 整型常量
    else {
        int float_flag = NOT_FLOAT; // 先默认是非浮点数
        int index = 0;
        token = result->code = CPP_N_DECIMAL;
        if (tkstr.data[index] == '0') {
            radix = 8;  //非浮点数，开头是0，则不可能是十进制
            index++;
            token = result->code = CPP_N_OCTAL;
            if ((tkstr.data[index] == 'x' || tkstr.data[index] == 'X') &&
                (tkstr.data[index + 1] == '.' || isXDigit(tkstr.data[index + 1]))) {
                radix = 16;
                index++;
                token = result->code = CPP_N_HEX;
            }//16进制
            else if (tkstr.data[index] == 'b' || tkstr.data[index] == 'B' &&
                (tkstr.data[index + 1] == '0' || tkstr.data[index + 1] == '1')) {
                radix = 2;
                index++;
                token = result->code = CPP_N_BINARY;
            }//2进制
        }//只处理了1开头的数字的第一位

        for (;;) {
            char c = tkstr.data[index++];
            if (isDigit(c) || (isXDigit(c) && radix == 16)) {
                //扫过所有小数点前面或者x前面的数字，小数点后面的数字也可以扫过，但是科学计数法符号后的数字不能扫
            }
            else if (c == '.') {//如果有小数点，说明他是个浮点数
                float_flag = AFTER_POINT;
            }
            else if ((radix <= 10 && (c == 'E' || c == 'e')) ||
                (radix <= 16 && (c == 'P' || c == 'p'))) {//科学计数法
                float_flag = AFTER_EXPON;
                break;//不用继续识别科学计数法后面部分的数字
            }
            else {
                index--;
                break;
            }
        }

        if (radix == 8 && float_flag != NOT_FLOAT) {//8进制数但是包含科学计数法
            radix = 10;
        }

        if (float_flag != NOT_FLOAT) {
            if (float_flag == AFTER_EXPON) {
                if (tkstr.data[index] == '+' || tkstr.data[index] == '-') {
                    index++;
                    do {
                        index++;
                    } while (isDigit(tkstr.data[index]));
                }
            }
            token = result->code = CPP_CFLOAT;
        }
        else {
            //如果是浮点数，就不分进制了，都是CPP_CFLOAT
            //如果是整形，就分 CPP_DECIMAL | CPP_BINARY | CPP_HEX | CPP_OCTAL
        }
    }
}

//main lexer func

void lexerDirect() {
    tkWord result;//用来存储词法分析的五元组(这里只是两元组)
//  ch = getChar();
    switch (ch) {//ch就很像那个startPtr，看能不能合到一起，这样ch=getChar()能不能去掉
        //....  本来是\n 但是 \r\n只会同时出现，所以\n全都处理掉了

    case ' ': case '\t': case '\f': case '\v': case '\r':
        skipWhiteSpace();
        charCount--;
        buffer->cur--;
        curLine.charaCount--;
        token = -1;
        break;
        //....

    case '\n':
        printf("%c", ch);
        token = -1;
        lineCount++;
        break;

    case '\'': case '\"':
        parseString(&result);
        token = result.code;
        constCount++;
        curLine.constCount++;
        break;
        //....

    case '/':
    {
        char c = *buffer->cur;
        if (c == '*') {
            ch = getChar();
            parseComment();
            //....
        }
        else if (c == '/') {//看看换行符前一个是不是\符号,如果是\符号，那他下一行还是注释
            // ... \\qweqweqweqweasdqwed\
                    weqweadadw
            char* find;
            find = (char*)malloc(sizeof(char) * BUFFER_MAX);
            find = strchr(buffer->data, '\\');
            while (find) {
                if ((*(find + 1) == '\r') && (*(find + 2) == '\n')) {
                    do {
                        ch = getChar();
                    } while (ch != '\n');
                    find = strchr(buffer->data, '\\');
                }
                else {
                    find = strchr(find + 1, '\\');
                }
                //*(find + 1) == '\n'   nextLine() : [&]()->void{find = strchr(find, '\\');}
            }
            while ((!find) && ch != '\n' && ch != '\0') {
                ch = getChar();
            }
            if (ch == '\n') {
                lineCount++;
                printf("\n");
                token = -1;
            }
            if (ch == '\0') {
                buffer->cur--;
                charCount--;
                curLine.charaCount--;
                token = -1;
            }
        }//看一下能不能和parseComment()合到一起
        else if (c == '=') {
            ch = getChar();
            result.code = CPP_DIV_EQ;
            token = result.code;
        }
        else {
            result.code = CPP_DIV;
            token = result.code;
        }
        //....
        break;
    }
    /*      这一段暂时先不管，还没搞清具体是啥意思
            case 'L':
            case 'u':
            case 'U':
            case 'R':
                if (ch == 'L' || CPP_OPTION(pfile, rliterals) || (c != 'R' && CPP_OPTIN(pfile, uliterals)) {
                    if ((buffer.str == '\'' && ch != 'R')
                        || buffer.str == '"'
                        || (buffer.str == 'R' && ch = 'R' && buffe.str[1] == '"' && CPP_OPTION(pfile, rliterals))
                        || (*buffer->cur == '8' && ch == 'u' && (buffer->cur[1] == '"' || (buffer->cur[1] == 'R' && buffer->cur[2] == '"' && CPP_OPTION(pfile, rliterals))))
                        )
                    {
                        parseString(pfile, result, buffer->cur - 1);
                        break;
                    }
                }
    */
    //classify number
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
        result.code = CPP_NUMBER;
        parseNumber(&result);
        constCount++;
        curLine.constCount++;
        token = result.code;
        buffer->cur--;
        charCount--;
        curLine.charaCount--;
    }
    break;

    //identifier
    case '_':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z':
        result.code = CPP_IDENT;
        {
            //....
            parseIdentifier(&result);
            token = result.code;
            buffer->cur--;
            charCount--;
            curLine.charaCount--;
            if (token < CPP_OPERA) {

            }
            else {
                curLine.identificatorCount++;
                identifierCount++;
            }
            //...
        }
        break;

        //operator&&delimiter

        //看看能否解析头文件
    case '<':
        token = result.code = CPP_LESS;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '=') {
            ch = getChar();
            result.code = CPP_LESS_EQ;
            token = CPP_LESS_EQ;
        }
        else if (*buffer->cur == '<') {
            ch = getChar();
            result.code = ifNexIs('=', CPP_LSHIFT_EQ, CPP_LSHIFT);
            token = result.code;
        }
        else if (isDigit(*buffer->cur)) {
            break;
        }
        /*
        else if (notDigit(*buffer->cur)) {
            parseString(&result);//如果没有对应的> 说明他就是一个<符号，所以就可以让result.code = CPP_LESS
            angleMatching++;
            constCount++;
            curLine.constCount++;
        }*/

        //....
        break;

    case '>'://
        token = CPP_GREATER;
        curLine.operatorCount++;
        operatorCount++;
        result.code = CPP_GREATER;
        if (*buffer->cur == '=') {
            ch = getChar();
            result.code = CPP_GREATER_EQ;
            token = CPP_GREATER_EQ;
        }
        else if (*buffer->cur == '>') {
            ch = getChar();
            result.code = ifNexIs('=', CPP_RSHIFT_EQ, CPP_RSHIFT);
            token = result.code;
        }
        //....
        break;
        //....

    case '+':
        token = result.code = CPP_PLUS;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '+')
            ch = getChar(), token = result.code = CPP_PLUSPLUS;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_PLUS_EQ;
        //....
        break;

    case '&':
        token = result.code = CPP_ARITH_AND;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '&')
            ch = getChar(), token = result.code = CPP_LOGIC_AND;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_AND_EQ;
        //....
        break;

    case '|':
        token = result.code = CPP_ARITH_OR;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '|')
            ch = getChar(), token = result.code = CPP_LOGIC_OR;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_OR_EQ;
        //....
        break;

    case '-':
        result.code = token = CPP_MINUS;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '>') {
            ch = getChar();
            result.code = CPP_PTR;
            token = CPP_PTR;
            curLine.boundaryCount++;
            boundaryCount++;
            curLine.operatorCount--;
            operatorCount--;
            if (*buffer->cur == '*') {
                ch = getChar();
                result.code = CPP_PTR_STAR;
                token = CPP_PTR_STAR;
            }
        }
        else if (*buffer->cur == '-') {
            ch = getChar();
            result.code = token = CPP_MINUSMINUS;
        }
        else if (*buffer->cur == '=') {
            ch = getChar();
            token = result.code = CPP_MINUS_EQ;
        }
        break;

    case '%':
        result.code = token = CPP_MOD;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_MOD_EQ;
        //....
        break;

    case '*':
        result.code = token = CPP_MULT;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_MULT_EQ;
        //....
        break;

    case '=':
        token = result.code = CPP_ASSIGN;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_EQ;
        //....
        break;

    case '!':
        token = result.code = CPP_NOT;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_NOT_EQ;
        //....
        break;

    case '^':
        token = result.code = CPP_XOR;
        curLine.operatorCount++;
        operatorCount++;
        if (*buffer->cur == '=')
            ch = getChar(), token = result.code = CPP_MOD_EQ;
        //....
        break;

    case '.':
        //...
        token = result.code = CPP_DOT;
        if (isDigit(*buffer->cur)) {
            //....
            token = result.code = CPP_NUMBER;
            parseNumber(&result);
            constCount++;
            curLine.constCount++;
            //....
        }
        else if (*buffer->cur == '.' && buffer->cur[1] == '.') {
            getChar(), getChar();
            token = result.code = CPP_ELLIPSIS;
            curLine.definatorCount++;
            definatioinCount++;
        }
        else {
            curLine.operatorCount++;
            operatorCount++;
        }
        break;
        //...

    case '@':
        token = CPP_AT;
        break;

    case '$':
        token = CPP_DOLLAR;
        break;
        //add file head #include || #define func
    case '#':
        token = result.code = CPP_HASH;
        curLine.boundaryCount++;
        boundaryCount++;
        if (*buffer->cur == '#') {
            ch = getChar();
            token = result.code = CPP_PASTE;
        }
        /*
        else if (*buffer->cur == 'd' || *buffer->cur == 'i' || *buffer->cur == 'p') { //#define #include
            char c = ch;
            parseIdentifier(&result);
            curLine.identificatorCount++;
            identifierCount++;
            token = result.code = CPP_IDENT;
            if (strcmp("#define", result.str) || strcmp("#include", result.str) || strcmp("#ifndef", result.str) || strcmp("#pragma", result.str)) {
            }
            else if (c == 'd') {
                //should be define but write as result.str
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "invalid defination, should be #define";
                curLine.staticInfo.push_back(curError);
            }
            else {
                //should be include but write as result.str
                errorInfo curError;
                curError.line = lineCount;
                curError.col = curLine.charaCount;
                curError.errorInfo = "invalid reference, should be #include or #ifndef";
                curLine.staticInfo.push_back(curError);
            }
        }*/
        break;

    case ':':
        token = result.code = CPP_COLON;
        curLine.boundaryCount++;
        boundaryCount++;
        if (*buffer->cur == ':') {
            ch = getChar();
            token = result.code = CPP_COLON_COLON;
        }
        break;

    case '?':
        token = result.code = CPP_QUERY;
        curLine.boundaryCount++;
        boundaryCount++;
        break;

    case '~':
        token = result.code = CPP_COMPL;
        curLine.boundaryCount++;
        boundaryCount++;
        break;

    case ',': token = result.code = CPP_COMMA;
        curLine.boundaryCount++;
        boundaryCount++;
        break;

        //检查右括号
    case '(':
        token = result.code = CPP_OPEN_PAREN;
        parenMatching++;
        curLine.boundaryCount++;
        boundaryCount++;
        break;

    case '[':
        token = result.code = CPP_OPEN_SQUARE;
        squareMatching++;
        curLine.boundaryCount++;
        boundaryCount++;
        break;

    case '{':
        token = result.code = CPP_OPEN_BRACE;
        braceMatching++;
        curLine.boundaryCount++;
        boundaryCount++;
        break;

    case ')':
        token = result.code = CPP_CLOSE_PAREN;
        curLine.boundaryCount++;
        boundaryCount++;
        parenMatching--;
        break;

    case ']':
        token = result.code = CPP_CLOSE_SQUARE;
        curLine.boundaryCount++;
        boundaryCount++;
        squareMatching--;
        break;

    case '}':
        token = result.code = CPP_CLOSE_BRACE;
        curLine.boundaryCount++;
        boundaryCount++;
        braceMatching--;
        break;

    case ';'://一般如果这一行'\r'前面的最后一个字符是: { } 那么不用; 否则都需要分号，所以staticOnLine 可以统计这一行最后一个字符存的是什么，如果发现缺少;可以报错
        token = result.code = CPP_SEMICOLON;
        //noMatchingHandler();
        curLine.boundaryCount++;
        boundaryCount++;
        break;

    case  -1: case '\0':
        over = true;
        token = -1;
        break;

    default:
        break;
    }
    ch = getChar();
}


void lexerInit() {
    tkWord* temp;
    static tkWord keywords[] = {

        { CPP_NAME, NULL,   (char*)"identifier",    NULL,   NULL },     //definator
        { KW_CHAR,  NULL,   (char*)"char",  NULL,   NULL },         //char keyword
        { KW_DOUBLE,    NULL,   (char*)"double",    NULL,   NULL },         //double keyword
        { KW_FLOAT, NULL,   (char*)"float", NULL,   NULL },         //float keyword
        { KW_INT,   NULL,   (char*)"int",   NULL,   NULL },             //int keyword
        { KW_LONG,  NULL,   (char*)"long",  NULL,   NULL },     //long keyword
        { KW_BOOL,  NULL,   (char*)"bool",  NULL,   NULL },     //bool keyword
        { KW_REGISTER,  NULL,   (char*)"register",  NULL,   NULL },     //register keyword
        { KW_SHORT, NULL,   (char*)"short", NULL,   NULL },     //short keyword
        { KW_SIGNED,    NULL,   (char*)"signed",    NULL,   NULL },     //signed keyword
        { KW_STATIC,    NULL,   (char*)"static",    NULL,   NULL },     //static keyword
        { KW_STRUCT,    NULL,   (char*)"struct",    NULL,   NULL },     //struct keyword
        { KW_UNSIGNED,  NULL,   (char*)"unsigned",  NULL,   NULL },     //unsigned keyword
        { KW_VOID,  NULL,   (char*)"void",  NULL,   NULL },     //void keyword
        { KW_CHAR_STAR, NULL,   (char*)"char*", NULL,   NULL },     //char* keyword
        { KW_DOUBLE_STAR,   NULL,   (char*)"double*",   NULL,   NULL },     //double* keyword
        { KW_FLOAT_STAR,    NULL,   (char*)"float*",    NULL,   NULL },     //float* keyword
        { KW_INT_STAR,      NULL,   (char*)"int*",  NULL,   NULL },     //int* keyword
        { KW_LONG_STAR,     NULL,   (char*)"long*", NULL,   NULL }, //long* keyword
        { KW_BOOL_STAR,     NULL,   (char*)"bool",  NULL,   NULL }, //bool* keyword
        { KW_SHORT_STAR,    NULL,   (char*)"short*",    NULL,   NULL },     //short* keyword
        { KW_VOID_STAR,     NULL,   (char*)"void*", NULL,   NULL },//void* keyword
        { KW_VOLATILE,  NULL,   (char*)"volatile",  NULL,   NULL }, //volatile keyword

        { KW_IF,    NULL,   (char*)"if",    NULL,   NULL },             //if keyword
        { KW_ELSE,  NULL,   (char*)"else",  NULL,   NULL },         //else keyword
        { KW_FOR, NULL, (char*)"for", NULL, NULL },             //for keyword
        { KW_CONTINUE, NULL, (char*)"continue", NULL, NULL },       //continue keyword
        { KW_BREAK, NULL, (char*)"break", NULL, NULL },         //break keyword
        { KW_RETURN, NULL, (char*)"return", NULL, NULL },           //return keyword
        { KW_SIZEOF, NULL, (char*)"sizeof", NULL, NULL },           //sizeof keyword
        { KW_WHILE, NULL, (char*)"while", NULL, NULL },         //while keyword
        { KW_AUTO, NULL, (char*)"auto", NULL, NULL },           //auto keyword
        { KW_CASE, NULL, (char*)"case", NULL, NULL },           //case keyword
        { KW_CONST, NULL, (char*)"const", NULL, NULL },         //const keyword
        { KW_DEFAULT, NULL, (char*)"default", NULL, NULL },         //default keyword
        { KW_DO, NULL, (char*)"do", NULL, NULL },               //do keyword
        { KW_ENUM, NULL, (char*)"enum", NULL, NULL },           //enum keyword
        { KW_EXTERN, NULL, (char*)"extern", NULL, NULL },           //extern keyword
        { KW_GOTO, NULL, (char*)"goto", NULL, NULL },           //goto keyword
        { KW_SWITCH, NULL, (char*)"switch", NULL, NULL },           //switch keyword
        { KW_TYPEDEF, NULL, (char*)"typedef", NULL, NULL },         //typedef keyword,
        { KW_UNION, NULL, (char*)"union", NULL, NULL },         //union keyword
        { KW_NULL, NULL, (char*)"NULL", NULL, NULL },           //NULL keyword
        { kW_B_TRUE,    NULL,   (char*)"TRUE",  NULL,   NULL },     //True keyword
        { KW_B_FALSE,   NULL,   (char*)"FALSE", NULL,   NULL },     //False keyword
        { KW_L_TRUE,    NULL,   (char*)"true",  NULL,   NULL },     //true keyword
        { KW_L_FALSE,   NULL,   (char*)"false", NULL,   NULL },     //false keyword

        { PREV_WHITE,   NULL,   (char*)"white space"    ,NULL,  NULL },         //white space
        { CPP_ELLIPSIS, NULL,   (char*)"...",       NULL,   NULL }, //...
        { CPP_AT,   NULL,   (char*)"@",     NULL,   NULL }, //@
        { CPP_DOLLAR,   NULL,   (char*)"$",     NULL,   NULL }, //$

        { CPP_EOF,  NULL,   (char*)"eof",   NULL,   NULL },     //end of file

        //运算符
        { CPP_OPERA,        NULL,   (char*)"operator",      NULL,   NULL }, //operator
        { CPP_NOT,  NULL,   (char*)"!", NULL,   NULL },//!
        { CPP_PLUS, NULL,   (char*)"+", NULL,   NULL },//+
        { CPP_MINUS,    NULL,   (char*)"-", NULL,   NULL },//-
        { CPP_MULT, NULL,   (char*)"*", NULL,   NULL },//*
        { CPP_DIV, NULL, (char*)"/", NULL, NULL },// /
        { CPP_MOD,  NULL,   (char*)"%", NULL,   NULL },//%
        { CPP_PLUS_EQ,  NULL,   (char*)"+=",    NULL,   NULL },//+=
        { CPP_MINUS_EQ, NULL,   (char*)"-=",    NULL,   NULL },//-=
        { CPP_DIV_EQ,   NULL,   (char*)"/=",    NULL,   NULL },///=
        { CPP_MULT_EQ,  NULL,   (char*)"*=",    NULL,   NULL },//*=
        { CPP_PLUSPLUS, NULL,   (char*)"++",    NULL,   NULL },//++
        { CPP_MINUSMINUS,   NULL,   (char*)"--",    NULL,   NULL },//--
        { CPP_MOD_EQ,   NULL,   (char*)"%=",    NULL,   NULL },//%=
        { CPP_EQ,   NULL,   (char*)"==",    NULL,   NULL },//==
        { CPP_NOT_EQ,   NULL,   (char*)"!=",    NULL,   NULL },//!=
        { CPP_LESS, NULL,   (char*)"<", NULL,   NULL },//<
        { CPP_GREATER,  NULL,   (char*)">", NULL,   NULL },//>
        { CPP_LESS_EQ,  NULL,   (char*)"<=",    NULL,   NULL },//<=
        { CPP_GREATER_EQ,   NULL,   (char*)">=",    NULL,   NULL },//>=
        { CPP_LSHIFT,   NULL,   (char*)"<<",    NULL,   NULL },//<<
        { CPP_RSHIFT,   NULL,   (char*)">>",    NULL,   NULL },//>>
        { CPP_LSHIFT_EQ,    NULL,   (char*)"<<=",   NULL,   NULL },//<<=
        { CPP_RSHIFT_EQ,    NULL,   (char*)">>=",   NULL,   NULL },//>>=
        { CPP_ASSIGN,   NULL,   (char*)"=", NULL,   NULL },//=
        { CPP_PTR,  NULL,   (char*)"->",    NULL,   NULL },//->
        { CPP_PTR_STAR, NULL,   (char*)"->*",   NULL, NULL },//->*
        { CPP_DOT,  NULL,   (char*)".", NULL,   NULL },//.
        { CPP_LOGIC_AND,    NULL, (char*)"&&",  NULL,   NULL },//&&
        { CPP_ARITH_AND,    NULL,   (char*)"&", NULL,   NULL },//&
        { CPP_AND_EQ,   NULL,   (char*)"&=",    NULL,   NULL },//&=
        { CPP_LOGIC_OR, NULL,   (char*)"||",    NULL,   NULL },//||
        { CPP_ARITH_OR, NULL,   (char*)"|", NULL,   NULL },//|
        { CPP_OR_EQ,        NULL,   (char*)"|=",    NULL,   NULL },//|=
        { CPP_XOR,  NULL,   (char*)"^", NULL,   NULL },//^
        { CPP_XOR_EQ,   NULL,   (char*)"^=",    NULL,   NULL }, //^=

        //界符
        { CPP_BOUNDARY, NULL,   (char*)"delimiter", NULL,   NULL }, //boundary
        { CPP_OPEN_PAREN,   NULL,   (char*)"(", NULL,   NULL },//(
        { CPP_CLOSE_PAREN,  NULL,   (char*)")", NULL,   NULL },//)
        { CPP_OPEN_SQUARE,  NULL,   (char*)"[", NULL,   NULL },//[
        { CPP_CLOSE_SQUARE, NULL,   (char*)"]", NULL,   NULL },//]
        { CPP_OPEN_BRACE,   NULL,   (char*)"{", NULL,   NULL }, //{
        { CPP_CLOSE_BRACE,  NULL,   (char*)"}", NULL,   NULL },//}
        { CPP_QUERY,        NULL,   (char*)" ", NULL,   NULL }, //
        { CPP_COMPL,        NULL,   (char*)"~", NULL,   NULL },     //~
        { CPP_COMMA,        NULL,   (char*)",", NULL,   NULL }, //,
        { CPP_SEMICOLON,        NULL,   (char*)";", NULL,   NULL },//;
        { CPP_HASH,     NULL,   (char*)"#", NULL,   NULL },     //#
        { CPP_PASTE,        NULL,   (char*)"##",    NULL,   NULL }, //##
        { CPP_COLON,        NULL,   (char*)":", NULL,   NULL },         //:
        { CPP_COLON_COLON,  NULL,   (char*)"::",    NULL,   NULL },//::

        //常量
        { CPP_NUMBER,   NULL,   (char*)"number",    NULL,   NULL },         //number
        { CPP_CINT, NULL,   (char*)"const int", NULL,   NULL },     //const int
        { CPP_CFLOAT,   NULL,   (char*)"const float",   NULL,   NULL},  //const float
        { CPP_CCHAR,    NULL,   (char*)"const char",    NULL,   NULL },     //const char
        { CPP_CSTR, NULL,   (char*)"const char*",   NULL,   NULL },     //const char*

        { NOT_FLOAT,    NULL,   (char*)"not float", NULL,   NULL },         //not a float number
        { AFTER_POINT,  NULL,   (char*)"get the mark of float", NULL,   NULL },     //already find the mark of the float number
        { AFTER_EXPON,  NULL,   (char*)"scientific mark",   NULL,   NULL }, //scientific count mark

        { CPP_N_DECIMAL,    NULL,   (char*)"dec",   NULL,   NULL },     //decimal number
        { CPP_N_HEX,    NULL,   (char*)"hex",   NULL,   NULL },         //hex number
        { CPP_N_BINARY, NULL,   (char*)"bin",   NULL,   NULL }, //binary number
        { CPP_N_OCTAL,  NULL,   (char*)"oct",   NULL,   NULL },     //octal number

        //标识符
        { CPP_IDENT,    NULL,   (char*)"identifier",    NULL,   NULL}           //identifier
    };

    myDynArrayInit(&tkTable, 8);
    int i = 0;
    for (temp = &keywords[0]; temp->str != NULL; temp++) {
        tkInsertKeyWord(temp);
    }

    fgets(buffer->data, BUFFER_MAX, fin);
    buffer->cur = &buffer->data[0];
}

void lexerCleanup() {
    printf("\ntkTable length = %d\n", tkTable.len);
    //  for (int i = 0; i < tkTable.len; i++) {
    //      free(tkTable.data[i]);
    //  }
    free(tkTable.data);
}

void warning(char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    exceptionHandle(COMPLIE, CPP_WARNING, fmt, ap);
    va_end(ap);
}

void except(char* msg) {
    error("shortage %s \n", msg);
}

void skip(int c) {
    if (token != c)
        error("shortage of '%s'", getTkstr(c));
    lexerDirect();
}

void linkError(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    exceptionHandle(LINK, CPP_ERROR, fmt, ap);
    va_end(ap);
}

void infoDisplay() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    printf("\n");

    if (debuglevel == "2")
        for (int i = 0; i < tokenCount; i++) {
            cout << "<" << tokenStream[i].first << ",\t" << tokenStream[i].second << ">" << endl;
        }

    printf("\n");
    int errorCount = 0;
    for (int i = 0; i < lexError.size(); i++) {
        for (int j = 0; j < lexError[i].staticInfo.size(); j++) {
            errorCount++;
            printf("%d Compile Error: <line:%d, col:%d>", errorCount, lexError[i].staticInfo[j].line, lexError[i].staticInfo[j].col);
            cout << lexError[i].staticInfo[j].errorInfo << endl;
        }
    }

    printf("\n代码行数 : %d", lineCount);
    printf("\n字符个数 %d", charCount);
    printf("\n单词个数 %d\n", identifierCount + definatioinCount);

    if (debuglevel == "2") {
        printf("\n统计各单词符号出现次数\n");
        map<string, int>::iterator strmap_iter = CountToken.begin();
        for (; strmap_iter != CountToken.end(); strmap_iter++)
        {
            cout << "<" << strmap_iter->first << ' ' << strmap_iter->second << ">" << endl;
        }
        cout << endl;
    }

    printf("\n定义符 %d\n", definatioinCount);
    printf("界符 %d\n", boundaryCount);
    printf("运算符 %d\n", operatorCount);
    printf("常量 %d\n", constCount);
    printf("标识符 %d\n", identifierCount);

    printf("总共 %d\n", tokenCount);

}

void init_tokens(){
    for(auto i = 0 ; i < tokens_type.size() ; i++){
        tokens.push_back(getTkstr(tokens_type[i]));
    }
    for(auto i = 0 ; i < tokens.size() ; i++){
        cout << tokens[i] << " " ;
    }
    cout << endl;
}

int lexerinit(int argc, char** argv) {// arg[0] .exe的名称 | arg[1] src.c |  arg[2] grammer.json | arg[3] debug_level |
    if (argc == 4) {
        debuglevel = argv[3];
    }

    fin = fopen(argv[1], "rb");
    filename = (char*)malloc(sizeof(char) * strlen(argv[1]) + 1);
    memcpy(filename, argv[1], strlen(argv[1]) + 1);
    if (!fin) {
        printf("can't open source file!\n");
        return 0;
    }

    lineCount = 1;

    lexerInit();//符号表初始化，缓冲区初始化，缓冲区从文件中读入第一行

    preprocess();//预处理，去掉注释
    while (ch != -1) {
        lexerDirect();
        if (token >= 0)
            tokens_type.push_back(token);
        if (over || ch == '\0')
            break;//endoflexer();
    }

}

void endoflexer(){

    //infoDisplay();
    lexerCleanup();
}

int lexer(){
    if (ch != -1) {
        lexerDirect();
        if (token >= 0)
            return token;
        if (over || ch == '\0')
            endoflexer();
    }
}

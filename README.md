# Parser
> **Structure**\
![structure](structure.jpg)

> **Pipeline**
![](workstream.jpg)

> **grammar Defination**
>>*context-free grammar* \
G[s] := <V<sub>N</sub>, V<sub>T</sub>, P, S>

>>V<sub>N</sub> = {} \
V<sub>T</sub> = {} \
S = {}


    Cpp Productions::

    <declaration> -> [<declaration_descriptor>]<R1>
    <R1> -> <declaration_notion><R2>|;
    <R2> -> <R5><R4>; | [<declaration_list>]<composition_sentence>
    <R4> -> <R3><R4> | epsilon 
    <R5> -> ,<declaration_init>
    <R5> -> =<content_init> | epsilon 

    <declaration_init> -> <declaration_notion><R5>

    <declaration_descriptor> -> <store_type>[<declaration_descriptor>]
                                | <type>[<declaratioin_descriptor>]
                                | <type_constrant>[<declaratioin_descriptor>]
                                | <func>[<declaratioin_descriptor>]
                                | <algin>[<declaratioin_descriptor>]


    <declaration_notion> -> [<pointer>]<direct_declaration>

    <type> -> int | long | char | double | ...

    <direct_declaration> -> <identifier><T4>
                            | ( <declaration_notion> )<T4>

    <T4> -> <T1> | <T5> | epsilon
    <T1> -> <array_declaration> T1 | epsilon
    <T5> -> ( <T5> )
    <T6> -> <params_type_list> | <identifier_list> | epsilon

    <array_declaration> -> [ [type_constrant_list][<copy_expression>] ]

    <params_type_list> -> <param_list><T9>

    <T9> -> , ... | epsilon

    <params_list> -> <params_declaration><T10>
    <T10> -> ,<params_list> | epsilon

    <params_declaration> -> <declaration_descriptor><declaration_notion>


> **We use json to load the grammar of the language** 
>> **V<sub>N</sub>** :: *non_terminals = {}* \
>> **V<sub>T</sub>** :: *terminals = {}* \
>> **S** :: *start_symbols = {}* \
>> **productions** ={
>>          <p>&emsp; &emsp;*{ E -> E + T | E - T | T }*, \
>> &emsp; &emsp;            *{ T -> T * F | T / F | F }*, \
>> &emsp; &emsp;            *{ F -> ( E ) | num }*</p>
    }  

> ***grammar.json***
```json
{
    "Type": "Grammar",
    "non_terminals": "E T F",
    "terminals": "+ - * / ( ) num",
    "start_symbol": "E",
    "productions": [
        {
            "left": "E",
            "candidate": [
                "E+T",
                "E-T",
                "T"
            ]
        },
        {
            "left": "T",
            "candidate":[
                "T*F",
                "T/F",
                "F"
            ]
        },
        {
            "left": "F",
            "candidate": [
                "(E)",
                "num"
            ]
        }
    ]
}
```

> ***Eliminate the left recursion***
> - direct left recursion
> - indirect left recursion
-
    **left recursion** ::
    ![fomula](1.jpg)
    ![fomula](2.jpg)
- 
  **if there exists many productions that include left recursion** \
  we should reorder if and eliminate all the productions contains left recursion 
- **code as follow**
    ```C
    if(grammar.contains("production") &&
            grammar.value("production").isArray()){
            QJsonArray Ps = grammar["production"].toArray();
            for (int i = 0 ; i < Ps.size() ; i++) {
                if(Ps[i].isObject()){
                    QJsonObject rule = Ps[i].toObject();
                    if(rule.contains("left") &&
                            rule.contains("candidate") &&
                            rule["left"].isString() &&
                            rule["candidate"].isArray()){
                        cout << "[" << i <<"th rule: ]" << rule["left"].toString().toStdString() << " -> ";
                        QString leftStr = rule["left"].toString();
                        QJsonArray candidates = rule["candidate"].toArray();
                        QString leftExtendStr = leftStr+"'";

                        MyQString left;
                        left.leftStr = leftStr;

                        MyQString leftExtend;
                        leftExtend.leftStr = leftExtendStr;
                        productions[left] = {};
                        for (int j = 0 ; j < candidates.size() ; j++ ) {
                            if(j>0){
                                cout<<" | ";
                            }
                            cout<<candidates[j].toString().toStdString();
                            if(candidates[j].toString().indexOf(leftStr) != 0){//没有左递归的表达式，利用βA' 放入容器当中
                                if(non_terminals.contains(leftExtendStr)){
                                    QString betaExtend = candidates[j].toString() + leftExtendStr;
                                    productions[left].push_back(betaExtend);
                                }
                                else{
                                    QString tmp = candidates[j].toString();
                                    productions[left].push_back(tmp);
                                }
                            }
                            else{//该候选式含有左递归
                                QVector<QString>::iterator iter=find(non_terminals.begin(),non_terminals.end(), leftExtendStr);
                                if(iter == non_terminals.end()){
                                    non_terminals.push_back(leftExtendStr);
                                }
                                if(non_terminals.contains(leftExtendStr)){//已经拓展了符号
                                    QString alpha = candidates[j].toString().mid(1);
                                    alpha+=leftExtendStr;
                                    productions[leftExtend].push_back(alpha);
                                }
                            }
                        }
                        QVector<QString>::iterator iter=find(non_terminals.begin(),non_terminals.end(), leftExtendStr);
                        if(iter != non_terminals.end()){
                            productions[leftExtend].push_back("epsilon");
                        }
                        cout<<endl;
                    }
                }
            }
    }
    ```
![eliminate_left_recurent](result1.jpg)
> ***Extract the left common factor***
![fomula](3.jpg)
***Use Tire tree to find the left common factor of the Grammar***
- **data structure**
    ```C++
        class Tire{
            public:
                struct TrNode{
                    QChar ch;
                    bool isEnd = false;
                    int count = 0;
                    TrNode(){
                        ...
                    }
                    TrNode(MyQChar ch, bool isEnd){
                        ...
                    }
                    QMap<MyQChar, TrNode*> childs;
                };

                Tire(){
                    root = new TrNode();
                }

                TrNode* getRoot(){
                    return root;
                }

                void insert(QString word){
                    ...
                }

                TrNode* searchPrefix(QString word){
                    ...
                }

                bool search(QString word){
                    ...
                }

                bool startwith(QString prefix){
                    ...
                }
                // abc123 | abt986 | cd5777 | cd699 | e
                // return {"ab", "cd", ""}
                // 也就是字典树节点计数不等于一就可以
                void searchLeftCommonFactor(TrNode* rt, QVector<QString> res_set, QString res){
                    TrNode* tmp = rt;
                    bool flag = false;
                    if(tmp->isEnd || (tmp->childs.size() == 1 && tmp->childs.begin().value()->count == 1)){
                        res_set.push_back(res);
                        return;
                    }
                    for (auto it = tmp->childs.begin(); it != tmp->childs.end(); it++) {
                        if(it.value()->count!=1){
                            flag = true;
                        }
                    }
                    if(flag == false){
                        res_set.push_back(res);
                        return;
                    }

                    for(auto it = tmp->childs.begin(); it != tmp->childs.end(); it++){
                        if(it.value()->count != 1){
                            res += it.key().ch;
                            searchLeftCommonFactor(it.value(), res_set, res);
                        }
                    }
                }

            private:
                TrNode* root;
        };
    ```
- **code as follow**
    ```C++
    for(auto iter = productions.begin(); iter != productions.end(); iter++){
            Tire tire;
            QVector<QString> candidates = iter.value();
            for(int i = 0; i < candidates.size(); i++){
                tire.insert(candidates[i]);
            }
            QVector<QString> common_left;
            tire.searchLeftCommonFactor(tire.getRoot(), common_left, "");
            // new non_terminals
            // new productions
            QSet<QString> newproductions;
            bool* inMatch = (bool*)malloc(sizeof(bool) * (candidates.size()+1));
            for(int j = 0 ; j < candidates.size() ; j++){
                inMatch[j] = false;
            }
            for (int j = 0 ; j < common_left.size() ; j++) {
            std::string num = std::to_string(j);
            QString VN = iter.key().leftStr;
            VN += QString::fromStdString(num);
            non_terminals.push_back(VN);

            for(int k = 0 ; k < candidates.size() ; k++){
                if(!inMatch[k]){
                    inMatch[k] = candidates[k].indexOf(common_left[j]) == 0 ? true : false;
                    if(inMatch[k]){
                        MyQString newVN;
                        newVN.leftStr = VN;
                        if(candidates[k].length() == common_left[j].length())
                            productions[newVN].push_back("epsilon");//是否有epsilon
                        else{
                            productions[newVN].push_back(candidates[k].mid(common_left[j].length()));
                        }
                        candidates[k] = common_left[j] + VN;
                        newproductions.insert(candidates[k]);
                    }
                }
            }
            }
            QVector<QString> updatePro;
            for (auto it = newproductions.begin(); it != newproductions.end(); it++ ){
                updatePro.push_back(QString::fromStdString(it->toStdString()));
            }
            productions[iter.key()] = updatePro;
        }
    ```


> ***first Set***
![formula](4.jpg)
<!-- $$ If\space X \in V_T, first(X) = {X}$$
$$ If\space X \in V_N, X\rightarrow a (a \in V_T \vee a = \epsilon),\space first(X) = first(X) \cup \{a\} $$
$$ If\space X \rightarrow Y_1 Y_2 ... Y_k Y_{k+1}...\space , \epsilon \in 
first(Y_i) (i = 1,...,k),\space first(X) = first(X) \cap \{ a \in first(Y_i) | a \not ={\epsilon} \}$$ -->
- code as follow
```C++
    void calculate_first(string left){
        string eps = "epsilon";
        vector<string> candidates = productions[left];
        for(int i = 0 ; i < candidates.size() ; i++){
            bool all_has_epsilon(true);
            for (int j = 0 ; j < candidates[i].length(); j++) {
                string tmp= candidates[i].substr(j, 1);
                if(tmp=="n"){
                    tmp = candidates[i].substr(j, 3);
                    j+=2;
                }
                if(tmp == "e"){
                    tmp = candidates[i].substr(j, 7);
                    j+=6;
                }
                if(V_T.find(tmp) != V_T.end() || tmp == eps){//是终结符
                    first_of_prod[candidates[i]].insert(tmp);
                    all_has_epsilon = false;
                    break;
                }
                if(!first_constructed[tmp]){
                    calculate_first(tmp);
                }

                for(auto irer = first[tmp].begin(); irer != first[tmp].end() ; irer++){
                    first_of_prod[candidates[i]].insert(*irer);
                }

                if(first[tmp].find(eps) == first[tmp].end()){
                    all_has_epsilon = false;
                    break;
                }else{
                    first_of_prod[candidates[i]].erase(eps);
                }
            }
            if(all_has_epsilon){
                first_of_prod[candidates[i]].insert(eps);
            }
            for(auto iter = first_of_prod[candidates[i]].begin(); iter != first_of_prod[candidates[i]].end() ; iter++){
                first[left].insert(*iter);
            }
        }
        first_constructed[left] = true;
    }

```
![result2](result2.jpg)
> ***follow Set***
<!-- $$If\space X \in Start\_symbols\space ,follow(X) = follow(X) \cup \{ \$ \} $$
$$If\space \exists \space A\rightarrow \alpha B\beta\space $$
$$If\space \beta {\Rightarrow}^* \epsilon$$
$$ follow(A) \subset follow(B)$$
$$ Else\space if\space \beta {\nRightarrow} \epsilon $$
$$ first(\beta) \subset follow(B) $$  -->
![formula](5.jpg)
two $V_N$'s follow set may be contains each other, so the recursion programme may be nonterminable
We notice that 
<!-- $$ follow(A) \subset follow(B) \wedge follow(B) \subset follow(A) \Leftrightarrow follow(A) = follow(B)  $$ -->
![formula](6.jpg)
- code as follow
```C++
    void calculate_follow(string left){
        string eps;
        eps = "epsilon";
        for (auto iter = productions.begin(); iter != productions.end() ; iter++) {
            vector<string> candidates = iter->second;
            for(int i = 0 ; i < candidates.size() ; i++){
                int index = 0 ;
                index = candidates[i].find(left); //left 有可能长度不等于1
                if(index == -1)
                    break;
                for(int j = index + left.length() ; j < candidates[i].size() ; j++){
                    //next 也有可能不是长度为1的非终结符
                    string following;
                    string next = candidates[i].substr(index+1,1);
                    following  = candidates[i].substr(index+1);
                    if(candidates[i].substr(index+2, 1) ==  "'"){
                        next += "'";
                        following = candidates[i].substr(index+1);
                        index += 1;
                    }
                    if(next == "n"){
                        next = candidates[i].substr(index+1, 3);
                        index += 2;
                    }
                    if(next == "e"){
                        next = candidates[i].substr(index+1, 7);
                        index += 6;
                    }

                    if(V_T.find(next) != V_T.end()){
                        follow[left].insert(next);
                        break;
                    }

                    for (auto iter = first[next].begin() ; iter != first[next].end() ; iter++){
                        follow[left].insert(*iter);
                    }

                    if(first_of_prod[following].find(eps) != first_of_prod[following].end()){
                        if(!follow_constructed[iter->first]){
                            calculate_follow(iter->first);
                        }
                        if(follow_contains[iter->first][left]){
                            follow_contains[left][iter->first] = true;
                            follow[left] = follow[iter->first];
                            break;
                        }
                        follow[left].insert(
                                    follow[iter->first].begin(),
                                    follow[iter->first].end()
                                    );
                    }

                    if(first[next].find(eps) == first[next].end()){
                        break;
                    }else{
                        follow[left].erase(eps);
                    }
                }
                // A->αB | β->epsilon
                if(iter->first != left && candidates[i].length() == (index + left.length()) ){
                    follow_contains[left][iter->first] = true;
                    if(!follow_contains[iter->first][left]){
                        if(!follow_constructed[iter->first]){
                            calculate_follow(iter->first);
                        }
                        for(auto ir = follow[iter->first].begin(); ir != follow[iter->first].end() ; ir++){
                            follow[left].insert(*ir);
                        }

                    }
                    else{
                        follow[left] = follow[iter->first];
                    }
                }
            }
        }
        follow_constructed[left] = true;
    }


```
![result](result3.jpg)
> ***LL(1) Table***
>> It's difficult to align the table ::x::
![result](result4.jpg)


> ***Non-recursive predictive analysis*** 
>> Use a Symbol Stack which has $ and start_symbol at first
```
    +-------+
    |   S   |
    +-------+
    |   $   |
    +-------+               input string abcd...xyz

```
find [S,a] in LL(1) Table to get the production going to be used
example1 :
![result](result5.jpg)
![result](result6.jpg)

example 2 :\
![result](result7.jpg)
![rssult](result8.jpg)

[To Do List]
> ***Abstract Syntax Tree***
>> use graphviz 2.x to visualize a AST
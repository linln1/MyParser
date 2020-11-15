#include "tire.h"

unordered_set<string> non_terminals;
unordered_set<string> terminals;
unordered_set<string> V_T;
unordered_set<string> V_N;
unordered_map<string, vector<string>> productions;
unordered_map<string, set<string>> first;
unordered_map<string, set<string>> first_of_prod;
unordered_map<string, set<string>> follow;
unordered_map<string, bool> first_constructed;
unordered_map<string, bool> follow_constructed;
unordered_map<string, unordered_map<string, bool>> follow_contains;
unordered_map<string, unordered_map<string, string>> LL1_table;

string start_symbol;
vector<string> tokens;

void extract_left_common_factor(){
    for(auto iter = productions.begin(); iter != productions.end(); iter++){
        Tire tire;
        vector<string> candidates = iter->second;
        if(candidates.size() == 1){
            break;//只有一个候选式，就可以直接退出，不用提取公因子
        }
        for(int i = 0; i < candidates.size(); i++){
            cout << candidates[i] << " ";
            tire.insert(candidates[i]);
        }
        cout << endl;
        vector<string> common_left;
        tire.searchLeftCommonFactor(tire.getRoot(), common_left, "");
        // new non_terminals
        // new productions
        set<string> newproductions;
        bool* inMatch = (bool*)malloc(sizeof(bool) * (candidates.size()+1));
        for(int j = 0 ; j < candidates.size() ; j++){
            inMatch[j] = false;
        }
        for (int j = 0 ; j < common_left.size() ; j++) {
           std::string num = std::to_string(j);
           string VN = iter->first;
           VN += num;
           non_terminals.insert(VN);

           for(int k = 0 ; k < candidates.size() ; k++){
               if(!inMatch[k]){
                   inMatch[k] = candidates[k].find(common_left[j]) == 0 ? true : false;
                   if(inMatch[k]){
                       string newVN;
                       newVN = VN;
                       if(candidates[k].length() == common_left[j].length())
                           productions[newVN].push_back("epsilon");//是否有epsilon
                       else{
                           productions[newVN].push_back(candidates[k].substr(common_left[j].length()));
                       }
                       candidates[k] = common_left[j] + VN;
                       newproductions.insert(candidates[k]);
                   }
               }
           }
        }
        vector<string> updatePro;
        for (auto it = newproductions.begin(); it != newproductions.end(); it++ ){
            updatePro.push_back(*it);
        }
        if(updatePro.size() != 0)
            productions[iter->first] = updatePro;
    }
    return ;
}

void calculate_first(string left){
    string eps = "epsilon";
    vector<string> candidates = productions[left];
    for(unsigned int i = 0 ; i < candidates.size() ; i++){
        bool all_has_epsilon(true);
        for (unsigned int j = 0 ; j < candidates[i].length(); j++) {
            string tmp= candidates[i].substr(j, 1);
            if(tmp=="n"){
                tmp = candidates[i].substr(j, 3);
                j+=2;
            }
            if(tmp == "e"){
                tmp = candidates[i].substr(j, 7);
                j+=6;
            }
            if(j+1 < candidates[i].length() && (candidates[i][j+1] == '\'' || isDigit(candidates[i][j+1]))){
                tmp = candidates[i].substr(j,2);
                j+=1;
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

void init(){
    V_N.insert(non_terminals.begin(), non_terminals.end());
    V_T.insert(terminals.begin(), terminals.end());
    for(auto iter = V_N.begin(); iter != V_N.end(); iter++){
        first_constructed[*iter] = false;
    }
}

void calculate_all_first(){
    init();
    for(auto iter = V_N.begin(); iter != V_N.end(); iter++){
        if(!first_constructed[*iter])
            calculate_first(*iter);
    }
    for(auto iter = first.begin() ; iter != first.end() ; iter++){
        first_of_prod[iter->first].insert(
                    first[iter->first].begin(),
                    first[iter->first].end()
                    );
    }
    follow[start_symbol].insert("$");
    return ;
}

void init_follow(){
    for(auto iter = V_N.begin(); iter!=V_N.end();iter++){
        follow_constructed[*iter] = false;
        for (auto ir = V_N.begin() ; ir != V_N.end() ; ir ++) {
            follow_contains[*iter][*ir] = false;
        }
    }
    return ;
}

void calculate_follow(string left){
    string eps;
    eps = "epsilon";
    for (auto iter = productions.begin(); iter != productions.end() ; iter++) {
        vector<string> candidates = iter->second;
        for(unsigned int i = 0 ; i < candidates.size() ; i++){
            int index = 0 ;
            index = candidates[i].find(left); //left 有可能长度不等于1
            if(index == -1)
                break;
            for(unsigned int j = index + left.length() ; j < candidates[i].size() ; j++){
                //next 也有可能不是长度为1的非终结符
                string following;
                string next = candidates[i].substr(index+1,1);
                following  = candidates[i].substr(index+1);
                if(candidates[i].substr(index+2, 1) ==  "'" || isDigit(candidates[i][index+2])){
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

void calculate_all_follow(){
    init_follow();
    for(auto iter = V_N.begin(); iter != V_N.end() ; iter++){
        if(!follow_constructed[*iter])
            calculate_follow(*iter);
    }
    return ;
}

void generate_LL1_table(){
    for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
        for(unsigned int i = 0; i < productions[*iter].size() ; i++){
            bool has_eps(false);
            for(string terminal : first_of_prod[productions[*iter][i]]){
                if(terminal != "epsilon"){
                    LL1_table[*iter][terminal] = *iter + "->" + productions[*iter][i];
                }else{
                    has_eps = true;
                }
            }
            if(has_eps){
                for(const auto& terminal : follow[*iter]){
                    LL1_table[*iter][terminal] = *iter + "->" + productions[*iter][i];
                }
            }
        }
        for(const auto& terminal : follow[*iter]){
            if(LL1_table[*iter].find(terminal) == LL1_table[*iter].end()){
                LL1_table[*iter][terminal] = "SYNCH";
            }
        }
    }
}

void print_symbol(const string& s){
    cout << s;
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

void parser(){
    init_tokens();
    stack<string> symbolstack;
    pair<deque<string>, deque<string>> left_seq;
    left_seq.first = {};
    left_seq.second.push_back(start_symbol);

    symbolstack.push("$");
    symbolstack.push(start_symbol);

    for(int i = 0 ; i < tokens.size() ;){
        cout << "current left sentencial form:\n\t\t\t\t";
        for(const auto& s: left_seq.first){
            print_symbol(s);
            cout << " ";
        }
        for(const auto& s: left_seq.second){
            print_symbol(s);
            cout << " ";
        }
        cout << endl;
        cout << "current token stream: \n\t\t\t\t";
        for(int j = i ; j < tokens.size() ; j++){
            print_symbol(tokens[j]);
            cout << " ";
        }//打印token 类型
        cout << endl;
        cout << "output:\n\t\t\t\t";


        if(V_T.find(symbolstack.top()) != V_T.end()){//栈顶是终结符
            if(symbolstack.top() == tokens[i]){
                ++i;
            }else{
                cout << "error : ";
                print_symbol(symbolstack.top());
                cout << "expected but get " << tokens[i] <<endl;
            }
            symbolstack.pop();

            if(left_seq.second.size()){
                left_seq.first.push_back(
                                left_seq.second.front()
                            );
                left_seq.second.pop_front();
            }
        }else if(V_N.find(symbolstack.top()) != V_N.end()){
            string action = LL1_table[symbolstack.top()][tokens[i]];
            if(action != ""){
                string nonterm = std::move(symbolstack.top());
                symbolstack.pop();
                left_seq.second.pop_front();
                if(action != "SYNCH"){
                    //将产生式压栈
                    cout << action;
                    int prod_index = action.find("->");
                    if(action.substr(prod_index+2) == "epsilon"){
                        //栈顶已经弹栈，所以不用进行操作
                    }
                    else{//将产生式倒着压入栈中
                        string tmp_prod = action.substr(prod_index+2);
                        for(int k = tmp_prod.length() - 1 ; k>=0 ; k--){
                            string tmp = tmp_prod.substr(k,1);
                            //如果是',说明还要取一个字符
                            if(tmp == "'"){
                                tmp = tmp_prod.substr(k-1,2);
                                k-=1;
                            }
                            if(tmp == "m"){
                                tmp = tmp_prod.substr(k-2,3);
                                k-=2;
                            }
                            if(tmp == "n"){
                                tmp = tmp_prod.substr(k-6,7);
                                k-=6;
                            }
                            symbolstack.push(tmp);
                            left_seq.second.push_front(tmp);
                        }
                    }
                }else{
                    cout << "error : " << nonterm;
                }
            }else{
                cout << "error : " ;
                print_symbol(tokens[i]);
                cout << " unexpected";
                ++i;
            }
        }else{
            if(symbolstack.top() == "$" && tokens[i] == "$"){
                symbolstack.pop();
                cout << "accept" << endl;
                return ;
            }
        }
        cout << endl;
    }
    cout << endl;
}

std::vector<std::string> split(const char *s, const char *delim)
{
    std::vector<std::string> result;
    if (s && strlen(s))
    {
        size_t len = strlen(s);
        char *src = new char[len + 1];
        strcpy(src, s);
        src[len] = '\0';
        char *tokenptr = strtok(src, delim);
        while (tokenptr != nullptr)
        {
            std::string tk = tokenptr;
            result.emplace_back(tk);
            tokenptr = strtok(nullptr, delim);
        }
        delete[] src;
    }
    return result;
}


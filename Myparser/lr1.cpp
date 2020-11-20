#include "lr1.h"

unordered_map<string, int> Reduce;

void extend_grammar(QJsonObject grammar){
    int total_prods = 0;
    string extend_start_symbol = start_symbol + "'";
    V_N.insert(extend_start_symbol);
    productions[extend_start_symbol].push_back(start_symbol);
    string first_production = extend_start_symbol + "->" + start_symbol;
    Reduce[first_production] = total_prods++;

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
                        string leftStr = rule["left"].toString().toStdString();
                        QJsonArray candidates = rule["candidate"].toArray();

                        productions[leftStr] = {};
                        for (int j = 0 ; j < candidates.size() ; j++ ) {
                            if(j>0){
                                cout<<" | ";
                            }
                            productions[leftStr].push_back(candidates[j].toString().toStdString());
                            string temp_production = leftStr + "->" + candidates[j].toString().toStdString();
                            Reduce[temp_production] = total_prods++;
                            cout<<candidates[j].toString().toStdString();
                        }
                        cout<<endl;
                    }
                }
            }
    }
}

void ItemSet::closure(const Item &start_item){
    unordered_set<Item, hash_item> new_items(items);
    //当不再有新的项的时候就退出
    //首先遍历当前所有的项目，检查
    for(; new_items.size() != 0; ){
        unordered_set<Item, hash_item> tmp;
        for(const Item& item:new_items){
            if(item.candidate.length() > item.dot){
                string next_V = item.candidate.substr(item.dot,1);
                if(next_V=="n"){
                    next_V = "num";
                }
                if(V_N.find(next_V) != V_N.end()){
                    string non_terminal = next_V;
                    vector<string> prod = productions[non_terminal];
                    for(auto i = 0 ; i < prod.size() ; i++){
                        Item new_item(non_terminal ,prod[i], 0);
                        if(items.find(new_item) == items.end()){
                            tmp.insert(new_item);
                            if(new_item.dot < new_item.candidate.length()){
                                transitableItems[non_terminal].insert(new_item);
                            }
                        }
                    }
                }else{//如果是终结符,至少也要加入到transitableItems里面
                    transitableItems[next_V].insert(item);
                }
            }
        }
        //当前项目是待约项目并且还没有加入new_items，就加入
        items.insert(
                    tmp.begin(),
                    tmp.end()
                    );
        new_items = tmp;
    }
}

DFA::DFA(){
    int cur(start);
    string start_production(productions[start_symbol+"'"][0]);
    Item start_item =  Item(start_symbol+"'",start_production,0);
    ItemSet start_itemSet(start_item); //构造函数直接调用closure函数，完善了start_itemSet
    itemsFamily.emplace_back(start_itemSet);//将I0加入项目集规范族

    //广度优先搜索
    queue<int> q;
    q.push(start);//从0开始
    while(!q.empty()){
        cur = q.front();
        q.pop();
        cout << "now state is : " << cur << endl;
        unordered_map<string, unordered_set<Item, hash_item>> record;
        for(const auto& items: itemsFamily[cur].transitableItems){//对于当前项目集，取出项目集中仍可以转移的项目
            string symbol(items.first);
            unordered_set<Item, hash_item> item_common_prefix(items.second);
            for(const auto& item: item_common_prefix){
                Item new_item(item);
                if(new_item.candidate.length() > new_item.dot){
                    string input_symbol = new_item.candidate.substr(item.dot, 1);
                    if(new_item.candidate[item.dot] == 'n'){
                        input_symbol = new_item.candidate.substr(item.dot, 3);
                        new_item.dot+=2;
                    }
                    new_item.dot++;
                    record[input_symbol].insert(new_item);//I_cur 经过符号 input_symbol 转换得到的项目集中 含有new_item
                }
            }    
        }
        //对每个项目集，遍历完所有的transi_item之后，得到的可以转移的，按顺序构造新的项目集
        for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
            if(record[*iter].size()){
                ItemSet new_itemSet(record[*iter]);
                //先查找有没有这个项目集
                auto pos(std::find(itemsFamily.begin(), itemsFamily.end(), new_itemSet));
                if(pos == itemsFamily.end()){
                    std::pair<string, string>mypair(*iter, std::to_string(itemsFamily.size()));
                    GOTO[cur].insert(mypair);
                    q.push(itemsFamily.size());
                    itemsFamily.push_back(new_itemSet);
                }else{
                    std::pair<string, string>mypair(*iter, std::to_string(pos - itemsFamily.begin()));
                    GOTO[cur].insert(mypair);
                }
            }

        }

        for(auto iter = V_T.begin() ; iter != V_T.end() ; iter++){
            if(record[*iter].size()){
                ItemSet new_itemSet(record[*iter]);
                auto pos(std::find(itemsFamily.begin(), itemsFamily.end(), new_itemSet));
                if(pos == itemsFamily.end()){
                    std::pair<string, string>mypair(*iter, "s"+std::to_string(itemsFamily.size()));
                    ACTIONS[cur].insert(mypair);
                    q.push(itemsFamily.size());
                    itemsFamily.push_back(new_itemSet);
                }else{
                    std::pair<string, string>mypair(*iter, "s" + std::to_string(pos - itemsFamily.begin()));
                    ACTIONS[cur].insert(mypair);
                }
            }
        }
    }

    cout << "The state num of the DFA is :" << itemsFamily.size() << endl;
    for(int i = 0 ; i < itemsFamily.size() ; i++){
        cout << "State I" << i << " : " << endl;
        for(auto iter = itemsFamily[i].items.begin() ; iter != itemsFamily[i].items.end() ; iter++){
            cout << iter->left << " -> " << iter->candidate.substr(0,iter->dot) << "." << iter->candidate.substr(iter->dot) << endl;
        }
        cout << endl;
    }
}

void caculate_lr_first(string left){
    string eps = "epsilon";
    vector<string> candidates = productions[left];
    int has_left_curr(-1);
    for(unsigned int i = 0 ; i < candidates.size() ; i++){
        bool all_has_epsilon(true);
        if(candidates[i].substr(0,1) == left){
            has_left_curr = i;
        }
        for (unsigned int j = 0 ; j < candidates[i].length() && has_left_curr != i; j++) {
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
                if(tmp != left)
                    caculate_lr_first(tmp);
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
        if(all_has_epsilon && has_left_curr != i){
            first_of_prod[candidates[i]].insert(eps);
        }
        for(auto iter = first_of_prod[candidates[i]].begin(); iter != first_of_prod[candidates[i]].end() ; iter++){
            first[left].insert(*iter);
        }
    }
    if(first[left].find(eps) != first[left].end() && has_left_curr != -1){//left可以产生空
        for(unsigned int i = 0 ; i < candidates[i].length(); i++){//可能不止一项候选式有左递归，所以还是要全部重新遍历一遍
            bool all_has_epsilon(true);
            string tmp = candidates[i].substr(0,1);
            int j = 1;
            if(tmp != left){
                break;
            }
            for( ;j<candidates[i].length() ; j++){
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
                    if(tmp != left)
                        caculate_lr_first(tmp);
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
            if(all_has_epsilon && has_left_curr != i){
                first_of_prod[candidates[i]].insert(eps);
            }
            for(auto iter = first_of_prod[candidates[i]].begin(); iter != first_of_prod[candidates[i]].end() ; iter++){
                first[left].insert(*iter);
            }
        }
    }
    first_constructed[left] = true;
}

void lr_init_first(){
    V_N.insert(non_terminals.begin(), non_terminals.end());
    V_T.insert(terminals.begin(), terminals.end());
    for(auto iter = V_N.begin(); iter != V_N.end(); iter++){
        first_constructed[*iter] = false;
    }
}
void lr_init_follow(){
    for(auto iter = V_N.begin(); iter!=V_N.end();iter++){
        follow_constructed[*iter] = false;
        for (auto ir = V_N.begin() ; ir != V_N.end() ; ir ++) {
            follow_contains[*iter][*ir] = false;
        }
    }
    return ;
}

void caculate_all_lr_first(){
    lr_init_first();
    for(auto iter = V_N.begin(); iter != V_N.end(); iter++){
        if(!first_constructed[*iter])
            caculate_lr_first(*iter);
    }
    for(auto iter = first.begin() ; iter != first.end() ; iter++){
        first_of_prod[iter->first].insert(
                    first[iter->first].begin(),
                    first[iter->first].end()
                    );
    }
    follow[start_symbol+"'"].insert("$");
    return ;
}

void caculate_lr_follow(string left){
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
                        caculate_lr_follow(iter->first);
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
                        caculate_lr_follow(iter->first);
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

void caculate_all_lr_follow(){
    lr_init_follow();
    for(auto iter = V_N.begin(); iter != V_N.end() ; iter++){
        if(!follow_constructed[*iter])
            caculate_lr_follow(*iter);
    }
    return ;
}

void generate_slr1_table(DFA dfa){
//    for(int i = 0 ; i < dfa.itemsFamily.size() ; i++){
//        unordered_set<Item, hash_item> temp = dfa.itemsFamily[i].items;
//        for(auto iter = temp.begin() ; iter != temp.end() ; iter++ ){
//            if(iter->candidate.length() == iter->dot){
//                set<string> alphas = follow[iter->left];
//                for(auto irr = alphas.begin() ; irr != alphas.end() ; irr++){
//                    string temp_production = iter->left + "->" + iter->candidate;
//                    dfa.ACTIONS[i].insert(make_pair(*irr, "R"+to_string(Reduce[temp_production])));
//                }
//            }
//        }
//    }

//    for(auto iter = V_T.begin() ; iter != V_T.end() ; iter++){
//        cout << "\t" << *iter;
//    }
//    cout << "\t" << "$";
//    for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
//        cout << "\t" << *iter;
//    }
//    cout << endl;
//    for(int i = 0 ; i < dfa.itemsFamily.size() ; i++){
//        cout << i;
//        for(auto iter = V_T.begin() ; iter != V_T.end() ; iter++){
//            cout << "\t" << dfa.ACTIONS[i][*iter] ;
//        }
//        if(dfa.ACTIONS[i]["$"] == "R0")
//            dfa.ACTIONS[i]["$"] = "ACC";
//        cout << "\t" << dfa.ACTIONS[i]["$"];
//        for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
//            cout << "\t" << dfa.GOTO[i][*iter] ;
//        }
//        cout << endl;
//    }
}

void lr1_parser(DFA dfa){
    init_tokens();
    deque<pair<int, string>> lrsymbolstack;
    lrsymbolstack.push_back({0, "$"});

    for(int i = 0 ; i < dfa.itemsFamily.size() ; i++){
        unordered_set<Item, hash_item> temp = dfa.itemsFamily[i].items;
        for(auto iter = temp.begin() ; iter != temp.end() ; iter++ ){
            if(iter->candidate.length() == iter->dot){
                set<string> alphas = follow[iter->left];
                for(auto irr = alphas.begin() ; irr != alphas.end() ; irr++){
                    string temp_production = iter->left + "->" + iter->candidate;
                    dfa.ACTIONS[i].insert(make_pair(*irr, "R"+to_string(Reduce[temp_production])));
                }
            }
        }
    }
    for(auto iter = V_T.begin() ; iter != V_T.end() ; iter++){
        cout << "\t" << *iter;
    }
    cout << "\t" << "$";
    for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
        cout << "\t" << *iter;
    }
    cout << endl;
    for(int i = 0 ; i < dfa.itemsFamily.size() ; i++){
        cout << i;
        for(auto iter = V_T.begin() ; iter != V_T.end() ; iter++){
            cout << "\t" << dfa.ACTIONS[i][*iter] ;
        }
        if(dfa.ACTIONS[i]["$"] == "R0")
            dfa.ACTIONS[i]["$"] = "ACC";
        cout << "\t" << dfa.ACTIONS[i]["$"];
        for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
            cout << "\t" << dfa.GOTO[i][*iter] ;
        }
        cout << endl;
    }

    for(int i = 0 ; i < tokens.size() ; ){
        cout << endl;
        cout << "current parsing stack:\n\t\t\t\t";
        for(int j = 0 ; j < lrsymbolstack.size() ; j++){
            cout << "[" << lrsymbolstack[j].first << ", " << lrsymbolstack[j].second << "]";
        }
        cout << endl;
        string cur_symbol(tokens[i]);
        cout << "current token stream:\n\t\t\t\t";
        for(int j = i ; j < tokens.size() ; j++){
            cout << tokens[j] << " ";
        }
        cout << endl;
        cout << "output:\n\t\t\t\t";
        int x = lrsymbolstack.back().first;
        auto res = dfa.ACTIONS[x][cur_symbol];

        if(res != ""){
            string action = res;
            cout << action;
            if(action[0] == 's'){
                lrsymbolstack.push_back({stoi(action.substr(1)), cur_symbol});
                ++i;
            }
            else if(action[0] == 'R'){
                string tmp_prod;
                for(auto iter = Reduce.begin() ; iter != Reduce.end() ; iter++){
                    int sqn = iter->second;
                    if(sqn == stoi(action.substr(1))){
                        tmp_prod = iter->first;
                    }
                }
                int pos = tmp_prod.find("->");
                string non_terminal = tmp_prod.substr(0, pos);
                string candidate = tmp_prod.substr(pos+2);
                //for(int j = candidate.length()-1 ; j-- ; lrsymbolstack.pop_back());
                int k = 0;
                while(k < candidate.length()){
                    if(candidate[k] = 'n'){
                        k+=3;
                    }else{
                        k+=1;
                    }
                    lrsymbolstack.pop_back();
                }
                string nxt = dfa.GOTO[lrsymbolstack.back().first][non_terminal];
                if(nxt == "")
                    nxt = "2";
                lrsymbolstack.push_back({stoi(nxt), non_terminal});
            }
            else if(action[0] == 'A'){//ACC
                return ;
            }
        }else{
            cout << "error \n ";
            return ;
        }
    }
    return ;
}

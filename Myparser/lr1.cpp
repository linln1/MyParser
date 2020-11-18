#include "lr1.h"

void extend_grammar(QJsonObject grammar){
    string extend_start_symbol = start_symbol + "'";
    productions[extend_start_symbol].push_back(start_symbol);

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
                            productions[leftStr].push_back(candidates[i].toString().toStdString());
                            cout<<candidates[j].toString().toStdString();
                        }
                        cout<<endl;
                    }
                }
            }
    }
}

void ItemSet::closure(const Item &start_item){
    set<Item> new_items(items);
    //当不再有新的项的时候就退出
    //首先遍历当前所有的项目，检查
    for(; new_items.size(); ){
        set<Item> tmp;
        for(const Item& item:new_items){
            if(item.candidate.length() > item.dot && V_N.find(to_string(item.candidate[item.dot])) != V_N.end()){
                string non_terminal = std::to_string(item.candidate[item.dot]);
                vector<string> prod = productions[non_terminal];
                for(auto i = 0 ; i < prod.size() ; i++){
                    Item new_item(non_terminal ,prod[i], 0);
                    if(new_items.find(new_item) == new_items.end()){
                        tmp.insert(std::move(new_item));
                    }
                }

            }
        }
        //当前项目是待约项目并且还没有加入new_items，就加入
        items.insert(
                    tmp.begin(),
                    tmp.end()
                    );
        new_items = std::move(tmp);
    }
}

DFA::DFA(){
    int cur(start);
    string start_production(productions[start_symbol+"'"][0]);
    Item start_item =  Item{
                start_symbol+"'",
                start_production,
                0
        };
    ItemSet start_itemSet(start_item); //构造函数直接调用closure函数，完善了start_itemSet
    itemsFamily.emplace_back(std::move(start_itemSet));//将I0加入项目集规范族

    //广度优先搜索
    queue<int> q;
    q.push(start);//从0开始
    while(!q.empty()){
        cur = q.front();
        q.pop();
        for(const auto& items: itemsFamily[cur].transitableItems){//遍历项目集规范族，取出项目集中仍可以转移的项目
            string symbol(items.first);
            set<Item> item_common_prefix(items.second);
            set<Item> next_items;
            for(const auto& item: item_common_prefix){
                Item new_item(item);
                if(new_item.candidate.length() > new_item.dot)
                    new_item.dot++;
                next_items.insert(std::move(new_item));
            }
            ItemSet next_itemSet(next_items);
            auto exists = find(itemsFamily.begin(), itemsFamily.end(), next_itemSet);
            if(exists == itemsFamily.end()){
                if(V_T.find(symbol) != V_T.end()){
                    ACTIONS[make_pair(cur, symbol)] = "s" + std::to_string(itemsFamily.size());
                }
                else{
                    GOTO[make_pair(cur, symbol)] = std::to_string(itemsFamily.size());
                }
                q.push(itemsFamily.size());
                itemsFamily.emplace_back(std::move(next_itemSet));
            } else{
                if(V_T.find(symbol) != V_T.end()){
                    ACTIONS[make_pair(cur, symbol)] = "s" + std::to_string(exists - itemsFamily.begin());
                }
                else{
                    GOTO[make_pair(cur, symbol)] = std::to_string(exists - itemsFamily.begin());
                }
            }
        }
    }
}

void init(){
    V_N.insert(non_terminals.begin(), non_terminals.end());
    V_T.insert(terminals.begin(), terminals.end());
    for(auto iter = V_N.begin(); iter != V_N.end(); iter++){
        first_constructed[*iter] = false;
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

void caculate_all_lr_first(){
    init();
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

void init_follow(){
    for(auto iter = V_N.begin(); iter!=V_N.end();iter++){
        follow_constructed[*iter] = false;
        for (auto ir = V_N.begin() ; ir != V_N.end() ; ir ++) {
            follow_contains[*iter][*ir] = false;
        }
    }
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

void caculate_all_lr_follow(){
    init_follow();
    for(auto iter = V_N.begin(); iter != V_N.end() ; iter++){
        if(!follow_constructed[*iter])
            caculate_lr_follow(*iter);
    }
    return ;
}

void generate_slr1_table(){

}

void lr1_parser(){

}

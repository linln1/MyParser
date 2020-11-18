#ifndef LR1_H
#define LR1_H

#include "defines.h"

class Item{
    public:
        string left;
        string candidate;
        int dot;
        Item(string left, string candidate, int dot){
            this->left = left;
            this->candidate = candidate;
            this->dot = dot;
        }
};

class ItemSet{//项目集
    public:
        set<Item> items;
        set<Item> reducedItems; //归约项目集
        set<Item> shiftInItems; //移进项目集
        set<Item> waitedItems; //待约项目集
        unordered_map<string, set<Item>> transitableItems;

        ItemSet(const set<Item>& items = {}){
            for(auto item : items){
                if(item.dot < item.candidate.length()){
                    Item new_item(item.left, item.candidate, item.dot);
                    transitableItems[item.left].insert(new_item);
                }
                closure(item);
            }
        }
        ItemSet(const Item& start_item){
            if(start_item.dot < start_item.candidate.length()){
                Item new_item(start_item.left, start_item.candidate, start_item.dot);
                transitableItems[start_item.left].insert(new_item);
            }
            closure(start_item);
        }
        void closure(const Item& start_item); //求闭包
};

//DFA就是一张表格，其中对应ACTION 和 GOTO
class DFA{
    public :
        vector<ItemSet> itemsFamily;
        unordered_map<pair<int, string>, string> ACTIONS;
        unordered_map<pair<int, string>, string> GOTO;

        DFA();

        int start = 0;
        int state;
        int statenum;
};

void generate_slr1_table();
void extend_grammar(QJsonObject grammar);
void caculate_lr_first();
void caculate_lr_follow();
void lr1_parser();

#endif // LR1_H

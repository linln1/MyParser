#ifndef LR1_H
#define LR1_H

#include "defines.h"
#include "lexer.h"

class Item{
    public:
        string left;
        string candidate;
        int dot;
        Item(){}
        Item(const Item& A){
            this->left = A.left;
            this->candidate = A.candidate;
            this->dot = A.dot;
        }
        Item(string left, string candidate, int dot){
            this->left = left;
            this->candidate = candidate;
            this->dot = dot;
        }
        bool operator == (const Item& A) const{
            if(this->left == A.left && (this->candidate == A.candidate) && (this->dot == A.dot)){
                return true;
            }
            return false;
        }
};

struct hash_item{
    size_t operator()(const Item& i) const{
        return hash<string>()(i.left) ^ hash<int>()(i.dot) ^ hash<string>()(i.candidate);
    }
};

class ItemSet{//项目集
    public:
        unordered_set<Item, hash_item> items;
        unordered_set<Item, hash_item> reducedItems; //归约项目集
        unordered_set<Item, hash_item> shiftInItems; //移进项目集
        unordered_set<Item, hash_item> waitedItems; //待约项目集
        unordered_map<string, unordered_set<Item, hash_item>> transitableItems;

        ItemSet(unordered_set<Item, hash_item> items){
            for(auto item = items.begin() ; item != items.end() ; item++){
                this->items.insert(*item);
                if(item->dot < item->candidate.length()){
                    //transitableItems 经过什么符号可以转移为什么项目集
                    string input_symbol = item->candidate.substr(item->dot,1);
                    if(item->candidate[item->dot] == 'n'){
                        input_symbol = item->candidate.substr(item->dot,3);
                    }
                    transitableItems[input_symbol].insert(*item);
                    closure(*item);
                }
            }
        }
        ItemSet(Item start_item){
            items.insert(start_item);
            if(start_item.dot < start_item.candidate.length()){
                string input_symbol = start_item.candidate.substr(start_item.dot, 1);
                if(input_symbol == "n"){
                    input_symbol = start_item.candidate.substr(start_item.dot, 3);
                }
                transitableItems[input_symbol].insert(start_item);
            }
            closure(start_item);
        }
        void closure(const Item& start_item); //求闭包

        bool operator ==(const ItemSet& A) const{
            return items == A.items && transitableItems == A.transitableItems && reducedItems == A.reducedItems && shiftInItems == A.shiftInItems && waitedItems == A.waitedItems;
        }
};

//DFA就是一张表格，其中对应ACTION 和 GOTO
class DFA{
    public :
        vector<ItemSet> itemsFamily;
        unordered_map<int, map<string, string>> ACTIONS;//
        unordered_map<int, map<string, string>> GOTO;

        DFA();

        int start = 0;
        int state;
        int statenum;
};

extern unordered_map<string, int> Reduce;
extern void init_tokens();
extern void lr_init_first();
extern void lr_init_follow();
extern void generate_slr1_table(DFA dfa);
extern void extend_grammar(QJsonObject grammar);
extern void caculate_all_lr_first();
extern void caculate_all_lr_follow();
extern void caculate_lr_first(string left);
extern void caculate_lr_follow(string left);
extern void lr1_parser(DFA dfa);

#endif // LR1_H

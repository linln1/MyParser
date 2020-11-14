#ifndef TIRE_H
#define TIRE_H

#include "ll1.h"

class Tire{
    public:
        struct TrNode{
            char ch;
            int count = 0;
            bool isEnd = false;
            TrNode(){
                count = 0;
                ch = '#';
            }
            TrNode(char ch, bool isEnd){
                this->ch = ch;
                this->isEnd = isEnd;
                this->count++;
            }
            unordered_map<char, TrNode*> childs;
        };

        Tire(){
            root = new TrNode();
        }

        TrNode* getRoot(){
            return root;
        }

        void insert(string word){
            TrNode* temp = root;
            for(int i = 0; i < word.length(); i++ ){
                char mych;
                mych = word[i];
                if(temp->childs.find(word.at(i)) == temp->childs.end()){
                    temp->childs[mych] = new TrNode(mych, false);
                }
                temp->childs[mych]->count++;//无论是新建的还是旧的，这里都会加一
                temp = temp->childs[mych];
            }
            temp->isEnd = true;
        }

        TrNode* searchPrefix(string word){
            TrNode* tmp = root;
            for(char ch: word){
                if(tmp->childs.find(ch) == tmp->childs.end()){
                    return nullptr;
                }else{
                    tmp = tmp->childs[ch];
                }
            }
            return tmp;
        }

        bool search(string word){
            TrNode* prefix = searchPrefix(word);
            if(prefix == nullptr || !prefix->isEnd)
                return false;
            else {
                return true;
            }
        }

        bool startwith(string prefix){
            TrNode* pre = searchPrefix(prefix);
            if(pre == nullptr){
                return false;
            }else {
                return true;
            }
        }
        // abc123 | abt986 | cd5777 | cd699 | e
        // return {"ab", "cd", ""}
        // 也就是字典树节点计数不等于一就可以
        void searchLeftCommonFactor(TrNode* rt, vector<string> res_set, string res){
            TrNode* tmp = rt;
            bool flag = false;
            if(tmp->isEnd || (tmp->childs.size() == 1 && tmp->childs.begin()->second->count == 1)){
                res_set.push_back(res);
                return;
            }
            for (auto it = tmp->childs.begin(); it != tmp->childs.end(); it++) {
                if(it->second->count!=1){
                    flag = true;
                }
            }
            if(flag == false){
                res_set.push_back(res);
                return;
            }

            for(auto it = tmp->childs.begin(); it != tmp->childs.end(); it++){
                if(it->second->count != 1){
                    res += it->first;
                    searchLeftCommonFactor(it->second, res_set, res);
                }
            }
        }

    private:
        TrNode* root;
};

#endif // TIRE_H

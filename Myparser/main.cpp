#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QSysInfo>
#include <QDir>
#include <QDebug>
#include <QMessageBox>

#include "ll1.h"
#include "lr1.h"
#include "lexer.cpp"

int main(int argc, char *argv[])
{
    lexerinit(argc, argv);

    if(QSysInfo::productType() == "windows" ||
            QSysInfo::productType() == "winrt"){
        system("chcp 65001");
    }

    QFile file(QDir::homePath() + "/Desktop/grammar.json");
    if(!file.open(QIODevice::ReadWrite)){
        cout<<"failure to open file!\n";
        exit(1);
    }

    QJsonParseError jpe;
    QJsonDocument grammarJson = QJsonDocument::fromJson(file.readAll(), &jpe);

    if(!grammarJson.isNull() &&
            jpe.error == QJsonParseError::NoError){
        cout<<"read file successfully!\n";
        if(grammarJson.isObject()){
            QJsonObject grammar = grammarJson.object();
            if(grammar.contains("Type") &&
                    grammar.value("Type") == "grammar"){
                cout<<"Type of file is "<<grammar.value("Type").toString().toStdString()<<endl;
            }

            if(grammar.contains("non_terminals") &&
                    grammar.value("non_terminals").isString()){
                    string VN = grammar["non_terminals"].toString().toStdString();
                    const char* VN_str = VN.c_str();
                    vector<string> VNs = split(VN_str, " ");
                    foreach(string s, VNs){
                        non_terminals.insert(s);
                    }
            }

            if(grammar.contains("terminals") &&
                    grammar.value("terminals").isString()){
                    string VT = grammar["terminals"].toString().toStdString();
                    const char* VT_str = VT.c_str();
                    vector<string> VTs = split(VT_str, " ");
                    foreach(string s, VTs){
                        terminals.insert(s);
                    }
            }

            if(grammar.contains("start_symbol") &&
                    grammar.value("start_symbol").isString()){
                    start_symbol = grammar["start_symbol"].toString().toStdString();
            }
            if(strcmp(argv[3],"ll1")==0){
                eliminate_left_recurrent(grammar);
                extract_left_common_factor();
                cout<< endl << endl<<"extract the left common factor"<< endl ;
                int cnt = 0;
                for(auto it = productions.begin(); it != productions.end(); it++){
                    cout << "[" << cnt <<"th rule: ]";
                    string val = it->first;
                    cout << val << " -> ";
                    vector<string> regCandidates = it->second;//已经消除左递归的候选式
                    for (int i = 0 ; i < regCandidates.size() ; i++ ) {
                        if(i > 0){
                            cout<<" | ";
                        }
                        cout << regCandidates[i];
                    }
                    cout << endl;
                    cnt++;
                }

                //calculate First set and Follow set

                calculate_all_first();
                cout << "ALL the NON_Terminals are as follows :" << endl;
                for(auto it = V_N.begin() ; it != V_N.end(); it++){
                    cout << (*it) << endl;
                }

                cout << "ALL the Terminals are as follows :" << endl;
                for(auto it = V_T.begin(); it != V_T.end() ; it++){
                    cout << (*it) << endl;
                }

                cout << "The start Symbol of this grammar is :" << start_symbol << endl;

                cout << "The First Set of all the NON_Terminals are as follows: " << endl;
                for(auto it = first.begin() ; it != first.end() ; it++){
                    cout << it->first << " : ";
                    set<string> candidates = it->second;
                    for(auto ir = candidates.begin(); ir != candidates.end() ; ir++){
                        cout << (*ir) << " ";
                    }
                    cout << endl;
                }

            //    cout << "The first set of prod are as follows: " << endl;
            //    for(auto it = first_of_prod.begin(); it != first_of_prod.end() ; it++){
            //        cout << it->first << " : ";
            //        set<string> candidates = it->second;
            //        for(auto ir = candidates.begin(); ir != candidates.end() ; ir++){
            //            cout << (*ir) << " ";
            //        }
            //        cout << endl;
            //    }


                calculate_all_follow();
                cout << "The Follow Set of all the NON_Terminals are as follows: " << endl;
                for(auto it = follow.begin() ; it != follow.end() ; it++){
                    cout << it->first << " : ";
                    set<string> candidates = it->second;
                    for(auto ir = candidates.begin(); ir != candidates.end() ; ir++){
                        cout << (*ir) << " ";
                    }
                    cout << endl;
                }

                generate_LL1_table();
                //unordered_map<string, unordered_map<string, string>> LL1_table;
                cout << "The LL1 Table is as follows: " << endl;
                cout << " " << "\t";
                for(auto irr = V_T.begin() ; irr != V_T.end() ; irr++){
                    cout << *irr << "\t" << "       ";
                }
                cout << "$" << endl;
                for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
                    LL1_table[*iter]["$"] = {};
                    if(follow[*iter].find("$") != follow[*iter].end()){
                        if(first[*iter].find("epsilon") == first[*iter].end()){
                            LL1_table[*iter]["$"] = "SYNCH";
                        }
                        else{
                            LL1_table[*iter]["$"] = *iter + "->epsilon";
                        }
                    }
                }

                for(auto iter = V_N.begin() ; iter != V_N.end() ; iter++){
                    cout << *iter << "\t";
                    for(auto irr = V_T.begin() ; irr != V_T.end() ; irr++){
                        cout << LL1_table[*iter][*irr] << "\t";
                        int len = LL1_table[*iter][*irr].length();
                        if(len == 0){
                            len = 5;
                        }
                        for(int i = 0 ; i < 10 - len ; i++){
                            cout << " ";
                        }
                    }
                    cout << LL1_table[*iter]["$"] << endl;
                }
                ll1_parser();
            }
            else if(strcmp(argv[3],"lr1")==0){
                extend_grammar(grammar);
                //拓展完文法，还要重新计算first和follow集
                caculate_all_lr_first();
                cout << "ALL the NON_Terminals are as follows :" << endl;
                for(auto it = V_N.begin() ; it != V_N.end(); it++){
                    cout << (*it) << endl;
                }

                cout << "ALL the Terminals are as follows :" << endl;
                for(auto it = V_T.begin(); it != V_T.end() ; it++){
                    cout << (*it) << endl;
                }

                cout << "The start Symbol of this grammar is :" << start_symbol << endl;

                cout << "The First Set of all the NON_Terminals are as follows: " << endl;
                for(auto it = first.begin() ; it != first.end() ; it++){
                    cout << it->first << " : ";
                    set<string> candidates = it->second;
                    for(auto ir = candidates.begin(); ir != candidates.end() ; ir++){
                        cout << (*ir) << " ";
                    }
                    cout << endl;
                }

                caculate_all_lr_follow();

                cout << "The Follow Set of all the NON_Terminals are as follows: " << endl;
                for(auto it = follow.begin() ; it != follow.end() ; it++){
                    cout << it->first << " : ";
                    set<string> candidates = it->second;
                    for(auto ir = candidates.begin(); ir != candidates.end() ; ir++){
                        cout << (*ir) << " ";
                    }
                    cout << endl;
                }


                DFA dfa;  //构造LR(0)项目集规范族，然后构造自动机
                generate_slr1_table(dfa);  //有冲突的时候才需要follow集，用来解决冲突，否则不需要
                lr1_parser(dfa);
            }

        }
    }


    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return 0; //a.exec();
}

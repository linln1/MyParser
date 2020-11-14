#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QSysInfo>
#include <QDir>
#include <QDebug>
#include <QMessageBox>

#include "ll1.h"
#include "lexer.cpp"

int main(int argc, char *argv[])
{
    //lexerinit(argc, argv);
//    QFile files("test1.c");
//    if(!files.open(QFile::ReadOnly|QFile::Text)){
//        cout<<files.errorString().toStdString()<<endl;
//

    if(QSysInfo::productType() == "windows" ||
            QSysInfo::productType() == "winrt"){
        system("chcp 65001");
    }

    QFile file(QDir::homePath() + "/Desktop/test.json");
    if(!file.open(QIODevice::ReadWrite)){
        cout<<"failure to open file!\n";
        exit(1);
    }

    QJsonParseError jpe;
    QJsonDocument grammerJson = QJsonDocument::fromJson(file.readAll(), &jpe);

    if(!grammerJson.isNull() &&
            jpe.error == QJsonParseError::NoError){
        cout<<"read file successfully!\n";
        if(grammerJson.isObject()){
            QJsonObject grammer = grammerJson.object();
            if(grammer.contains("Type") &&
                    grammer.value("Type") == "Grammer"){
                cout<<"Type of file is "<<grammer.value("Type").toString().toStdString()<<endl;
            }

            if(grammer.contains("non_terminals") &&
                    grammer.value("non_terminals").isString()){
                    string VN = grammer["non_terminals"].toString().toStdString();
                    const char* VN_str = VN.c_str();
                    vector<string> VNs = split(VN_str, " ");
                    foreach(string s, VNs){
                        non_terminals.insert(s);
                    }
            }

            if(grammer.contains("terminals") &&
                    grammer.value("terminals").isString()){
                    string VT = grammer["terminals"].toString().toStdString();
                    const char* VT_str = VT.c_str();
                    vector<string> VTs = split(VT_str, " ");
                    foreach(string s, VTs){
                        terminals.insert(s);
                    }
            }

            if(grammer.contains("start_symbol") &&
                    grammer.value("start_symbol").isString()){
                    start_symbol = grammer["start_symbol"].toString().toStdString();
            }

            if(grammer.contains("production") &&
                    grammer.value("production").isArray()){
                    QJsonArray Ps = grammer["production"].toArray();
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
                                string leftExtendStr = leftStr+"'";

                                productions[leftStr] = {};
                                for (int j = 0 ; j < candidates.size() ; j++ ) {
                                    if(j>0){
                                        cout<<" | ";
                                    }
                                    cout<<candidates[j].toString().toStdString();
                                    if(candidates[j].toString().toStdString().find(leftStr) != 0){//没有左递归的表达式，利用βA' 放入容器当中
                                        if(non_terminals.find(leftExtendStr) != non_terminals.end()){
                                            string betaExtend = candidates[j].toString().toStdString() + leftExtendStr;
                                            productions[leftStr].push_back(betaExtend);
                                        }
                                        else{
                                            string tmp = candidates[j].toString().toStdString();
                                            productions[leftStr].push_back(tmp);
                                        }
                                    }
                                    else{//该候选式含有左递归
                                        if(non_terminals.find(leftExtendStr) == non_terminals.end()){
                                            non_terminals.insert(leftExtendStr);
                                        }
                                        if(non_terminals.find(leftExtendStr) != non_terminals.end()){//已经拓展了符号
                                            string alpha = candidates[j].toString().toStdString().substr(1);
                                            alpha+=leftExtendStr;
                                            productions[leftExtendStr].push_back(alpha);
                                        }
                                    }
                                }
                                if(non_terminals.find(leftExtendStr) != non_terminals.end()){
                                    productions[leftExtendStr].push_back("epsilon");
                                }
                                cout<<endl;
                            }
                        }
                    }
                    cout<< endl << endl<<"eliminate the left recursion"<< endl ;
                    int total = 0;
                    for(auto it = productions.begin(); it != productions.end(); it++){
                        cout << "[" << total <<"th rule: ]";
                        string val = it->first;
                        cout << val<< " -> ";
                        vector<string> regCandidates = it->second;//已经消除左递归的候选式
                        for (int i = 0 ; i < regCandidates.size() ; i++ ) {
                            if(i > 0){
                                cout<<" | ";
                            }
                            cout << regCandidates[i];
                        }
                        cout << endl;
                        total++;
                    }
            }
        }
    }
    //extract_left_common_factor
    //    cout<< endl << endl<<"extract the left common factor"<< endl ;
    //    extract_left_common_factor();
    //    int cnt = 0;
    //    for(auto it = productions.begin(); it != productions.end(); it++){
    //        cout << "[" << cnt <<"th rule: ]";
    //        string val = it->first;
    //        cout << val.leftStr.toStdString() << " -> ";
    //        vector<string> regCandidates = it->second;//已经消除左递归的候选式
    //        for (int i = 0 ; i < regCandidates.size() ; i++ ) {
    //            if(i > 0){
    //                cout<<" | ";
    //            }
    //            cout << regCandidates[i].toStdString();
    //        }
    //        cout << endl;
    //        cnt++;
    //    }
    //    cout << cnt << endl;

    //calculate First set and Follow set

    calculate_all_first();
//    cout << "ALL the NON_Terminals are as follows :" << endl;
//    for(auto it = V_N.begin() ; it != V_N.end(); it++){
//        cout << (*it) << endl;
//    }

//    cout << "ALL the Terminals are as follows :" << endl;
//    for(auto it = V_T.begin(); it != V_T.end() ; it++){
//        cout << (*it) << endl;
//    }

    cout << "The start Symbol of this grammer is :" << start_symbol << endl;

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

    QFile src(QDir::homePath() + "/Desktop/src.json");
    if(!src.open(QIODevice::ReadWrite)){
        cout<<"failure to open file!\n";
        exit(1);
    }
    QJsonParseError srcjpe;
    QJsonDocument srcJson = QJsonDocument::fromJson(src.readAll(), &srcjpe);

    if(!srcJson.isNull() &&
            srcjpe.error == QJsonParseError::NoError){
        cout<<"read file successfully!\n";
        if(srcJson.isObject()){
            QJsonObject srcj = srcJson.object();
            if(srcj.contains("tokens") && srcj.value("tokens").isArray()){
                QJsonArray Tks = srcj["tokens"].toArray();
                for (int i = 0 ; i < Tks.size() ; i++) {
                    tokens.push_back(Tks[i].toString().toStdString());
                    cout<<"Token is "<<Tks[i].toString().toStdString()<<endl;
                }
            }
        }
    }

    parser();
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    return 0; //a.exec();
}

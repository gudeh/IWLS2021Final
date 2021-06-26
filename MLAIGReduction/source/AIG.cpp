/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "AIG.h"

string wordSelector(string line, int word_index){
    int i=1;
    line.erase(0,line.find_first_not_of(" "));
    while(i<word_index)
    {
        line.erase(0,line.find_first_not_of(' '));
        line.erase(0,line.find_first_of(' ')+1);
        line.erase(0,line.find_first_not_of(' '));
        i++;
    }
    if(line.find(' ')!=string::npos)
        line.erase(line.find_first_of(' '),line.size());
//    cout<<line<<endl;
    
    return line;
}



bool getThisPtrPolarity(nodeAig* param){
    nodeAig* aux;
    
    aux=param->fixLSB();
//    cout<<aux->getId()<<"->"<<((int)((uintptr_t)param) & 01)<<endl;
    //returns TRUE for inverted and FALSE for regular
    return ((int)((uintptr_t)param) & 01);
}


int binToDec(vector<int> param){
    int N=param.size();
//    cout<<"N:"<<N<<endl;
    int result=0,aux=0;
    result=-param[N-1]*pow(2,N-1);
    
    for(int i=0;i<=N-2;i++)
        aux+=param[i]*pow(2,i);
//    cout<<"res:"<<result<<endl;
//    cout<<"aux:"<<aux<<endl;
    result=result+aux;
    
//    int soma=0,indice=0;
////    for(int e=param.size()-1;e>0;e--)
//     for(int e=0;e<param.size()-1;e++)
//           {            
//               soma+=pow(2,indice)*(param[e]);
//               indice++;
////               cout<<indice<<"*"<<param[e]<<"+";
//           }
////    if(param[0]==1)
//    if(param[param.size()-1]==1 && soma>0)
//        soma*=-1;
//    else if (soma==0 && param[param.size()-1]==1)
//        soma=1;
//        
//    if(soma==32767)
//        soma=-1;
    return result;
}
string abc_log_name="abc_stuff/logAbc.txt";
void abcCall21(string aig_source, string aig_name,string cifar_file){
//#if DEBUG >= 1
//    cout<<"Calling ABC, read on:"<<aig_name<<",with &mltest on file:"<<cifar_file<<endl;;
//#endif
//    ofstream script("script.scr"),log(abc_log_name); log.close();;
//    script<<"&r "<<(source+aig_name)<<".aig"<<endl<<"&ps"<<endl<<"&iwls21test "<<cifar_file<<endl<<"quit";
//    script.close();
    string command;
    ofstream log(abc_log_name); log.close();
    command="-c '&r "+(aig_source+aig_name)+".aig' -c '&ps' -c '&iwls21test "+cifar_file+"' -c 'quit'";
    command=("./abc "+command+" >> "+abc_log_name);
//    cout<<"Calling ABC, command:"<<command<<endl
    system(command.c_str());
//    system((my_root+"abc.exe -c 'source script.scr' >> "+abc_log_name).c_str());
//    system((my_root+"abc -c 'source script.scr' >> "+abc_log_name).c_str());
}

void abcWrite(string old_name,string new_name){
#if DEBUG >=1
    cout<<endl<<"ABC WRITE: ("<<old_name<<") -> ("<<new_name<<")"<<endl;
#endif
//    ofstream script("abc_stuff/script.scr");
    string command;
    ofstream log(abc_log_name); log.close();
    command="-c '&r "+old_name+".aig' -c '&ps' -c '&w "+new_name+"' -c 'quit'";
    command=("./abc "+command+" >> "+"log.txt");
//    cout<<"Calling ABC, command:"<<command<<endl;
    system(command.c_str());
//    script<<"&r "<<old_name<<".aig"<<endl<<"&ps"<<endl<<"&w "<<new_name<<endl<<"quit";
//    script.close();
//    system(("./abc -c 'source abc_stuff/script.scr' >> "+abc_log_name).c_str());
}

void abcCeC(string new_name,string abc_name,float min_th,int option){
    cout<<endl<<"ABC CEC: ("<<new_name<<") VS ("<<abc_name<<")"<<endl;
    ofstream script("abc_stuff/script.scr"),log(abc_log_name,ios::app);
    script<<"&cec "<<new_name<<".aig "<<abc_name<<endl<<"quit";
    script.close();
    log<<"CEC on circuits: "<<new_name<<" VS: "<<abc_name<<endl<<"TH:"<<min_th<<", OPTION:"<<option<<endl;
    log.close();
    system(("./abc -c 'source abc_stuff/script.scr' >>"+abc_log_name).c_str());
}
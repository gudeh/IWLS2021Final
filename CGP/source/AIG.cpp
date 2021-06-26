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
//&ps -n, should be no color
string abc_log_name="logAbc.txt";

void abcCall21(string aig_source, string aig_name,string cifar_file){
//#if DEBUG >= 1
//    cout<<"Calling ABC, read on:"<<aig_name<<",with &mltest on file:"<<cifar_file<<endl;;
//#endif
    string my_root=root,command;
//    ofstream script("script.scr"),log(abc_log_name); log.close();;
//    script<<"&r "<<(source+aig_name)<<".aig"<<endl<<"&ps"<<endl<<"&iwls21test "<<cifar_file<<endl<<"quit";
//    script.close();
    
    ofstream log(abc_log_name); log.close();
    command="-c '&r "+(aig_source+aig_name)+".aig' -c '&ps' -c '&iwls21test "+cifar_file+"' -c 'quit'";
    command=(my_root+"abc "+command+" >> "+abc_log_name);
    cout<<"Calling ABC, command:"<<command<<endl;
    system(command.c_str());
//    system((my_root+"abc.exe -c 'source script.scr' >> "+abc_log_name).c_str());
//    system((my_root+"abc -c 'source script.scr' >> "+abc_log_name).c_str());
}

void abcCallML(string aig_name,string PLA_file,string source){
#if DEBUG >= 1
    cout<<"Calling ABC, read on:"<<aig_name<<",with &mltest on file:"<<PLA_file<<endl;
#endif
    ofstream script("script.scr"),log(abc_log_name); log.close();
    script<<"&r "<<(source+aig_name)<<".aig"<<endl<<"&ps"<<endl<<"&mltest "<<PLA_file<<endl<<"quit";
    script.close();
    system(("./abc -c 'source script.scr' >> "+abc_log_name).c_str());

}

void abcWrite(string aig_name,string out_name){
    cout<<endl<<"ABC WRITE: ("<<aig_name<<") -> ("<<out_name<<")"<<endl;
    ofstream script("script.scr"),refresh(abc_log_name); refresh.close();
    
    script<<"&r "<<aig_name<<".aig"<<endl<<"&ps"<<endl<<"&w "<<out_name<<endl<<"quit";
    script.close();
    system(("./abc -c 'source script.scr' >>" +abc_log_name).c_str() );
}

void abcGenerateAIGfromPLA(string pla_name,string out_name){
    cout<<endl<<"ABC WRITE: ("<<pla_name<<") -> ("<<out_name<<")"<<endl;
    ofstream script("script.scr");
    script<<"read_pla "<<pla_name<<endl<<"strash"<<endl<<"write "<<out_name<<endl<<"quit";
    script.close();
    system(("./abc -c 'source script.scr' >> "+abc_log_name).c_str() );
}

tuple<int,int,float> abcReadData(){
        ifstream input_file; string line,aux; int size=0,level=0; float score=0;
        input_file.open(abc_log_name);
        while(getline(input_file,line))
        {
            if(line.find("and = ")!=string::npos)
            {
                aux=line;
                line.erase(0,line.find("and = ")+9);
                line.erase(line.find("lev"),line.back());
                size=atoi(line.c_str());
                
                cout<<aux<<endl;
                aux.erase(0,line.find("lev")+9);
                cout<<aux<<endl;
//                aux.erase(line.find_first_of(" "))
                level=atoi(aux.c_str());
            }
            if(line.find("Correct =")!=string::npos)
            {
                line.erase(0,line.find_first_of("(")+2);
                line.erase(line.find_first_of("%")-1,line.size());
                score=atof(line.c_str());
            }
        }
//        ofstream refresh(abc_log_name);
        cout<<"ABC--->"<<size<<","<<level<<","<<score<<endl;
        return tuple<int,int,float> (size,level,score);
}
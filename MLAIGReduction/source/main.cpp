#include "AIG.h"
#include "binaryDS.h"
using namespace std;



int posY_max,posX_max;

int main(int argc, char** argv) {
    if(MNIST_DS == 1)
        {posY_max=28; posX_max=224;}
    else
        {posY_max=32*3; posX_max=32*8;}//3*8*32;
    
    struct rusage buf;
    int start,stop,start_simplf,stop_simplf,start_app,stop_app,iterations=0;
    if(getrusage(RUSAGE_SELF,&buf)==-1)
        cout<<"GETRUSAGE FAILURE!"<<endl;
    start=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
    string file_name,old_name,abc_name;


    ofstream dump_append("dumps/dump_append.txt"),exec_times("exec_times.csv"),script("abc_stuff/script.scr"),log("abc_stuff/log.txt"),abc_info("dumps/abc_info.txt"),nodes_file("Nodes_in_level.csv");
    log.close(); dump_append.close(); nodes_file.close();
    ifstream read_aig,read_mnist;
    read_aig.open(file_name.c_str(),ifstream::binary);
    binaryDS mnist_obj;
    aigraph graph_obj;
    int option,alpha=2,LEAVE_CONSTANTS=0;
    float min_th=0.9999;
//    option=RUN_OPTION;
    cout<<"Setting option to:"<<option<<endl;
          
    //-1->OnlyPI, 0->AllNodes 1->FB-LD linear, 2->FB-LD root, 3->FB-LD exp, 4->FB-NPD linear, 5->FB-NPD root, 6->FB-NPD exp
#if APPLY_MNIST == 1
    exec_times<<"Otption:"<<option<<", Circuit:"<<file_name<<endl<<"Min_th, Set Constants, Train Set ABC, Test Set ABC, My Simplification, Train Set, Test Set"<<endl;
#else
    exec_times<<"Otption:"<<option<<", Circuit:"<<file_name<<endl<<"Min_th, Set Constants, My Simplification"<<endl;
#endif
    ofstream results("results.csv",ios::app);
    results<<"OldName,Method,TH,#PIconstant,#ANDConstant,DiffTrain,DiffTest,DiffSize,BootsTrain,BootsTest,BootsSize,NewTrain,NewTest,NewSize,NewName"<<endl;
    results.close();

    mnist_obj.clearMnist();
    read_mnist.open("mnist/train-images.idx3-ubyte",ifstream::binary);
    mnist_obj.readIdx(read_mnist,"mnist/train-images.idx3-ubyte");
    mnist_obj.setPIsBitsProbabilities(read_mnist);
    read_mnist.close();
    
        
    vector<string> exemplars;
    string input_file="../../AIGS/";
    for (const auto & entry : fs::directory_iterator(input_file))
    {
        string my_str=entry.path();
        exemplars.push_back(my_str);
    }
    if(input_file=="../inputFromCgp-abcFix/"){
        sort(begin(exemplars), end(exemplars), [](string const& a, string const& b){
            return std::lexicographical_compare(a.begin()+(a.find("CgpAcc")+6), a.begin()+(a.find("CgpSize")) ,
                                                b.begin()+(b.find("CgpAcc")+6), b.begin()+ (b.find("CgpSize")));
        });
    }
    for(int s=exemplars.size()-1;s>=0;s--)
        cout<<exemplars[s]<<endl;
    for(int s=exemplars.size()-1;s>=0;s--){
        file_name=exemplars[s];
        graph_obj.clearAndsProbabilities();
        graph_obj.clearCircuit(); read_aig.close(); read_aig.open(file_name.c_str(),ifstream::binary);  
        graph_obj.readAIG(read_aig,file_name);
        graph_obj.setANDsProbabilities(mnist_obj);
        for(option=0;option<=6;option++){
            int th_int;
            for(th_int=999500;th_int<=999980;th_int+=10) {
                min_th=(float)th_int/1000000;
                float boots_test_acc=0,boots_train_acc=0;
                int boots_size=0;
                string method_name;
                if(option==-1) method_name="OPI";
                else if (option==0) method_name="AN";
                else if (option==1) method_name="LD-linear";
                else if (option==2) method_name="LD-root";
                else if (option==3) method_name="LD-exp";
                else if (option==4) method_name="NPD-linear";
                else if (option==5) method_name="NPD-root";
                else if (option==6) method_name="NPD-exp";
                ///////////////////////////////Generating file WITH CONSTANTS to go trhough ABC/////////////////////////////////////////////////
                cout<<"//////////////////////////"<<endl<<"/////////"<<min_th<<"///////////"<<endl<<"//////////////////////////"<<endl;

                //clearCircuit won't remove the ANDs probability values
                graph_obj.clearCircuit(); read_aig.close(); read_aig.open(file_name.c_str(),ifstream::binary);
                graph_obj.setThrehsold(min_th);        
                graph_obj.readAIG(read_aig,file_name);
                
                cout<<"Starting evaluation! Method:"<<method_name<<endl;
                graph_obj.evaluateScorseAbcCommLine21(0,3);
                boots_train_acc=graph_obj.getTrainScore();
                graph_obj.evaluateScorseAbcCommLine21(4,4);
                boots_test_acc=graph_obj.getTestScore();
                boots_size=graph_obj.getSize();
                cout<<"-----------SCORE BEFORE:"<<graph_obj.getTestScore()<<endl;
                
                LEAVE_CONSTANTS=1;  
                getrusage(RUSAGE_SELF,&buf); start_simplf=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
                    graph_obj.propagateAndDeleteAll(mnist_obj,option,min_th,alpha,LEAVE_CONSTANTS);
                getrusage(RUSAGE_SELF,&buf); stop_simplf=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
                exec_times<<method_name<<","<<min_th<<","<<((stop_simplf-start_simplf)/(float)3600)<<",";
                graph_obj.evaluateScorseAbcCommLine21(0,3);
                graph_obj.evaluateScorseAbcCommLine21(4,4);
                cout<<"-----------SCORE AFTER:"<<graph_obj.getTestScore()<<endl;

                graph_obj.writeAIG();
                old_name=graph_obj.getName();
//                abc_name="outputAigs/"+old_name+".aig";
                if(input_file=="../inputFromCgp-abcFix/"){
                    string aux_name=graph_obj.getName();
                    cout<<"!!!!!!!!!!!!!!!OLD NAME:"<<aux_name<<endl;
                    aux_name.erase(aux_name.find_first_of("-"),aux_name.size());
                    cout<<"!!!!!!!!!!!!!!!NEW NAME:"<<aux_name<<endl;
                    graph_obj.setName(aux_name);
                }
                abc_name="outputAigs/"
                        +graph_obj.getName()
                        +"-Method"+method_name
                        +"TH"+to_string(min_th)
                        +"BootsTrain"+to_string(boots_train_acc)
                        +"BootsTest"+to_string(boots_test_acc)
                        +"BootsSize"+to_string(boots_size)
                        +"NewTrain"+to_string(graph_obj.getTrainScore())
                        +"NewTest"+to_string(graph_obj.getTestScore())
                        +"NewSize"+to_string(graph_obj.getSize())
                        +".aig";
                
                abcWrite(old_name,abc_name);  
                results.open("results.csv",ios::app);
                results<<graph_obj.getName()<<","
                        <<method_name<<","
                        <<(min_th)<<","
                        <<(graph_obj.getPiConstantsSize())<<","
                        <<(graph_obj.getAndsConstantsSize())<<","
                        <<(graph_obj.getTrainScore()-boots_train_acc)<<","
                        <<(graph_obj.getTestScore()-boots_test_acc)<<","
                        <<(graph_obj.getSize()-boots_size)<<","
                        <<(boots_train_acc)<<","
                        <<(boots_test_acc)<<","
                        <<(boots_size)<<","
                        <<(graph_obj.getTrainScore())<<","
                        <<(graph_obj.getTestScore())<<","
                        <<(graph_obj.getSize())<<","
                        <<abc_name<<endl;
                results.close();
               
            }
        }
    }
    if(getrusage(RUSAGE_SELF,&buf)==-1)
        cout<<"GETRUSAGE FAILURE!"<<endl;
    stop=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
    cout<<"Time for whole process:"<<((stop-start)/(float)3600)<<endl;
        return 0;
}
    




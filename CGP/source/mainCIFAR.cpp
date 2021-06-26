/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mainCIFAR.cpp
 * Author: gudeh
 *
 * Created on May 31, 2021, 11:51 AM
 */
#if PLA_dataset == 0


#include <cstdlib>
//#include "CGP.h"
#include "AIG.h"
#include "binaryPLA.h"
#include <algorithm>
#include <chrono>  // for high_resolution_clock
#include <unistd.h> //sleep


using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    CIFAR my_cifar;
    binaryPLA dummy;
//    dataSet my_dataset;
    AigPopulation aig_popu_obj;
    AIGraph best_aig;
    double min_mutation=0.00001,mutation_chance=0.001,init_mut=2;  
    string my_root=root,my_out=out_path;
    
    int g;
    vector<string> exemplars_argv(argc-1);
    for (g=1;g<argc;g++)
        exemplars_argv[g-1]=argv[g];
#if run_option == 2        
    exemplars_argv.clear();
    string path = aigs_path;
    cout<<path<<endl;
    for (const auto & entry : fs::directory_iterator(my_root+path))
    {
        string my_str=entry.path();
        exemplars_argv.push_back(my_str);        
    }
#endif
    for(g=0;g<exemplars_argv.size();g++)
        cout<<exemplars_argv[g]<<endl;
    
    for(int s=0;s<exemplars_argv.size();s++){
#if random_seed == 1
    random_device rd;
    int seed_num=rd();
#else
//    int seed_num=;
    vector<int> seeds;
    seeds={-1562926682,1852531451};
#endif
        for(int ind=0;ind<seeds.size();ind++){
            int seed_num=seeds[ind];
            mt19937 mt(seed_num);
            int popu_size=5,boots_size=0,smaller_size=INT_MAX,write_frequency=250;
            float bootstrap_acc=0,best_acc=0;
            string path,aig_name;
            path=exemplars_argv[s];
            path.erase(path.find_last_of("/",path.size()));
            aig_name=exemplars_argv[s];
            aig_name.erase(0,aig_name.find_last_of("/"));
            best_aig.clearCircuit();
            aig_popu_obj.clearAigPopu();
            aig_popu_obj.bootstrapAigStartPopulation(path,aig_name,mt,popu_size);
            vector<int> my_vec {0};
            aig_popu_obj.evaluateScorseAbcCommLine21(&my_vec,4,4);
            bootstrap_acc=aig_popu_obj.getAigFromPopulation(0)->getCurrentScore();
            boots_size=aig_popu_obj.getAigFromPopulation(0)->getRealSize();
            aig_popu_obj.evaluateScorseAbcCommLine21(&my_vec,0,3);

            if(aig_popu_obj.getAigFromPopulation(0)->getRealSize()>=400000)
                write_frequency=100;

            for(int num_generations=0;num_generations<total_gens;num_generations++){
                #if COUT >= 1
                cout<<"--->Generation: "<<num_generations<<endl;
                #endif
                mutation_chance=aig_popu_obj.generateOffspring(mutation_chance,min_mutation,&dummy,&my_cifar,my_cifar.getNumImages(),mt);
                {
                    aig_popu_obj.evaluateScorseAbcCommLine21(&my_vec,4,4);
                    if(aig_popu_obj.getAigFromPopulation(0)->getCurrentScore()>best_acc){
                        best_acc=aig_popu_obj.getAigFromPopulation(0)->getCurrentScore();
                        smaller_size=aig_popu_obj.getAigFromPopulation(0)->getRealSize();
                        best_aig=*aig_popu_obj.getAigFromPopulation(0);
                    }
                    else if ((aig_popu_obj.getAigFromPopulation(0)->getCurrentScore()==best_acc) && (aig_popu_obj.getAigFromPopulation(0)->getRealSize()<smaller_size)){
                        best_acc=aig_popu_obj.getAigFromPopulation(0)->getCurrentScore();
                        smaller_size=aig_popu_obj.getAigFromPopulation(0)->getRealSize();
                        best_aig=*aig_popu_obj.getAigFromPopulation(0);
                    }
                    if(best_acc>bootstrap_acc){
                        if(((1+num_generations)%write_frequency==0) && num_generations<total_gens){
                        best_aig.writeAIG(my_root+my_out,
                                                        aig_popu_obj.getAigFromPopulation(0)->getName()
                                                        +"-BootsAcc"+to_string(bootstrap_acc)
                                                        +"BootsSize"+to_string(boots_size)
                                                        +"CgpAcc"+to_string(best_aig.getCurrentScore())
                                                        +"CgpSize"+to_string(best_aig.getRealSize())
                                                        +"Seed"+to_string(seed_num)
                                                        +"XtraNodes"+to_string(num_extra_nodes)
                                                        +"Gener"+to_string(num_generations)
                                                        +"-PartialBest.aig");
                        }
                    }
                    aig_popu_obj.evaluateScorseAbcCommLine21(&my_vec,0,3);
                }
            }
            aig_popu_obj.evaluateScorseAbcCommLine21(&my_vec,4,4);
            aig_popu_obj.getAigFromPopulation(0)->writeAIG(my_root+my_out,
                                                    aig_popu_obj.getAigFromPopulation(0)->getName()
                                                    +"-BootsAcc"+to_string(bootstrap_acc)
                                                    +"BootsSize"+to_string(boots_size)
                                                    +"CgpAcc"+to_string(aig_popu_obj.getAigFromPopulation(0)->getCurrentScore())
                                                    +"CgpSize"+to_string(aig_popu_obj.getAigFromPopulation(0)->getRealSize())
                                                    +"Seed"+to_string(seed_num)
                                                    +"XtraNodes"+to_string(num_extra_nodes)
                                                    +"Gener"+to_string(total_gens)
                                                    +".aig");
            abcWrite(my_root+my_out+
                                aig_popu_obj.getAigFromPopulation(0)->getName()
                                +"-BootsAcc"+to_string(bootstrap_acc)
                                +"BootsSize"+to_string(boots_size)
                                +"CgpAcc"+to_string(aig_popu_obj.getAigFromPopulation(0)->getCurrentScore())
                                +"CgpSize"+to_string(aig_popu_obj.getAigFromPopulation(0)->getRealSize())
                                +"Seed"+to_string(seed_num)
                                +"XtraNodes"+to_string(num_extra_nodes)
                                +"Gener"+to_string(total_gens),
                    my_root+my_out+
                                aig_popu_obj.getAigFromPopulation(0)->getName()
                                +"-BootsAcc"+to_string(bootstrap_acc)
                                +"BootsSize"+to_string(boots_size)
                                +"CgpAcc"+to_string(aig_popu_obj.getAigFromPopulation(0)->getCurrentScore())
                                +"CgpSize"+to_string(aig_popu_obj.getAigFromPopulation(0)->getRealSize())
                                +"Seed"+to_string(seed_num)
                                +"XtraNodes"+to_string(num_extra_nodes)
                                +"Gener"+to_string(total_gens)
                                +".aig");
            }
        }
    return 0;
}

#endif

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AIG.h
 * Author: user
 *
 * Created on April 4, 2020, 12:13 PM
 */

#ifndef AIG_H
#define AIG_H

#define mutable_PO 0
//#define checkAllNodesAsPO 0
#define new_benchmarks 4
//#define XAIG 0

#define COUT 1
#define DEBUG 0

#define total_gens 1
#define PLA_dataset 0
#define run_option 0 //2-all files in aigs_path, any other value runs emulab
#define cifarv2 1
//#define aigs_path "run/"
//#define explicit_generations 1
#define improve_size 1
//#define bootstrap_init 1
#define random_seed 0
//#define dataset_size 4
//#define ds_path "cifar-10-batches-bin/"
#define pla_path "IWLS2020-benchmarks/"

//#define smart_init 0
//#define expresso_path "Benchmarks_2_espresso_aig/"
//#define sk_path  "AIGs_SK_4020/"
#define num_extra_nodes 1

#define out_path "AIGS/"
#define root "../../"
//#define root_path "../"
#define write_times 0
#define write_learning 0

#define IGNORE_OUTPUTS 1
#define BITS_PACKAGE_SIZE 64
//#define write_size 1

//#define mini_batch 0
//#define exemplar_set 0
//#define train_with_validation_set 0
//#define argv_Ntimes 0
//#define rand_loops 0
//#define LOOP 0
//#define refresh_aig_folder 0

#include <vector>
#include <array>
#include <set>
#include <deque>
#include <stack>
#include <map>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <limits.h> //char_bit
#include <algorithm> //max
#include <stdint.h> //uintptr_t
#include <functional> //reference_wraper
#include <sys/time.h>
#include <math.h> //pow
#include <sys/resource.h> //getrusage
#include <string.h> //strlen, strncmp
#include <bitset>
#include <numeric> //iota
#include <chrono>
#include <omp.h>
//#include <unistd.h>
#include <thread>
#include <queue>
#include <functional> //reference  wrapper
//#include <filesystem> //requires c++17
#include <experimental/filesystem> //requires c++17
//namespace fs = std::filesystem;
namespace fs = std::experimental::filesystem;

//#include "CGP.h"
#include "binaryPLA.h" //check bits on bit-parallel simulation
#include "CIFAR.h"
#include "dataSet.h"



using namespace std;
//string learning_file_name;
//class CgpPopulation;

class nodeAig{
protected:
    unsigned int id;
    short int signal;
    unsigned long long int bit_vector;
public:
    nodeAig();
    nodeAig(unsigned int);
    virtual ~nodeAig();
    
    //modifiers
    virtual void pushOutput(nodeAig* param){}
    virtual void pushInput(nodeAig* param,bool param_polarity){}
    void setId(unsigned int);
    void setDepth(short int);
    void setSignal(int);
    void setBitVector(unsigned long long int);
    
    //member access
    unsigned int getId();
    int getDepth();
    int getSignal();
    unsigned long long int getBitVector();
    nodeAig* fixLSB();    
    nodeAig* forceInvert();
    virtual vector<nodeAig*> getInputs(){}
    virtual vector<nodeAig*> getOutputs(){}
    virtual vector<int> getInputPolarities(){}
    
    //operations
    virtual int computeDepthInToOut(){}
    virtual unsigned long long int PropagSignalDFS(){}
    virtual void removeOutput(unsigned int){}
    virtual void clearOutputs(){}
    virtual void writeNode(ofstream&){}
    virtual void printNode(){}
};
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class input: public nodeAig {
    vector<nodeAig*> outputs;
    bool visited;
    int visit_count;
    
public:
    input();
    using nodeAig::nodeAig; //enable use of constructor from node
    using nodeAig::setDepth;;
    using nodeAig::setBitVector;
    using nodeAig::getDepth;
    using nodeAig::getId;
    using nodeAig::getSignal;
    using nodeAig::getBitVector;
    
    virtual ~input();
   
   //modifiers
   void pushOutput(nodeAig* param) override;
   
   //operations
   void setUnvisited();
   int computeDepthInToOut() override;;
   unsigned long long int PropagSignalDFS() override;
   void clearOutputs() override;
   void writeNode(ofstream&) override;
   void printNode() override;
   //member access
   vector<nodeAig*> getOutputs() override;
   bool wasVisited();
   int getVisitCount();
   
};
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class output : public nodeAig{
    nodeAig* input;
    
public:
    output();
    using nodeAig::nodeAig; //enable use of constructor from node
    virtual ~output();
    using nodeAig::setBitVector;
    using nodeAig::setDepth;
    using nodeAig::getDepth;
    using nodeAig::getSignal;
    using nodeAig::getBitVector;
    
    //modifiers
    void pushInput(nodeAig* param,bool param_polarity);
    void clearInput();
            
    //member access
    nodeAig* getInput();
    int getInputPolarity();
    
    //operations
    void writeNode(ofstream&) override;
    int computeDepthInToOut() override;
    void printNode() override;
    unsigned long long int PropagSignalDFS() override;
};
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class AND : public nodeAig{
    vector <nodeAig*> inputs;
#if XAIG == 1
    bool isXOR;
#endif    
#if IGNORE_OUTPUTS == 0
    vector <nodeAig*> outputs;
#endif
    
public:
    AND();
    using nodeAig::nodeAig; 
    using nodeAig::setBitVector;
    using nodeAig::setDepth;
    using nodeAig::getDepth;
    using nodeAig::getSignal;
    using nodeAig::getBitVector;
    virtual ~AND();
    
    //member access
   vector<nodeAig*> getInputs() override; 
#if IGNORE_OUTPUTS == 0
       vector<nodeAig*> getOutputs() override;
#endif
   vector<int> getInputPolarities() override;
   
   //modifiers
   void pushInput(nodeAig* param,bool param_polarity) override;
   void replaceInput(int index, nodeAig* param,bool param_polarity);
   void invertInputs();
   void invertInputPolarity(int index);
#if XAIG == 1
   void setAsXOR(bool);
   bool getIsXOR();
   void clearInputs();
#endif
   
   //operations
   int computeDepthInToOut() override;
   unsigned long long int PropagSignalDFS() override;
   
#if IGNORE_OUTPUTS == 0
   void removeOutput(unsigned int id_to_remove) override;
   void clearOutputs() override;
   void pushOutput(nodeAig* param) override;
#endif
   
   void writeNode(ofstream&) override;
   void printNode() override;

};
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class XOR : public nodeAig{
    vector <nodeAig*> inputs;
#if IGNORE_OUTPUTS == 0
    vector <nodeAig*> outputs;
#endif
    
public:
    XOR();
    using nodeAig::nodeAig; 
    using nodeAig::setBitVector;
    using nodeAig::setDepth;
    using nodeAig::getDepth;
    using nodeAig::getSignal;
    using nodeAig::getBitVector;
    virtual ~XOR();
    
    //member access
   vector<nodeAig*> getInputs() override; 
#if IGNORE_OUTPUTS == 0
       vector<nodeAig*> getOutputs() override;
#endif
   vector<int> getInputPolarities() override;
   
   //modifiers
   void pushInput(nodeAig* param,bool param_polarity) override;
   void invertInputs();
   
   //operations
   int computeDepthInToOut() override;
   unsigned long long int PropagSignalDFS() override;
   
#if IGNORE_OUTPUTS == 0
   void removeOutput(unsigned int id_to_remove) override;
   void clearOutputs() override;
   void pushOutput(nodeAig* param) override;
#endif
   
   void writeNode(ofstream&) override;
   void printNode() override;

};
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class AIGraph {
protected:
    vector<input> all_inputs;
    vector<output> all_outputs;
    vector<AND> all_ANDS;
    vector<AND> extra_ands;
    string name;
    float current_score;
//    float p1_cgp;
    int functional_and_count,functional_PI_count;
    int graph_depth;
    //all_depths' index is the node ID/2 and the all_depths[ID/2] is the depth for this node.
    vector<unsigned int> all_depths;
    nodeAig constant1,constant0;
public:
    AIGraph();
    AIGraph(string name);
   virtual ~AIGraph();
    
    //modifiers
//    void CGPtoAIG(vector<short> *cgp_vector,unsigned int num_lines,unsigned int I,unsigned int O, unsigned int A, string name);
    void propagateSignalsTrainPLA(binaryPLA* PLA_combinations);
    void propagateSignalsBatchPLA(binaryPLA* PLA);
    void propagateAllAnds(binaryPLA* PLA);
    void propagateSignalsCIFAR(CIFAR* cifar);
    void readAIG(ifstream& aig_file,string aig_name,mt19937& mt);
    void setName(string);
//    void copyP1Cgp(float param);
    void clearCircuit();
    input* pushPI(unsigned int index,input input_obj);
    void pushPO(unsigned int index,output output_obj);
    AND* updateAnd(unsigned int index,AND AND_obj);
    void mutateOffpsring(float mutation_chance,mt19937& mt);
    //the copied one calls the method, with the source as parameter
    void copyAIG(AIGraph* input_aig,mt19937& mt);
    
    //getters
    nodeAig* findAny(unsigned int node_id);
    string getName();
    float getCurrentScore();
//    float getP1Cgp();
    int getRealSize();
    int getRealPIsize();
    int getDepth();
    vector<AND>* getAnds();
    vector<input>* getPIs();
    vector<output>* getPOs();
    vector<unsigned int>* getAllDepths();
    
    //prints
    void printHeader();
    void printCircuit();
    void setDepthsInToOut();
    void setScore(float score);
    void setSize(int size);
    void writeAIG(string destination, string aig_name);
    void writeAAG(string destination, string aig_name);
    //The Primary Inputs are expected to be configured already.
#if XAIG == 1
    void mapXorToAnd(string destination, string aig_name);
#endif
    
private:
    void encodeToFile(ofstream& file, unsigned x);
    unsigned char getnoneofch (ifstream& file,int);
    unsigned decode (ifstream& file,int);

};
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
///////////////////////////////////////////////

class AigPopulation{
private:
    vector<AIGraph> aig_population;
    vector<float> all_scores;
    vector<int> num_functional_ands;
    vector<int> num_functional_PIs;
    vector<int> ordered_indexes;
#if Sannealing == 1
    float temperature;
#endif
public:
    AIGraph* getAigFromPopulation(int ith_aig);
    vector<float>* getScores();
    vector<int>* getOrderedIndexes();
    vector<int>* getFuncSizes();
    vector<int>* getFuncPis();
    float getMeanFromScores();
    float getStandardDeviation();
    float getTemp();
//    float getMeanP1Cgp();
    
    void setMeanFromScores();
    void setStandardDeviation();
    void setTemp(int temp);
    void setSingleAig(AIGraph param);
    void setSingleScore(float score);
    void setSingleFuncSize(int size);
    void setSingleFuncPis(int size);
//    void copyMeanP1Cgp(float param);
//    void setMeanP1Cgp();
    
//    void CGPpopToAIGpop(CgpPopulation* cgp_popu,unsigned int I,unsigned int O, unsigned int A,bool internal_call);
//    void evaluateScoresAbcCommLine(string PLA_file,int num_to_evaluate);
    void evaluateScorseAbcCommLine(string PLA_file,int num_to_evaluate);
    void evaluateScorseAbcCommLine21(vector<int>* popu_list,int ds_start,int ds_end);
    void evaluateScoresMyImplement(binaryPLA* PLA,int num_to_evaluate,mt19937& mt);
    tuple<float,float,float> lastEvaluation(binaryPLA* PLA);
    tuple<float,float,float> firstEvaluation(binaryPLA* PLA);
    void gatherAndOrder(AigPopulation* previous_population);
    void bootstrapAigStartPopulation(string path, string aig_name,mt19937& mt,int popu_size);
//    void bootstrapAigStartPopulation(string path_aig_name,mt19937& mt,int popu_size);    
    void writeAigs();
    void checkWithAbc(binaryPLA* PLA);
    void clearAigPopu();
    void printScores();
    void printAigSizes();
    
    //new method removing CGP class
    float generateOffspring(float mutation_chance,float min_mut,binaryPLA* my_pla,CIFAR* my_cifar,int change_each,mt19937& mt);
    AIGraph* buildAigInsidePop(string name);
};


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
///////////////////////////////////////////////
struct aux_struct{
    vector<nodeAig*> outputs;
    vector<bool> firsts_polarity;
    
};

//returns a word from a phrase, words are expected to be separeted by any number of blank spaces
string wordSelector(string line, int word_index);

//checks the pointer address passed as reference if it is inverted or not.
bool getThisPtrPolarity(nodeAig* param);

int binToDec(vector<int> param);

void abcCallML(string aig_name,string PLA_file,string source);
void abcCall21(string aig_name,string cifar_file,string source);

void abcWrite(string aig_name,string abc_name);

void abcGenerateAIGfromPLA(string pla_name,string out_name);

tuple<int,int,float> abcReadData();
#endif /* AIG_H */


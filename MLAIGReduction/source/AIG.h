/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AIG.h
 * Author: augusto
 *
 * Created on 23 de Março de 2018, 16:56
 */

#ifndef GDE_H
#define GDE_H

#define DBGVAR( os, var ) \
  (os) << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
       << #var << " = [" << (var) << "]" << std::endl

#define PROBS_FROM_FILE 0
#define IGNORE_OUTPUTS 0
#define SIMPLIFIEDAIG 0
#define DEBUG 0
#define debug_value -1
#define COUT 0

//#define RUN_OPTION 52
#define TEST 0
#define RENUMBER 1
#define REMOVE_PI 0
//#define LEAVE_CONSTANTS 0
#define USE_ABC 0
#define FIX_DOUBLED_NODES 0
#define EXECUTE_ONCE 1
#define APPLY_MNIST 0
#define CEC 0
#define WRITE_AIG 0
#define WRITE_AAG 0
#define WRITE_ORIGINAL_DEPTHS 0
#define MNIST_DS 0
#define cifarv2 1
#define local_test_run 1


//#define posY_max 28
//#define posX_max 224
//#define posY_max 32
//#define posX_max  768//3*32*8

#define BITS_PACKAGE_SIZE 64
extern int posY_max;
extern int posX_max;
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
#include <bitset> //check bits on bit-parallel simulation
//#include "papi.h"
//#include "mypapi.h"
#include "binaryDS.h"

//#include <filesystem> //requires c++17
//namespace fs = std::filesystem;
#include <experimental/filesystem> //requires c++17
namespace fs = std::experimental::filesystem;

//#include "include/alice/alice.hpp"
using namespace std;









class nodeAig{
    
protected:
    unsigned int id;
    int signal;
    unsigned long long int bit_vector;
    
public:
    nodeAig();
    nodeAig(unsigned int);
    virtual ~nodeAig();
    
    //modifiers
    virtual void pushOutput(nodeAig* param){}
    virtual void pushInput(nodeAig* param,bool param_polarity){}
    void setId(unsigned int);
    void setDepth(int);
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
//    virtual void computeDepthOutToIn(int previous_signal){}
//    virtual unsigned int enumerateDFS(unsigned int index){} //TODO: this is not used anymore
    virtual unsigned long long int PropagSignalDFS(){}
    //swap_index is the index from the input that will be replaced.
    virtual void replaceInput(int swap_index,nodeAig* new_node,bool polarity){}
//    virtual void removeOutput(node*){}
    virtual void removeOutput(unsigned int){}
//    virtual void recursiveRemoveOutput(unsigned int){}
    virtual void clearOutputs(){}
    
//    virtual void writeNode(){}
    virtual void writeNode(ofstream&){}
    
    virtual void printNode(){}
};








class input: public nodeAig {
    vector<nodeAig*> outputs;
    
public:
    input();
    using nodeAig::nodeAig; //enable use of constructor from node
    using nodeAig::setDepth;
    using nodeAig::getDepth;
    using nodeAig::getId;
    using nodeAig::getSignal;
   virtual ~input();
   
   //modifiers
   void pushOutput(nodeAig* param) override;
   
   //operations
   int computeDepthInToOut() override;
//   void computeDepthOutToIn(int previous_signal) override;
//   unsigned int enumerateDFS(unsigned int index) override;
   unsigned long long int  PropagSignalDFS() override;
//   void removeOutput(node* node_to_remove) override;
   void removeOutput(unsigned int id_to_remove) override;
//   void recursiveRemoveOutput(unsigned int id_to_remove) override;
   void clearOutputs() override;
   
   
   
//   void writeNode() override;
   void writeNode(ofstream&) override;
   void printNode() override;
   //member access
   vector<nodeAig*> getOutputs() override;
//   vector<bool> getInputPolarities() override;
   

   
};








class latch : public nodeAig{
    nodeAig* input;
    vector <nodeAig*> outputs;
public:
    //constructor
    latch();
    using nodeAig::nodeAig; //enable use of constructor from node
    virtual ~latch();
    
    //member access
    using nodeAig::getDepth;
    using nodeAig::getId;
    using nodeAig::getSignal;
//    int getInputPolarity();
    nodeAig* getInput();
    
    //modifiers
    using nodeAig::setDepth;
    void pushInput(nodeAig* param,bool param_polarity) override; 
    void pushOutput(nodeAig* param) override;

    //operations    
    int computeDepthInToOut() override;
//    void computeDepthOutToIn(int previous_signal) override;
//    unsigned int enumerateDFS(unsigned int index) override;
//    unsigned int enumerateBFS(unsigned int index) override;
//    void writeNode() override;
    
};








class output : public nodeAig{
    nodeAig* input;
    
public:
    output();
    using nodeAig::nodeAig; //enable use of constructor from node
    virtual ~output();
    using nodeAig::setDepth;
    using nodeAig::getDepth;
    using nodeAig::getSignal;
    
    //modifiers
    void pushInput(nodeAig* param,bool param_polarity);
    void clearInput();
            
    //member access
    nodeAig* getInput();
    int getInputPolarity();
    
    //operations
//    void writeNode() override;
    void writeNode(ofstream&) override;
    int computeDepthInToOut() override;
//    void computeDepthOutToIn(int previous_signal) override;
//    unsigned int enumerateDFS(unsigned int param_index) override;
    void printNode() override;
    unsigned long long int PropagSignalDFS() override;

};



class AND : public nodeAig{
    vector <nodeAig*> inputs;
#if IGNORE_OUTPUTS == 0
    vector <nodeAig*> outputs;
#endif
    
public:
    AND();
    using nodeAig::nodeAig; 
    using nodeAig::setDepth;
    using nodeAig::getDepth;
    using nodeAig::getSignal;
    virtual ~AND();
    
    //member access
   vector<nodeAig*> getInputs() override; 
#if IGNORE_OUTPUTS == 0
       vector<nodeAig*> getOutputs() override;
#endif

   vector<int> getInputPolarities() override;
   
   //modifiers
   
   void pushInput(nodeAig* param,bool param_polarity) override;
   void replaceInput(int swap_index,nodeAig* new_node,bool polarity) override;
   void invertInputs();
   
   //operations
   int computeDepthInToOut() override;
//   void computeDepthOutToIn(int previous_signal) override;
//   unsigned int enumerateDFS(unsigned int index) override;
   unsigned long long int PropagSignalDFS() override;
   
#if IGNORE_OUTPUTS == 0
//   void removeOutput(node* node_to_remove) override;
   void removeOutput(unsigned int id_to_remove) override;
//   void recursiveRemoveOutput(unsigned int id_to_remove) override;
   void clearOutputs() override;
   void pushOutput(nodeAig* param) override;
#endif
   
   
//   void writeNode() override;
   void writeNode(ofstream&) override;
   void printNode() override;

};






class aigraph {
protected:
    map<unsigned int,input> all_inputs;
    map<unsigned int,latch> all_latches;
    map<unsigned int,output> all_outputs;
    map<unsigned int,AND> all_ANDS;
    string name;
    //Vector to store the probability of all ANDs
    map <unsigned int,float> ANDs_probabilities;
    //Original order of Primary Outputs
    vector<unsigned int> POs_order;
    //all_depths' index is the node ID/2 and the all_depths[ID/2] is the depth for this node.
    vector<unsigned int> all_depths;
    float threshold;
    int graph_depth;
    vector<unsigned int> greatest_depths_ids;
    nodeAig constant1,constant0;
    
    ofstream log;
    
    int ANDs_constant,ANDs_removed,PIs_constant,PIs_removed;
    int size;
    float test_score,train_score;
    
public:
    aigraph();
   virtual ~aigraph();
    
    
    //modifiers
    void readAAG(ifstream&,string);
    void readAIG(ifstream&,string);
    void setName(string);
    void setThrehsold(float);
    void clearCircuit();
    void clearAndsProbabilities();
    input* pushPI(unsigned int index,input input_obj);
    void pushLatch(unsigned int index,latch latch_obj);
    void pushPO(unsigned int index,output output_obj);
    AND* pushAnd(unsigned int index,AND AND_obj);
    void recursiveRemoveOutput(unsigned int id_to_remove,nodeAig* remove_from);
    
    //member access
    input* findInput(unsigned int);
    latch* findLatch(unsigned int);
    output* findOutput(unsigned int);
    AND* findAnd(unsigned int);
    nodeAig* findAny(unsigned int);
    map<unsigned int,input>* getInputs();
    map<unsigned int,latch>* getLatches();
    map<unsigned int,output>* getOutputs();
    map<unsigned int,AND>* getANDS();
    string getName();
    int getDepth();
    float getTestScore();
    float getTrainScore();
    int getSize();
    int getPiConstantsSize();
    int getAndsConstantsSize();
    
    //operations
    void setDepthsInToOut();
    void setShortestDistanceToPO();
    void writeAAG();
    void writeAIG();
    void writeAIG(string destination, string aig_name); //from CGP project
    void writeProbsHistogram();
    void applyMnistRecursive(binaryDS&);
    void propagateAndDeletePIBased(binaryDS& mnist_obj,float th,int LEAVE_CONSTANTS);
    void cutAIG();
    void setANDsProbabilities(binaryDS&);
    void propagateAndDeleteAll(binaryDS&,int option,float min_th,int alpha, int LEAVE_CONSTANTS);
//    void writeFileWithConstantNodes();
    void evaluateScorseAbcCommLine21(int ds_start,int ds_end);
    
    
    void assignBits(binaryDS&);
    
    
    //debugging
    void printCircuit();
    void printDepths();
    void writeCircuitDebug();
    
private:
    void encodeToFile(ofstream& file, unsigned x);
    unsigned char getnoneofch (ifstream& file,int);
    unsigned decode (ifstream& file,int);
    
    
};




class synthesizer : public aigraph{
    deque<nodeAig*> circ_deque;
    unsigned int M,I,L,O,A,AND_index;
    
private:
    void connectNodes(nodeAig* in, nodeAig* destination, bool invert);
    deque<nodeAig*> buildCellDeque(int num_inputs,bool input_inverted);
    
    void addAND(int num_inputs,bool balance);
    void addNAND(int num_inputs,bool balance);
    void addOR(int num_inputs,bool balance);
    void addNOR(int num_inputs,bool balance);
    void addXNOR(int num_inputs,bool balance);
    void addXOR(int num_inputs,bool balance);
    
    void enumerateGraph(int enumeration);
    void clearIDs();
    
    int parseLine(char*);
    int getValue();
    
public:
    synthesizer();
    ~synthesizer();
    
    
    //modifiers
    void create(int num_PI,int function,bool balance,int enumeration);
};



struct aux_struct{
    vector<nodeAig*> outputs;
    vector<bool> firsts_polarity;
};

//returns a word from a phrase, words are expected to be separeted by any number of blank spaces
string wordSelector(string line, int word_index);

//checks the pointer address passed as reference if it is inverted or not.
bool getThisPtrPolarity(nodeAig* param);

int binToDec(vector<int> param);

void abcCall21(string aig_name,string cifar_file,string source); //from CGP project

void abcWrite(string old_name,string abc_name);
void abcCeC(string new_name,string abc_name,float min_th,int option);

#endif /* GDE_H */


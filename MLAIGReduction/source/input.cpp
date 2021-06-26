/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   input.cpp
 * Author: augusto
 * 
 * Created on 23 de Março de 2018, 17:38
 */

#include "AIG.h"

input::input() {
}



input::~input() {
}

void input::pushOutput(nodeAig* param){
    this->outputs.push_back(param);
}

int input::computeDepthInToOut(){
    int depth;
    ofstream write;
    depth=0;
    this->signal=0;
    return 0; //inputs always return 0 when computing the circuit depth
}

//void input::computeDepthOutToIn(int previous){
//    this->signal=previous+1;
//    cout<<this->id<<endl;
////    cout<<"PI:"<<this->id<<":signal=="<<this->signal<<"previous:"<<previous<<endl;
//}


//unsigned int input::enumerateDFS(unsigned int index){
//    return index;
//}

//unsigned int input::enumerateBFS(unsigned int index){
//    return index;
//}


vector<nodeAig*> input::getOutputs(){
    return this->outputs;
}


void input::writeNode(ofstream& write){
    int depth;
//    ofstream write;
//    write.open("log.txt",ios::app);
    
    write<<"Input: "<<this->id<<" .Outputs("<<this->outputs.size()<<"):";
    for(int i=0;i<this->outputs.size();i++)
        write<<this->outputs[i]->getId()<<",";
    write<<endl;
}


//vector<bool> input::getInputPolarities(){
//    vector<bool> ret(2);
//    this->
//    //returns FALSE for regular and TRUE for inverted
//    ret[0]=(int)((uintptr_t)input) & 01;
//    ret[1]=(int)((uintptr_t)input) & 01;
////    cout<<this->getInputs()[0]->getId()<<":"<<ret[0]<<"|";
////    cout<<this->getInputs()[1]->getId()<<":"<<ret[1]<<endl;
////    ret[0]= inputs[0] & 0x1;
////    ret[1]= inputs[1] & 0x1;
//    return ret;
//}



unsigned long long int input::PropagSignalDFS(){
//    cout<<"END "<< this->id <<"-";
    return this->bit_vector;
}

void input::clearOutputs() {
    this->outputs.clear();
}


//void input::removeOutput(node* node_to_remove) {
void input::removeOutput(unsigned int id_to_remove) {
//    this->printNode();
    for(int i=0;i<outputs.size();i++)
    {
        if(outputs[i]->getId()==id_to_remove)
        {
//            cout<<"input:"<<this->id<<" removing output:"<<outputs[i]->getId()<<endl;
            outputs.erase(outputs.begin()+i);
            break;
//            cout<<"after remove:";
//            this->printNode();
        }
    }
}

//void input::recursiveRemoveOutput(unsigned int id_to_remove){
//    for(int i=0;i<outputs.size();i++)
//    {
//        if(outputs[i]->getId()==id_to_remove)
//        {
//            outputs.erase(outputs.begin()+i);
//            break;
//        }
//    }
//    
//}

void input::printNode(){
    cout<<"Input:"<<this->id<<". Outputs:";
    
    for(int i=0;i<this->outputs.size();i++)
        cout<<this->outputs[i]->getId()<<",";
    cout<<endl;
}
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "AIG.h"

latch::latch(){}

latch::~latch(){}


void latch::pushInput(nodeAig* param,bool param_polarity){
    //TRUE to invert FALSE to regular
    if(param_polarity)
        param=((nodeAig*)(((uintptr_t)param) ^ 01));
    input=param;
}

void latch::pushOutput(nodeAig* param){
    this->outputs.push_back(param);
}

//void latch::computeDepthOutToIn(int prevous){
//}


int latch::computeDepthInToOut(){
    int depth;
    ofstream write;

    if(depth==-1)
    {
        depth= ((nodeAig*)input)->computeDepthInToOut() +1 ;

        return depth;
    }
    else 
        return depth;
    
}

//unsigned int latch::enumerateDFS(unsigned int index){
////    index++;
//    this->id=index*2;
//    return this->getInput()->enumerateDFS(index++);
//}


//void latch::writeNode(ofstream& write){
//    int depth;
////    ofstream write;
////    write.open("log.txt",ios::app);
//    
//    write<<"Latch id: "<<this->id<<". Depth: "<<depth<<" .Input: ";
////    write<<((node*)(input/2))->getId()+!(int)this->getInputPolarity();
//    write<<" Outputs: ";
//    for(int i=0;i<this->outputs.size();i++)
//        write<<this->outputs[i]->getId()<<",";
//    write<<endl;
//}


//int latch::getInputPolarity(){
//    return (int)((uintptr_t)input) & 01;
////    return input & 0x1;
//}

nodeAig* latch::getInput(){
    return this->input->fixLSB();
}
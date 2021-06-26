/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   circuit.cpp
 * Author: augusto
 * 
 * Created on 23 de Março de 2018, 16:59
 */
 
#include <bits/c++config.h>

#include "AIG.h"

aigraph::aigraph(){
    this->constant1.setId(1);
    this->constant1.setSignal(1);
    this->constant1.setBitVector(UINT_MAX);
    this->constant0.setId(0);
    this->constant0.setSignal(0);
    this->constant0.setBitVector(0);
}
    
//graph::graph(float th){
//    this->threshold=th;
//}
aigraph::~aigraph(){}

void aigraph::setName(string param){
    this->name=param;
}

string aigraph::getName(){
    return this->name;
}

int aigraph::getDepth(){
    return this->graph_depth;
}

void aigraph::setThrehsold(float param) {
    cout<<"Setting threshold to: "<<param<<endl;
    this->threshold=param;
}


input* aigraph::pushPI(unsigned int index,input input_obj){
    this->all_inputs.insert(pair<unsigned int,input>(index,input_obj));
    return &this->all_inputs.find(index)->second;
}

void aigraph::pushLatch(unsigned int index, latch latch_obj){
    this->all_latches.insert(pair<unsigned int,latch>(index,latch_obj));
}

void aigraph::pushPO(unsigned int index, output output_obj){
    this->all_outputs.insert(pair<unsigned int,output>(index,output_obj));
}

AND* aigraph::pushAnd(unsigned int index, AND AND_obj){

    std::pair<std::map<unsigned int,AND>::iterator,bool> ret;
    ret=this->all_ANDS.insert(pair<unsigned int,AND>(index,AND_obj));

    return &ret.first->second;
}

input* aigraph::findInput(unsigned int param){
    if(all_inputs.find(param)!=all_inputs.end())
        return &all_inputs.find(param)->second;
    else 
        return nullptr;
}

latch* aigraph::findLatch(unsigned int param){
    return &this->all_latches.find(param)->second;
}

AND* aigraph::findAnd(unsigned int param){
    return &this->all_ANDS.find(param)->second;
}
output* aigraph::findOutput(unsigned int param){
    return &this->all_outputs.find(param)->second;
}

nodeAig* aigraph::findAny(unsigned int param){
    if(all_ANDS.find(param)!=all_ANDS.end())
        return &all_ANDS.find(param)->second;
    else if(all_inputs.find(param)!=all_inputs.end())
        return &all_inputs.find(param)->second;
    else if(all_latches.find(param)!=all_latches.end())
        return &all_latches.find(param)->second;
    else
    {
        cout<<"ERROR: no node found with name "<<param<<" in findAny(unsigned int param)"<<endl;
        return nullptr;
    }
}

void aigraph::readAAG(ifstream& file, string name){
    
    string line;
    string type;
    unsigned int M,I,L,O,A;
    unsigned int i,lhs,rhs0,rhs1;
    
    if(name.find("/"))
        name.erase(0,name.find_last_of('/')+1);
    if(name.find("."))
        name.erase(name.find_last_of('.'),name.size());
    this->setName(name);
    cout<<"Reading AAG circuit:"<<name<<endl;
    //reading the first line in the file
    file.seekg(file.beg);
    getline(file,line);
    //line has the index information
    type=wordSelector(line,1);
    M=stoi(wordSelector(line,2));
    I=stoi(wordSelector(line,3));
    L=stoi(wordSelector(line,4));
    O=stoi(wordSelector(line,5));
    A=stoi(wordSelector(line,6));
    bool polarity0,polarity1,lhs_polarity;
    //////////////////////INPUTS///////////////////
    for(i=0;i<I;i++){
        getline(file,line);
        input input_obj(stoi(line));
        this->pushPI(stoi(line),input_obj);
#if DEBUG >= 4
        cout<<"pushing input "<<stoi(line)<<endl;
#endif  
    }
    
  //////////////////LATCHS//////////////////////
    map<unsigned int, aux_struct>  record;
    map<unsigned int, bool> record_PO;
    //map<unsigned int,vector<node*> > record; //first is undeclared node id and second is it's list of outputs
//    vector<node*> aux_record;
//    vector<bool> aux_record_polarities;
    aux_struct aux_record;
    for(i=0;i<L;i++){
        getline(file,line);
        lhs=stoi(wordSelector(line,1));
        input latch_obj(lhs);
        this->pushPI(lhs,latch_obj);
#if DEBUG >= 4
        cout<<"pushing latch (as input) "<<stoi(line)<<endl;
#endif  
        
        //solving first input's polarity
        rhs0=stoi(wordSelector(line,2));
        if(rhs0%2!=0)
        {
            rhs0--;
            polarity0=true;
        }
        else
            polarity0=false;
        
        output output_obj(rhs0);
        record_PO.insert(pair<unsigned int,bool>(rhs0,polarity0));
    
   if(record.find(lhs)!=record.end())//if the latch is on the record
        {
            //it is an input somewhere else, needs to be solved
            for(int a=0;a<record.find(lhs)->second.outputs.size();a++)
            {
                lhs_polarity=record.find(lhs)->second.firsts_polarity[a];
                record.find(lhs)->second.outputs[a]->pushInput(findInput(lhs),lhs_polarity);
            }
        }
        
        //the latch input will become a circuit output
//        output_obj.setPolarity(polarity0);
        this->pushPO(rhs0,output_obj);
            
            
        //the latch output will become a circuit input
        this->pushPI(lhs,latch_obj);
    }
   //////////////////OUTPUTS///////////////////
    
    for(i=0;i<O;i++){
        getline(file,line);
        lhs=stoi(wordSelector(line,1));
        
        if(lhs % 2 != 0)
        {
            lhs=lhs-1;
            polarity0=true;
        }
        else
            polarity0=false;
        
        record_PO.insert(pair<unsigned int,bool>(lhs,polarity0));
        
        output output_obj(lhs);
        this->pushPO(lhs,output_obj);
#if DEBUG >= 4
        cout<<"pushing output "<<stoi(line)<<endl;
#endif  
    }
   ///////////////////ANDS////////////////
    for(i=0;i<A;i++){
        getline(file,line);
        lhs=stoi(wordSelector(line,1));
        AND AND_obj(lhs);
        this->pushAnd(lhs,AND_obj);

#if DEBUG >= 4
        cout<<"pushing AND "<<this->findAnd(lhs)->getId()<<endl;
#endif  
        
        //solving inputs' polarity
        rhs0=stoi(wordSelector(line,2));
        if(rhs0%2!=0)
        {
            rhs0--;
            polarity0=true;
        }
        else
            polarity0=false;
        
        rhs1=stoi(wordSelector(line,3));
        if(rhs1%2!=0)
        {
            rhs1--;
            polarity1=true;
        }
        else
            polarity1=false;
        
#if DEBUG >= 3
        cout<<"rhs0:"<<rhs0<<endl;
        cout<<"rhs1:"<<rhs1<<endl;
#endif
        
        if(record.find(lhs)!=record.end())//if the AND is on the record
        {
            //it is an input somewhere else, needs to be solved
            for(int a=0;a<record.find(lhs)->second.outputs.size();a++)
            {
                lhs_polarity=record.find(lhs)->second.firsts_polarity[a];
                record.find(lhs)->second.outputs[a]->pushInput(this->findAnd(lhs),lhs_polarity);
                //cout<<"adding "<<lhs<<" to "<<record.find(lhs)->second[a]->getId()<<" inputs."<<endl;
            }
        }
        //solving the AND's inputs
        //rhs0
        if(this->findAny(rhs0)!=nullptr) //if input already exists
        {
            //add the AND to the output list of the rhs0 outputs
            this->findAny(rhs0)->pushOutput(this->findAnd(lhs));
            this->findAnd(lhs)->pushInput(this->findAny(rhs0),polarity0);
        }
        else //if the input(rhs0) was not declared yet
        {
            //save it on the record and add lhs to it's outputs
            if(record.find(rhs0)==record.end())
            {
                aux_record.outputs.clear();
                aux_record.firsts_polarity.clear();
                aux_record.outputs.push_back(this->findAnd(lhs));
                aux_record.firsts_polarity.push_back(polarity0);
                record.insert(pair<unsigned int,aux_struct>  (rhs0,aux_record));
            }
            else
            {
                record.find(rhs0)->second.outputs.push_back(this->findAnd(lhs));
                record.find(rhs0)->second.firsts_polarity.push_back(polarity0);
                
            }
        }
        //the same for the second input
        //rhs1
        if(this->findAny(rhs1)!=nullptr) //if input already exists
        {
            //add the AND to the output list of the rhs1 outputs
            this->findAny(rhs1)->pushOutput(this->findAnd(lhs));
            this->findAnd(lhs)->pushInput(this->findAny(rhs1),polarity1);
        }
        else //if the input(rhs1) was not declared yet
        {
            //save it on the record and add lhs to it's outputs
            if(record.find(rhs1)==record.end()) 
            {
                aux_record.outputs.clear();
                aux_record.firsts_polarity.clear();
                aux_record.outputs.push_back(this->findAnd(lhs));
                aux_record.firsts_polarity.push_back(polarity1);
                record.insert(pair<unsigned int,aux_struct>  (rhs1,aux_record));
            }
            else
            {
                record.find(rhs1)->second.outputs.push_back(this->findAnd(lhs));
                record.find(rhs1)->second.firsts_polarity.push_back(polarity1);
            }
        }
#if DEBUG >= 3
        cout<<lhs<<"|"<<rhs0<<","<<rhs1<<endl;
#endif

    }
    file.close();
    
    map<unsigned int, output>::iterator it_out;
    for(it_out=this->all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
        it_out->second.pushInput(findAny(it_out->first),record_PO.find(it_out->first)->second);
    }
}

void aigraph::printCircuit() {
    cout<<"Circuit name: "<<this->name<<endl;
    cout<<"MILOA:"<<this->all_ANDS.size()+this->all_inputs.size()+this->all_latches.size()<<","<<this->all_inputs.size()<<","<<this->all_latches.size()<<","<<this->all_outputs.size()<<","<<this->all_ANDS.size()<<endl;
    map<unsigned int, AND>::iterator it_and;
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    map<unsigned int, latch>::iterator it_latch;
    
    cout<<"Inputs: ";
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        cout<<it_in->second.getId()<<",";
    cout<<endl;
    
    cout<<"Latches: ";
    for(it_latch=all_latches.begin();it_latch!=all_latches.end();it_latch++)
        cout<<it_latch->second.getId()<<",";
    cout<<endl;
    
    cout<<"Outputs: ";
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        cout<<it_out->second.getId()+((int)it_out->second.getInputPolarity())<<",";
    cout<<endl;
    
    cout<<"ANDs: ";
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
    {
        if(it_and->second.getInputs().size()==2)
            cout<<it_and->second.getId()<<"("<<it_and->second.getInputs()[0]->getId()+((int)it_and->second.getInputPolarities()[0])<<","<<it_and->second.getInputs()[1]->getId()+((int)it_and->second.getInputPolarities()[1])<<"),";
 
        else
        {
            cout<<"ERROR, unexpected size of AND "<<it_and->first<<":"<<it_and->second.getInputs().size()<<endl;
            for(int a=0;a<it_and->second.getInputs().size();a++)
                cout<<"intput "<<a+1<<":"<<it_and->second.getInputs()[a]->getId()<<endl;
        }
    }
    cout<<endl;
}

void aigraph::clearCircuit(){
    this->all_ANDS.clear();
    this->all_inputs.clear();
    this->all_latches.clear();
    this->all_outputs.clear();
    this->name.clear();
    this->POs_order.clear();
//    this->ANDs_probabilities.clear();;
    this->log.close();
    this->all_depths.clear();
    this->greatest_depths_ids.clear();
    this->graph_depth=0;
    this->threshold=0;
    this->train_score=0;
    this->test_score=0;
    this->size=0;
    this->ANDs_constant=0;
    this->ANDs_removed=0;
    this->PIs_constant=0;
    this->PIs_removed=0;
}

void aigraph::clearAndsProbabilities(){
    this->ANDs_probabilities.clear();
}

map<unsigned int,input>* aigraph::getInputs(){
    return &all_inputs;
}
map<unsigned int,latch>* aigraph::getLatches(){
    return &all_latches;
}
map<unsigned int,output>* aigraph::getOutputs(){
    return &all_outputs;
}
map<unsigned int,AND>* aigraph::getANDS(){
    return &all_ANDS;
}

void aigraph::writeCircuitDebug(){
    ofstream write;
    write.open("Circuit debug.txt",ios::trunc);
        write<<"Circuit name: "<<this->name<<endl;
    
    map<unsigned int, AND>::iterator it_and;
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    map<unsigned int, latch>::iterator it_latch;

    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        it_in->second.writeNode(write);

    for(it_latch=all_latches.begin();it_latch!=all_latches.end();it_latch++)
        it_latch->second.writeNode(write);

    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.writeNode(write);
    
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.writeNode(write);
}

void aigraph::writeAAG(){
    ofstream write;
    int M;
    M=all_inputs.size()+all_latches.size()+all_ANDS.size();
    write.open((name+".aag").c_str(),ios::out|ios::trunc);
    write<<"aag "<<M<<" "<<all_inputs.size()<<" "<<all_latches.size()<<" "<<all_outputs.size()<<" "<<all_ANDS.size()<<endl;
    
    //write the circuit inputs
    for(map<unsigned int,input>::iterator it1=all_inputs.begin();it1!=all_inputs.end();it1++)
        write<<it1->second.getId()<<endl;

    //write outputs
    for(map<unsigned int,output>::iterator it2=all_outputs.begin();it2!=all_outputs.end();it2++)
        write<<it2->second.getId()+((int)it2->second.getInputPolarity())<<endl;
    
    //write ANDs
    for(map<unsigned int,AND>::iterator it3=all_ANDS.begin();it3!=all_ANDS.end();it3++)
    {
        write<<it3->second.getId();
        if(it3->second.getInputs()[0]->getId()>1)
            write<<" "<<it3->second.getInputs()[0]->getId()+((int)it3->second.getInputPolarities()[0]);
        else
            write<<" "<<((it3->second.getInputs()[0]->getId())^((int)it3->second.getInputPolarities()[0]));
        if(it3->second.getInputs()[1]->getId()>1)
            write<<" "<<it3->second.getInputs()[1]->getId()+((int)it3->second.getInputPolarities()[1])<<endl;
        else
            write<<" "<<((it3->second.getInputs()[1]->getId())^((int)it3->second.getInputPolarities()[1]))<<endl;
    }
    write.close();
}

void aigraph::writeAIG(){
    ofstream write;
    unsigned char charzin;
    int M=all_inputs.size()+all_latches.size()+all_ANDS.size();
    write.open((name+".aig").c_str(),ios::binary|ios::out|ios::trunc);
    write<<"aig "<<M<<" "<<all_inputs.size()<<" "<<all_latches.size()<<" "<<all_outputs.size()<<" "<<all_ANDS.size()<<endl;
//#if LEAVE_CONSTANTS ==1
//    for(map<unsigned int,input>::iterator it1=all_inputs.begin();it1!=all_inputs.end();it1++)
//        write<<it1->second.getId()<<endl;
//#endif
    
    for(map<unsigned int,output>::iterator it2=all_outputs.begin();it2!=all_outputs.end();it2++)
        write<<it2->second.getId()+((int)it2->second.getInputPolarity())<<endl;

    //deltas
    for(map<unsigned int,AND>::iterator it3=all_ANDS.begin();it3!=all_ANDS.end();it3++)
    {
        unsigned int first;
        if(it3->second.getInputs()[0]->getId()>1)
        {
            encodeToFile(write,(it3->second.getId())-(it3->second.getInputs()[0]->getId()+(it3->second.getInputPolarities()[0])));
            first=(it3->second.getInputs()[0]->getId()+(it3->second.getInputPolarities()[0]));
        }
        else
        {
            encodeToFile(write,(it3->second.getId())-((it3->second.getInputs()[0]->getId())^(it3->second.getInputPolarities()[0])));
            first=((it3->second.getInputs()[0]->getId())^(it3->second.getInputPolarities()[0]));
        }
//        encodeToFile(write,(it3->second.getInputs()[0]->getId()+(it3->second.getInputPolarities()[0]))-(it3->second.getInputs()[1]->getId()+((int)it3->second.getInputPolarities()[1])));
        if(it3->second.getInputs()[1]->getId()>1)
            encodeToFile(write,(first)-(it3->second.getInputs()[1]->getId()+((int)it3->second.getInputPolarities()[1])));
        else
            encodeToFile(write,(first)-((it3->second.getInputs()[1]->getId())^((int)it3->second.getInputPolarities()[1])));
    }
}

void aigraph::writeAIG(string destination, string aig_name){
    ofstream write;
    int M=all_inputs.size()+all_ANDS.size();
    write.open((destination+aig_name).c_str(),ios::binary|ios::out|ios::trunc);
//    if(write.is_open())
        #if COUT>=1
        cout<<"write aig file opened:"<<(destination+aig_name)<<endl;
        #endif
//    else
//        cout<<"ERROR WRITE AIG FILE NOT OPENED!!"<<(destination+aig_name)<<endl;

    write<<"aig "<<M<<" "<<all_inputs.size()<<" 0 "<<all_outputs.size()<<" "<<all_ANDS.size()<<endl;
        for(map<unsigned int,output>::iterator it2=all_outputs.begin();it2!=all_outputs.end();it2++)
        write<<it2->second.getId()+((int)it2->second.getInputPolarity())<<endl;

    //deltas
    for(map<unsigned int,AND>::iterator it3=all_ANDS.begin();it3!=all_ANDS.end();it3++)
    {
        unsigned int first;
        if(it3->second.getInputs()[0]->getId()>1)
        {
            encodeToFile(write,(it3->second.getId())-(it3->second.getInputs()[0]->getId()+(it3->second.getInputPolarities()[0])));
            first=(it3->second.getInputs()[0]->getId()+(it3->second.getInputPolarities()[0]));
        }
        else
        {
            encodeToFile(write,(it3->second.getId())-((it3->second.getInputs()[0]->getId())^(it3->second.getInputPolarities()[0])));
            first=((it3->second.getInputs()[0]->getId())^(it3->second.getInputPolarities()[0]));
        }
//        encodeToFile(write,(it3->second.getInputs()[0]->getId()+(it3->second.getInputPolarities()[0]))-(it3->second.getInputs()[1]->getId()+((int)it3->second.getInputPolarities()[1])));
        if(it3->second.getInputs()[1]->getId()>1)
            encodeToFile(write,(first)-(it3->second.getInputs()[1]->getId()+((int)it3->second.getInputPolarities()[1])));
        else
            encodeToFile(write,(first)-((it3->second.getInputs()[1]->getId())^((int)it3->second.getInputPolarities()[1])));
    }
    write.close();
}

void aigraph::encodeToFile(ofstream& file, unsigned x){
        unsigned char ch;
        while (x & ~0x7f)
        {
            ch = (x & 0x7f) | 0x80;
            file<<ch;
            x >>= 7;
        }
        ch = x;
        file<<ch;
}

unsigned char aigraph::getnoneofch (ifstream& file,int index){
		
        int ch;
        char c;
        file.get(c);
        ch=(int)c;
        if (ch != '\0')
            return ch;
        else
        {
            cout<<"AND:"<<index<<" ch:"<<ch<<" c:"<<c<<endl;
            fprintf (stderr, "*** decode: unexpected EOF\n\n");
            return ch;
        }
    }

unsigned aigraph::decode (ifstream& file,int index){
        unsigned x = 0, i = 0;
        unsigned char ch;
        while ((ch = getnoneofch (file,index)) & 0x80)
        {
            x |= (ch & 0x7f) << (7 * i++);
//            cout<<"x:"<<x<<"|";
        }
//        cout<<"X:"<<(x | (ch << (7 * i)))<<"|";
        return x | (ch << (7 * i));
    }

void aigraph::readAIG(ifstream& file, string param_name){
    string line;
    string type;
    unsigned int M,I,L,O,A;
    unsigned int lhs,rhs0,rhs1, delta1,delta2;
    #if DEBUG >= 3
    ofstream debs("dumps/debug");
    #endif
    
    if(param_name.find("/"))
        param_name.erase(0,param_name.find_last_of('/')+1);
    if(param_name.find("."))
        param_name.erase(param_name.find_last_of('.'),param_name.size());
    this->setName(param_name);
    #if COUT>=1
    cout<<endl<<"READING AIG CIRCUIT:"<<name<<endl;
    #endif
    //reading the first line in the file
    file.seekg(file.beg);
    getline(file,line);
    //line has the index information
    type=wordSelector(line,1);
    M=stoi(wordSelector(line,2));
    I=stoi(wordSelector(line,3));
    L=stoi(wordSelector(line,4));
    O=stoi(wordSelector(line,5));
    A=stoi(wordSelector(line,6));
    bool polarity0,polarity1,lhs_polarity;
    #if COUT>=1
    cout<<"MILOA:"<<M<<","<<I<<","<<L<<","<<O<<","<<A<<endl;
    #endif
    
    for(int ii=0;ii<O;ii++)
        getline(file,line);
    #if COUT>=1
    cout<<"Reading Inputs, ";
    #endif
    //////////////////////INPUTS//////////////////////
    for(int i=1;i<=I;i++)
    {
        input input_obj(i*2);
        this->pushPI(i*2,input_obj);
        #if COUT >= 1
        cout<<"pushing input "<<i*2<<endl;
        #endif
    }
    #if COUT>=1
    cout<<"Reading ANDs, ";
    #endif
    /////////////ANDS/////////////////////////
    int and_index=I+L;
    bool polar=false;
    AND* AND_ptr;
    for(int l=0;l<A;l++)
    {
        and_index++;
        AND AND_obj(and_index*2);
        AND_ptr=this->pushAnd(and_index*2,AND_obj);
        
        delta1=decode(file,and_index*2);
        rhs0=and_index*2-delta1;
        delta2=decode(file,and_index*2);
        rhs1=rhs0-delta2;
        
//         cout<<"AND:"<<and_index*2<<". ";
//         cout<<"delta1:"<<delta1<<" delta2:"<<delta2<<". ";
//         cout<<"rhs0:"<<rhs0<<" rhs1:"<<rhs1<<endl;
#if DEBUG >=3
        debs<<"AND:"<<and_index*2<<". ";
        debs<<"delta1:"<<delta1<<" delta2:"<<delta2<<". ";
        debs<<"rhs0:"<<rhs0<<" rhs1:"<<rhs1<<endl;
#endif
//        if(rhs0>1)
//        {
            if(rhs0 % 2!=0)
            {
                polar=true;
                rhs0--;
            }
            else
                polar=false;
            AND_ptr->pushInput(this->findAny(rhs0),polar);
//        }
//        else
//        {
//            if(rhs0==1)
//                AND_ptr->pushInput(&constant1,false);
//            else
//                AND_ptr->pushInput(&constant1,false);
//        }

//        if(rhs1>1)
//        {
            if(rhs1 % 2!=0)
            {
                polar=true;
                rhs1--;
            }
            else
                polar=false;
            AND_ptr->pushInput(this->findAny(rhs1),polar);
//        }
//        else
//        {
//            if(rhs0==1)
//                AND_ptr->pushInput(&constant1,false);
//            else
//                AND_ptr->pushInput(&constant1,false);
//        }
        
        
//        this->findAny(rhs1)->printNode();
//        this->findAny(and_index*2)->printNode();
//        cout<<"delta:"<<delta1<<","<<delta2<<" and:"<<and_index*2<<","<<rhs0<<","<<rhs1<<"\t";
        
#if IGNORE_OUTPUTS == 0
//        cout<<"rhs1:"<<rhs1<<" ";
//        this->findAny(rhs1)->printNode();
//            cout<<"rhs0:"<<rhs0<<" ";
//            this->findAny(rhs0)->printNode();
    if(rhs0>1)
       this->findAny(rhs0)->pushOutput(AND_ptr);
    if(rhs1>1)
       this->findAny(rhs1)->pushOutput(AND_ptr); 
//        cout<<"rhs1:"<<rhs1<<" ";
//        this->findAny(rhs1)->printNode();
//            cout<<"rhs0:"<<rhs0<<" ";
//            this->findAny(rhs0)->printNode();
#endif
    }
#if DEBUG >= 3
    debs.close();
#endif
#if COUT>=1
    cout<<"Reading Outputs"<<endl;
#endif
        ////////////////////OUTPUTS///////////////////
    //jumping the header, right to outputs list
    file.seekg(file.beg);
    getline(file,line);
    
    for(int f=0;f<O;f++){
        getline(file,line);
       lhs=stoi(wordSelector(line,1));      
//        lhs=decode(file);
//        cout<<line<<endl;
       if(lhs>1)
       {
            if(lhs % 2 != 0)
            {
                lhs=lhs-1;
                polarity0=true;
            }
            else
                polarity0=false;
       }
       else
           cout<<"CONSTANT BEING INSTANTIATED AS PO!"<<endl;

        POs_order.push_back(lhs);
        output output_obj(lhs);
        if(lhs>1)
            output_obj.pushInput(findAny(lhs),polarity0);
//        else if(lhs==0)
//            output_obj.setSignal(0);
//        else if(lhs==1)
//            output_obj.setSignal(1);
//        else
//            cout<<"ERROR, this if statement should not be reached, (readAIG)"<<endl;
        this->pushPO(lhs,output_obj);
#if DEBUG >= 3
        cout<<"pushing output "<<lhs<<" Polarity:"<<polarity0<<endl;
#endif  
    }
//    cout<<this->all_inputs.size()<<endl;
//    this->printCircuit();
}

void aigraph::applyMnistRecursive(binaryDS& mnist_obj){
    cout<<endl<<"APPLYING IMAGES TO AIG!"<<endl;
    map<unsigned int, AND>::iterator it_and;
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    int posY=0,posX=0;
    int correct_answers=0,img_count=0,offset=0;
    ofstream vamo;
    ofstream dump_app;
    ofstream check_bits;
    ofstream fix;
    ofstream dump_sig_vector("dump_sig_vector.txt");
    ofstream dump_bits("dump_bits.txt");

    string program_output_name;
    program_output_name="Scores_";
    
    if(mnist_obj.getAllBits().size()==60000 && this->name.find("train")==string::npos)
        this->name+="_train";
    else if (mnist_obj.getAllBits().size()==10000 && this->name.find("test")==string::npos)
    {
        if(name.find("train")!=string::npos)
            name.erase(name.find("train"),5);
        this->name+="_test";
    }
    else
        cout<<"mnist size unknown"<<endl;
    program_output_name+=this->name;
    program_output_name+=".csv";
#if DEBUG >= 1
    vamo.open(program_output_name);
    dump_app.open("dump_app.txt");
    check_bits.open("input_bits.txt");
    fix.open("dumpDFS.csv");
#endif
    int num_imgs;
    num_imgs=mnist_obj.getAllBits().size();
#if TEST == 0
    for(int counter=0;counter<num_imgs;counter=counter+BITS_PACKAGE_SIZE)
#else
    for(int counter=0;counter<65;counter=counter+BITS_PACKAGE_SIZE)
#endif
        {
#if DEBUG >=1
        cout<<"Applying images from "<<offset<<" to "<<offset+BITS_PACKAGE_SIZE<<endl;
#endif
#if TEST == 0
        //getting input signals from MNIST image
        set<int> removed_inputs;
        string line;
#if SIMPLIFIEDAIG == 0
        ofstream aux_ofstr("removed_inputs.txt");
        aux_ofstr.close();
#endif
        ifstream in_file("removed_inputs.txt");
        while(getline(in_file,line))
            removed_inputs.insert(stoi(line));
    
//        cout<<"Removed inputs size:"<<removed_inputs.size()<<endl;
        it_in=all_inputs.begin();
        bitset<BITS_PACKAGE_SIZE> bits;
        posY=0;
        posX=0;
        int pixel_count=0;
#if DEBUG >= 1
        dump_app<<"ignorando inputs:"<<endl;
#endif
//        cout<<"chosen positions:";
        while(pixel_count< (posX_max*posY_max))
       {
            if(removed_inputs.find((((posY)*(posX_max)) +posX+1)*2)==removed_inputs.end())
            {
                if(it_in==all_inputs.end())
                    cout<<"ERROR, out of range on all_inputs (applyMnistRecursive)."<<endl;
                bits.reset();
                for(int u=offset;u<offset+BITS_PACKAGE_SIZE;u++)
                {
                    if(u>=num_imgs)
                        break;
                    bits.set(u-offset,(bool)mnist_obj.getBit(u,posY,posX));
                }
                if(name.find("A3")==string::npos)
                    it_in->second.setBitVector(bits.to_ullong());
                else if (((posY)*(posX_max)+posX)>0)
                {
                    if((posX+1)%8 == 0)
                        it_in->second.setBitVector(bits.to_ullong());
                }
#if DEBUG >= 1
                dump_app<<it_in->second.getId()<<"="<<((((posY)*(posX_max)) +posX+1)*2)<<endl;
#endif
                
                it_in++;
            }
#if DEBUG >= 1
            else
                dump_app<<"ignoring:"<<((((posY)*(posX_max)) +posX+1)*2)<<" holding on it_in:"<<it_in->second.getId()<<endl;
#endif
            pixel_count++;
            posX++;
            if(posX==posX_max)
            {
                    posY++;
                    posX=0;
                    if(posY==posY_max)
                            posY=0;
            }
       }
#else 
        cout<<"RJAEIRJEOI"<<endl;
        bitset<BITS_PACKAGE_SIZE> bits;
        for(int u=offset;u<offset+BITS_PACKAGE_SIZE;u++)
            bits.set(u-offset,true);
//        bits.reset();
        cout<<bits<<endl;
        all_inputs.find(2)->second.setBitVector(bits.to_ullong());
        cout<<all_inputs.find(2)->second.getBitVector()<<endl;
        all_inputs.find(4)->second.setBitVector(bits.to_ullong());
        cout<<all_inputs.find(4)->second.getBitVector()<<endl;
        all_inputs.find(6)->second.setBitVector(bits.to_ullong());
        cout<<all_inputs.find(6)->second.getBitVector()<<endl;
        all_inputs.find(8)->second.setBitVector(bits.to_ullong());
         cout<<all_inputs.find(8)->second.getBitVector()<<endl;
        cout<<"RJAEIRJEOI"<<endl;
//       all_inputs.find(2)->second.setSignal(1);
//       all_inputs.find(4)->second.setSignal(1);
//       all_inputs.find(6)->second.setSignal(1);
//       all_inputs.find(8)->second.setSignal(1);
#endif

//#else
//        bitset<BITS_PACKAGE_SIZE> bits;
//        posY=0;
//        posX=0;
//        //getting input signals from MNIST image
//        for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
//        {
////            dump_sig_vector<<endl<<"INPUT:"<<it_in->second.getId()<<endl;
//            bits.reset();
////            cout<<"offset:"<<offset<<endl;
//            for(int u=offset;u<offset+BITS_PACKAGE_SIZE;u++)
//            {
//                if(u>=num_imgs)
//                    break;
////                cout<<u<<",";
//                bits.set(u-offset,(bool)mnist_obj.getBit(u,posY,posX));
////                dump_sig_vector<<"u:"<<u<<"->"<<(bool)mnist_obj.getBit(u,posY,posX)<<endl;
////                dump_sig_vector<<bits<<endl;
//            }
////            check_bits<<bits;
////               it_in->second.setSignal(mnist_obj.getBit(img_count,posY,posX));
//               posX++;
//               if(posX==posX_max)
//               {
////                   check_bits<<endl;
//                   posY++;
//                   posX=0;
//                   if(posY==posY_max)
//                   {
//                       posY=0;
////                       check_bits<<"end of image"<<endl;
//                   }
//               }
//            
////            check_bits<<it_in->second.getId()<<":"<<bits<<endl;
//            it_in->second.setBitVector(bits.to_ullong());
//            //check_bits<<"raw:"<<bits.to_ullong()<<endl;
//            //check_bits<<it_in->second.getBitVector();
//        }
//#endif

        //initializing all ANDs with -1
        for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
            it_and->second.setSignal(-1);
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
            it_out->second.setSignal(-1);
        
        struct rusage buf; 
        int start,stop;
//        if(getrusage(RUSAGE_SELF,&buf)==-1)
//            cout<<"GETRUSAGE FAILURE!"<<endl;
//        start=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
//
//        cout<<"DFS START"<<endl;
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        {
            if(it_out->second.getId()>1)
                it_out->second.PropagSignalDFS();
        }
//        if(getrusage(RUSAGE_SELF,&buf)==-1)
//            cout<<"GETRUSAGE FAILURE!"<<endl;
//        stop=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
//        
//        cout<<"Time to DFS:"<<stop-start<<endl;

       int aux=0;
       vector<vector<int> > PO_signals;
       map<nodeAig*,int>::iterator it_sig;
       vector<int> aux_vec;
       vector<vector<int> > images_bits(64);
       map<nodeAig*,int>::iterator iter;
       
       unsigned long long int mask=1;
       //inverting outputs depending on node's polarity
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        {
            if(it_out->second.getId()>1)
            {
                if(it_out->second.getInputPolarity())
                    it_out->second.setBitVector(~it_out->second.getBitVector());
            }
            else if (it_out->second.getId()==1)
                it_out->second.setBitVector(ULLONG_MAX);
            else if (it_out->second.getId()==0)
                it_out->second.setBitVector(0);
        }       
#if DEBUG >=1
        {
//            dump_app<<"image index:"<<img_count+1<<". Label:"<<mnist_obj.getLabel(img_count)<<endl;
            dump_app<<endl<<"Inputs' signals:"<<endl;
            for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
                dump_app<<it_in->first<<":"<<it_in->second.getBitVector()<<endl;//<<",";
            dump_app<<endl<<endl;

            dump_app<<endl<<"Outputs' signals:"<<endl;
            for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
            {
                if(it_out->second.getId()>1)
                    dump_app<<it_out->first<<":"<<it_out->second.getBitVector()<<endl;//" ["<<it_out->second.getSignal()<<"^"<<it_out->second.getInputPolarity()<<"]"<<endl;//<<",";
                else
                    dump_app<<it_out->second.getId()<<endl;
            }
            dump_app<<endl<<endl;

            dump_app<<endl<<"ANDs' signals:"<<endl;
            for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
                dump_app<<it_and->first<<":"<<it_and->second.getBitVector()<<"|";
            dump_app<<endl;
            
            dump_app<<"-----------------------------------------------------"<<endl<<endl;
        }
#endif
#if DEBUG >= 2
       for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        {
           bitset<64> x(it_out->second.getBitVector());
           cout<<"output:"<<it_out->second.getId()+it_out->second.getInputPolarity()<<"->"<<x<<endl;
       }
#endif
       
       images_bits.clear();
       //Separating the outputs in to groups of 16 bits
       int b=0;
       while(b<all_outputs.size())
       {
            it_out=all_outputs.find(POs_order[b]);
            for(int u=0;u<BITS_PACKAGE_SIZE;u++)
            {
                mask=1;
                mask=mask<<u;
                if(it_out->second.getId()>1)
                    images_bits[u].push_back((mask & it_out->second.getBitVector()) >>u);
                else if (it_out->second.getId()==1)
                    images_bits[u].push_back(1);
                else
                    images_bits[u].push_back(0);
#if DEBUG >= 2
                cout<<((mask & it_out->second.getBitVector()) >>u);
#endif
            }
#if DEBUG >= 2
            cout<<endl;
#endif
            
            b++;
       }
//    cout<<"images_bits size:"<<images_bits.size()<<endl;
//    cout<<"images_bits[0] size:"<<images_bits[0].size()<<endl;
//    cout<<"images_bits[1] size:"<<images_bits[1].size()<<endl;
//    for(int a=0;a<images_bits.size();a++)
//    {
//        dump_bits<<"image:"<<a<<"->";
//        for(int b=0;b<images_bits[a].size();b++)
//            dump_bits<<images_bits[a][b];
//        dump_bits<<endl;
//    }
    
    
        for(int u=0;u<BITS_PACKAGE_SIZE;u++)
        {
            img_count=offset+u;
            if(img_count>=num_imgs)
            {
                img_count--;
                break;
            }
                
            PO_signals.clear();
            if(all_outputs.size()==160)
            {
                for(aux=1;aux<=160;aux++)
                {
                    aux_vec.push_back(images_bits[u][aux-1]);
                    if(aux%16==0)
                    {
                        PO_signals.push_back(aux_vec);
                        aux_vec.clear();
                    } 
                }
            }
            else if (all_outputs.size()==10)
            {
                for(aux=1;aux<=10;aux++)
                {
                    aux_vec.push_back(images_bits[u][aux-1]);
                    PO_signals.push_back(aux_vec);
                    aux_vec.clear();
                }
            }
            else
                cout<<"ERROR, unexpected circuit size of primary outputs!!"<<endl;

               aux_vec.clear();
#if DEBUG >= 1
               vamo<<"image index:"<<img_count+1<<". Label:"<<mnist_obj.getLabel(img_count);
#endif

               //counting number of 0s and 1s
               int ones=0,zeroes=0;
               for(int i=0;i<PO_signals.size();i++)
               {
                   for(int j=0;j<PO_signals[i].size();j++)
                   {
                       if(PO_signals[i][j]==1)
                           ones++;
                       else if(PO_signals[i][j]==0)
                           zeroes++;
                       else
                           cout<<"ERROR, output signal is neither 0 or 1:"<<PO_signals[i][j]<<endl;
                   }
               }
#if DEBUG >= 1
               vamo<<", #1:"<<ones<<" #0:"<<zeroes;
               vamo<<endl;
#endif


               //from binary to decimal
               for(int w=0;w<PO_signals.size();w++)
               {        
//                   if(PO_signals[w][0]!=1 && PO_signals[w][0]!=0);
//                       cout<<"ERROR, unexpected signal value in apply_image:"<<PO_signals[w][0]<<endl;
                   int soma=0;
                   if(all_outputs.size()==160)
                        soma=binToDec(PO_signals[w]);
                   
                   else if(all_outputs.size()==10)
                   {
                       if(PO_signals[w].size()>1)
                           cout<<"ERROR, PO_signals size is greater than 1.(apply mnist recursive)"<<endl;
                       soma=PO_signals[w][0];
                   }
                   else
                       cout<<"Error, all outputs size is not either 10 or 160. (apply mnist recursive)"<<endl;
                    aux_vec.push_back(soma);
#if DEBUG >= 1
                    vamo<<w<<","<<soma;
                    vamo<<",,'";
                    for(int tt=0;tt<PO_signals[w].size();tt++)
                        vamo<<PO_signals[w][tt];
                   vamo<<endl;
#endif
               }
                PO_signals.clear();
//               map<int,vector<int> > img_scores;
//               img_scores.insert(pair<int,vector<int> >(img_count,aux_vec));

               
//               for(int k=0;k<img_scores.rbegin()->second.size();k++)
//               {
//                   if(highest_score<img_scores.rbegin()->second[k])
//                   {
//                       highest_score=img_scores.rbegin()->second[k];
//                       highest_index=k;
//                   }
//               }
                int count=0;
                if(all_outputs.size()==10)
                {
                    for(int k=0;k<aux_vec.size();k++)
                    {
                        if(aux_vec[k]==1)
                            count++;
                    }
                }
                //checking if current image is correct

               int highest_score=-32768;
               
               int highest_index=0;
               for(int k=0;k<aux_vec.size();k++)
               {
                   if(highest_score<aux_vec[k])
                   {
                       highest_score=aux_vec[k];
                       highest_index=k;
                   }
               }
               aux_vec.clear();
               if(count>1)
                   highest_score=0;
//               if(highest_score<=0)
//               {
//                   cout<<"Highest score number on image "<<img_count+1<<" is negative or 0:"<<highest_score<<endl;
//                   vamo<<"WRONG ANSWER!"<<endl;
//               }
//               else 
               if(highest_index==mnist_obj.getLabel(img_count))
               {
                   correct_answers++;
#if DEBUG >= 1
                   cout<<"RIGHT ANSWER ON IMAGE "<<img_count+1<<". Label:"<<mnist_obj.getLabel(img_count)<<". Score:"<<highest_score<<endl;
                   vamo<<"RIGHT ANSWER"<<endl;
#endif
               }
               else
               {
#if DEBUG >= 1
                   cout<<"WRONG ANSWER ON IMAGE "<<img_count+1<<". Label:"<<mnist_obj.getLabel(img_count)<<endl;
                   vamo<<"WRONG ANSWER"<<endl;
#endif
               }
               cout<<"IMG:"<<img_count+1<<":"<<(float)correct_answers/(img_count+1)<<"\t";
#if DEBUG >= 1
               vamo<<"total right answers:"<<correct_answers<<endl;
               if(correct_answers==0)
                   vamo<<"accuracy:0"<<endl;
               else
                   vamo<<"accuracy:"<<(float)correct_answers/(img_count+1)<<endl;
#endif

        }
       cout<<endl;
    offset=offset+BITS_PACKAGE_SIZE;
    if(offset>num_imgs)
        offset=num_imgs;
    }
    ofstream csv_final;
    csv_final.open("todos_scores.csv",ios::app);
    csv_final<<this->name<<","<<(float)correct_answers/(img_count+1);
    if(this->name.find("test")!=string::npos)
        csv_final<<endl;
    else
        csv_final<<",";
    cout<<"Circuit "<<this->name<<" has accuracy:"<<(float)correct_answers/(img_count+1)<<endl;
}


void aigraph::propagateAndDeletePIBased(binaryDS& mnist_obj,float th,int LEAVE_CONSTANTS) {
#if COUT>=1
    if(LEAVE_CONSTANTS==0)
        cout<<endl<<"SIMPLIFING CIRCUIT PI ONLY: "<<this->name<<", THRESHOLD:"<<th<<endl;
    else
        cout<<endl<<"SETTING CONSTANTS PI ONLY: "<<this->name<<", THRESHOLD:"<<th<<endl;
#endif
    int PI_constant=0,posX=0,posY=0;
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    map<unsigned int, AND>::iterator it_and;
    map<unsigned int, AND>::reverse_iterator it_rand;
    ofstream dump1("dumps/dump1.txt"); ofstream dump2("dumps/dump2.txt"); ofstream dump3("dumps/dump3-renumbering.txt");
//    string info_file_name;
//    info_file_name=this->name;
//    info_file_name+=to_string(1-threshold);
//    info_file_name+="_simplif_info.txt";
//    ofstream simpl_info(info_file_name);
    constant1.setSignal(1);
    constant1.setId(1);
    constant0.setSignal(0);
    constant0.setId(0);
    

#if WRITE_ORIGINAL_DEPTHS == 1
    this->setDepthsInToOut();
    this->all_depths.push_back(0);
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        all_depths.push_back(0);
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        all_depths.push_back(it_and->second.getDepth());
    vector<int> nodes_in_level(this->graph_depth+1,0);
    for(int v=0;v<all_depths.size();v++)
        nodes_in_level[all_depths[v]]++;
    nodes_in_level[0]--;
    
    ofstream original_nodes_level("original_nodes_level");
    for(int v=0;v<nodes_in_level.size();v++)
        original_nodes_level<<v<<","<<nodes_in_level[v]<<endl;
#endif
    
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        it_in->second.setSignal(2);
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.setSignal(-1);
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.setSignal(-1);
    //Inputs with probability of being 0 less than threshold are set to zero
  for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
  {
//        cout<<mnist_obj.getProbabilities()[posY][posX]<<",";
      if(mnist_obj.getPIsProbabilities()[posY][posX]<= 1-th)
      {
#if TEST == 0
            for(int g=0;g<it_in->second.getOutputs().size();g++)
            {
                if(it_in->second.getOutputs()[g]->getInputs()[0]->getId()==it_in->second.getId())
                    it_in->second.getOutputs()[g]->replaceInput(0,&constant0,it_in->second.getOutputs()[g]->getInputPolarities()[0]);
                else if(it_in->second.getOutputs()[g]->getInputs()[1]->getId()==it_in->second.getId())
                    it_in->second.getOutputs()[g]->replaceInput(1,&constant0,it_in->second.getOutputs()[g]->getInputPolarities()[1]);
            }
            it_in->second.clearOutputs();
#endif
            PI_constant++;
      }   
      posX++;
      if(posX==posX_max)
      {
          posY++;
          posX=0;
          if(posY==posY_max)
              posY=0;
      }
  }
#if COUT >= 1
  cout<<"# of PI that pass threshold:"<<PI_constant<<endl;
#endif
  this->PIs_constant=PI_constant;
//  simpl_info<<"Threshold:"<<to_string(1-threshold);
//  simpl_info<<"# of PI that pass threshold:"<<PI_constant<<endl;
  

    
#if TEST == 1
//  int conts=0;
//  for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
//      it_in->second.setSignal(2);
//  
//  for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++,conts++)
//  {
//      if(conts<10)
//          it_in->second.setSignal(0);
//  }
//  all_inputs.find(2)->second.setSignal(2);
//  all_inputs.find(4)->second.setSignal(0);
//  all_inputs.find(6)->second.setSignal(2);
#endif
    
//  int 3=2;
    
    vector<nodeAig*> stack;
    vector<nodeAig*> AUX;
    nodeAig* current;
//    cout<<"POs size:"<<all_outputs.size()<<endl<<endl;
    bool polarity,pol_new_node;
    nodeAig* new_node;
    
if(LEAVE_CONSTANTS == 0)
{
    //DFS to propagate constants
    for(it_out=this->all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
#if DEBUG >= 3
        dump1<<"DFS on PO:"<<it_out->second.getId()<<endl;
#endif
        stack.clear();
        //if PO's input is not a PI, push back on stack
        if(all_inputs.find(it_out->second.getInput()->getId())==all_inputs.end() && it_out->second.getId()>1)
            stack.push_back(it_out->second.getInput());
        //-1 means not visited
        //2 means not a constant
        while(!stack.empty())
        {            
            current=stack.back();
            if(current->getInputs()[0]->getSignal()==-1)
                stack.push_back(current->getInputs()[0]);
            else if(current->getInputs()[1]->getSignal()==-1)
                stack.push_back(current->getInputs()[1]);
            
            else if(current->getInputs()[0]->getSignal()>-1 && current->getInputs()[1]->getSignal()>-1)
            {
                int first=0,second=0;
                if(current->getInputs()[0]->getSignal()==2)
                    first=2;
                else
                    first=((int)current->getInputs()[0]->getSignal())^((int)current->getInputPolarities()[0]);
                
                if(current->getInputs()[1]->getSignal()==2)
                    second=2;
                else
                    second=((int)current->getInputs()[1]->getSignal())^((int)current->getInputPolarities()[1]);
#if DEBUG >= 3
                dump1<<endl<<"current before setsignal:"<<current->getId()<<", inputs:"<<current->getInputs()[0]->getId()<<"->"<<first<<". "<<current->getInputs()[1]->getId()<<"->"<<second<<endl;
#endif
                if(first==0 || second==0)
                {
                    current->setSignal(0);
                    current->clearOutputs();
                    current->getInputs()[0]->removeOutput(current->getId());
                    current->getInputs()[1]->removeOutput(current->getId());
#if DEBUG >= 3
                    dump1<<"0:";
                    current->writeNode(dump1);
#endif
                }
                else if(first==1 && second==1)
                {
                    current->setSignal(1);
                    current->clearOutputs();
                    current->getInputs()[0]->removeOutput(current->getId());
                    current->getInputs()[1]->removeOutput(current->getId());
#if DEBUG >= 3
                    dump1<<"1 & 1:";
                    current->writeNode(dump1);
#endif
                }
                else if(first==2 && second==2)
                {
                    current->setSignal(2);
#if DEBUG >= 3
                    dump1<<"2 & 2:";
                    current->writeNode(dump1);
#endif
                }
                //reconnecting
                else if(first==2 || second==2)
                {
#if DEBUG >= 3
                    dump1<<"Removing:";
                    current->writeNode(dump1);
#endif
                    current->setSignal(2);
                    if(first!=2)
                    {
#if DEBUG >= 3
                        dump1<<current->getInputs()[0]->getId()<<":1 and "<<current->getInputs()[1]->getId()<<":2"<<endl;
#endif
                        new_node=current->getInputs()[1];
                        pol_new_node=current->getInputPolarities()[1];
                    }
                    else if (second!=2)
                    {
#if DEBUG >= 3
                        dump1<<current->getInputs()[0]->getId()<<":2 and "<<current->getInputs()[1]->getId()<<":1"<<endl;
#endif
                        new_node=current->getInputs()[0];
                        pol_new_node=current->getInputPolarities()[0];
                    }
                    else
                            cout<<"ERROR, this if statement should not be reached1."<<endl;
                    AUX=current->getOutputs();
                    for(int l=0;l<AUX.size();l++)
                    {
#if DEBUG >= 3
                        dump1<<"current's output before:";
                        AUX[l]->writeNode(dump1);
#endif
                        if(AUX[l]->getInputs()[0]->getId()==current->getId())
                        {
#if DEBUG >= 3
                            dump1<<"(int)pol_new_node:"<<(int)pol_new_node<<". AUX[l]->getInputPolarities()[0]:"<<AUX[l]->getInputPolarities()[0]<<endl;
                            dump1<<"(bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[0]):"<<((bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[0]))<<endl;
#endif
                            polarity=(bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[0]);
                            AUX[l]->replaceInput(0,new_node,polarity);
                        }
                        else if (AUX[l]->getInputs()[1]->getId()==current->getId())
                        {
#if DEBUG >= 3
                            dump1<<"(int)pol_new_node:"<<(int)pol_new_node<<". AUX[l]->getInputPolarities()[1]:"<<AUX[l]->getInputPolarities()[1]<<endl;
                            dump1<<"(bool)((int)pol_new_nodepolarity)^(AUX[l]->getInputPolarities()[1]):"<<((bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[1]))<<endl;
#endif
                            polarity=(bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[1]);
                            AUX[l]->replaceInput(1,new_node,polarity);
                        }
                        else
                            cout<<"ERROR, this else statement should never be reached. (propagateAndDelete)"<<endl;
                        new_node->pushOutput(AUX[l]);
#if DEBUG >= 3
                        dump1<<AUX[l]->getId()<<" reciving new node:"<<new_node->getId()<<endl;
                        dump1<<"current's output after:";
                        AUX[l]->writeNode(dump1);
#endif
                    }
                    current->clearOutputs();
                    current->getInputs()[0]->removeOutput(current->getId());
                    current->getInputs()[1]->removeOutput(current->getId());
                    //treating if current to be removed is a PO
                    if(all_outputs.find(current->getId())!=all_outputs.end())
                    {
#if DEBUG >= 3                       
                        dump1<<"Renumbering output:"<<current->getId()<<" with:"<<new_node->getId()<<endl;
#endif
                        all_outputs.find(current->getId())->second.setId(new_node->getId());
                        polarity=((int)pol_new_node)^all_outputs.find(current->getId())->second.getInputPolarity();
                        all_outputs.find(current->getId())->second.pushInput(new_node,polarity);
                    }
                }
                else
                    cout<<"ERROR, this if statement should not be reached2."<<endl;
                stack.pop_back();
            }
        }
        if(it_out->second.getId()>1)
            it_out->second.setSignal(it_out->second.getInput()->getSignal());
//        if(it_out->second.getSignal()!=2)
//        {
//            cout<<"WARNING: output "<<it_out->second.getId()<<" has signal after constant propagation:"<<it_out->second.getSignal()<<endl;
//            simpl_info<<"WARNING: output "<<it_out->second.getId()<<" has signal after constant propagation:"<<it_out->second.getSignal()<<endl;
//        }
    }
    cout<<"Signal propagation done."<<endl;
#if DEBUG >=2
    dump2<<"Inputs signals:";
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        dump2<<it_in->second.getId()<<":"<<it_in->second.getSignal()<<"|";
    
    dump2<<endl<<endl<<"Outputs Signals:";
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
        dump2<<it_out->second.getId()<<":"<<(it_out->second.getSignal());
        if(it_out->second.getInputPolarity())
            dump2<<"!";
        dump2<<"|";
    }
    
    dump2<<endl<<endl<<"ANDs Signals:";
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        dump2<<it_and->second.getId()<<":"<<it_and->second.getSignal()<<"|";
#endif 
    //making sure outputs wont be deleted, by adding themselfs to their fanout list
    for(it_out=this->all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.getInput()->pushOutput(it_out->second.getInput());
    
    //Removing ANDs with 0 fanouts
    int ands_removed_aux=0,PIs_removed_aux=0,id=0;
    vector<int> removed_nodes_counter_by_depth(graph_depth+1,0);
    it_and=all_ANDS.begin();
    while(it_and!=all_ANDS.end())
    {
        if(it_and->second.getOutputs().size()==0) //&& all_outputs.find(it_and->first)==all_outputs.end())
        {
//            cout<<"Erasing AND:"<<it_and->first<<endl;
            removed_nodes_counter_by_depth[this->all_depths[it_and->second.getId()/2]]++;
            it_and=all_ANDS.erase(it_and);
            ands_removed_aux++;
        }
        else
            it_and++;
    }
    
    ofstream write("removed_inputs.txt");
    it_in=all_inputs.begin();
    //COUNTING inputs with 0 fanouts
    while(it_in!=all_inputs.end())
    {
        if(it_in->second.getOutputs().size()==0)
            PIs_removed_aux++;
        it_in++;
    }
#if REMOVE_PI ==1
    it_in=all_inputs.begin();
    //Removing inputs with 0 fanouts
    while(it_in!=all_inputs.end())
    {
        if(it_in->second.getOutputs().size()==0)
        {
            write<<it_in->first<<endl;
            it_in=all_inputs.erase(it_in);
            PIs_removed_aux++;
        }
        else
            it_in++;
    }
#endif
    cout<<"ANDs removed:"<<ands_removed_aux<<endl;
    cout<<"Inputs removed:"<<PIs_removed_aux<<endl;

//    simpl_info<<"ANDs removed:"<<ands_removed<<endl;
//    simpl_info<<"Inputs removed:"<<PIs_removed<<endl<<endl;
      //    this->name+="_after_remove";
   this->name+="_";
   this->name+=to_string(1-threshold);
    if(mnist_obj.getAllBits().size()==60000 && this->name.find("train")==string::npos)
        this->name+="_train_old";
    else if (mnist_obj.getAllBits().size()==10000 && this->name.find("test")==string::npos)
        this->name+="_test_old";
    else
        cout<<"mnist size unknown"<<endl;
    
#if DEBUG >= 1
    int sum=0;
    for(int t=0;t<removed_nodes_counter_by_depth.size();t++)
        sum+=removed_nodes_counter_by_depth[t];
    string file_name;
    file_name="Nodes_in_level.csv";
    ofstream nodes_per_level(file_name,ios::app);
//    dump1<<file_name<<":"<<sum<<","<<ands_removed<<endl;
    nodes_per_level<<this->name<<","<<th;
    for(int a=0;a<removed_nodes_counter_by_depth.size();a++)
        nodes_per_level<<","<<nodes_in_level[a]-removed_nodes_counter_by_depth[a];
    nodes_per_level<<endl;
    nodes_per_level.close();
#endif
    
    this->setDepthsInToOut();
    
    ofstream csv_final;
    csv_final.open("todos_scores.csv",ios::app);
//    simpl_info<<endl<<to_string(1-threshold)<<","<<PI_constant<<","<<PIs_removed<<","<<ands_removed<<","<<all_ANDS.size()<<endl;
    csv_final<<this->name<<"_FIXED_TH_"<<th<<","<<PI_constant<<","<<PIs_removed_aux<<","<<ands_removed_aux<<","<<all_ANDS.size()<<",,"<<graph_depth<<",,,,";
#if APPLY_MNIST == 0
    csv_final<<endl;
#endif
csv_final.close();
    
    //Renumbering PIs
#if DEBUG >= 3
    dump3<<"ANDs removed:"<<ands_removed_aux<<endl;
    dump3<<"Inputs removed:"<<PIs_removed_aux<<endl;

    //Renumbering PIs
   dump3<<endl<<endl<<"Renunmbering Inputs:"<<endl;
#endif
    id=1;
   for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++,id++)
   {
#if DEBUG >= 3
       dump3<<it_in->first<<"==>"<<id*2<<endl;
#endif
       it_in->second.setId(id*2);
   }
    
   //Renumbering ANDs
#if DEBUG >= 3
   dump3<<endl<<endl<<"Renumbering ANDS:"<<endl;
#endif
   for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++,id++)
   {
#if DEBUG >= 3
       dump3<<it_and->first<<"==>"<<id*2<<endl;
#endif
       it_and->second.setId(id*2);
   }
#if DEBUG >= 3
   dump3<<endl<<endl;
#endif

   //Renumbering POs
   for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
   {
#if DEBUG >= 3
        dump3<<it_out->second.getId()<<"==>"<<it_out->second.getInput()->getId()<<endl;
#endif
        it_out->second.setId(it_out->second.getInput()->getId());
   }
}  
   
    //Reordering AND's inputs (bigger first)
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.invertInputs();

#if WRITE_AIG == 1
    cout<<"Writing output file (AIG):"<<this->name<<endl;
    this->writeAIG();
#endif
#if WRITE_AAG == 1
    cout<<"Writing output file (AAG):"<<this->name<<endl;
    this->writeAAG();
#endif
}

void aigraph::cutAIG(){
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    map<unsigned int, AND>::iterator it_and;
    
    aigraph AIG_cut;
    vector<nodeAig*> stack;
    nodeAig* current;
    input* aux_PI;
    AND* aux_AND;
    
    
    //Initializing node's for DFS, -1 means not visited
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        it_in->second.setSignal(0);
    
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.setSignal(-1);

    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.setSignal(-1);
    
    
    
//    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
    it_out=all_outputs.begin();
    {
        AIG_cut.setName(to_string(it_out->second.getId()));
        AIG_cut.pushPO(it_out->second.getId(),it_out->second);
        it_out->second.setSignal(1);
        stack.clear();
        stack.push_back(it_out->second.getInput());
        
        //DFS to cut the AIG based on primary output
        while(!stack.empty())
        {
            current=stack.back();
            if(current->getInputs()[0]->getSignal()==-1)
                stack.push_back(current->getInputs()[0]);
            else if (current->getInputs()[1]->getSignal()==-1)
                stack.push_back(current->getInputs()[1]);
            else if(current->getInputs()[0]->getSignal()>=0 && current->getInputs()[1]->getSignal()>=0)
            {
                AND and_obj(current->getId());
                aux_AND=AIG_cut.pushAnd(current->getId(),and_obj);
                
                if(current->getInputs()[0]->getSignal()==0)
                {
                    if(AIG_cut.findInput(current->getInputs()[0]->getId())==nullptr)
                    {
                        input pi_obj(current->getInputs()[0]->getId());
                        aux_PI=AIG_cut.pushPI(current->getInputs()[0]->getId(),pi_obj);
                    }
                    else
                        aux_PI=AIG_cut.findInput(current->getInputs()[0]->getId());
                    aux_AND->pushInput(aux_PI,current->getInputPolarities()[0]);
                }
                else
                    aux_AND->pushInput(AIG_cut.findAnd(current->getInputs()[0]->getId()),current->getInputPolarities()[0]);
                
                if(current->getInputs()[1]->getSignal()==0)
                {
                    if(AIG_cut.findInput(current->getInputs()[1]->getId())==nullptr)
                    {
                        input pi_obj(current->getInputs()[1]->getId());
                        aux_PI=AIG_cut.pushPI(current->getInputs()[1]->getId(),pi_obj);
                    }
                    else
                        aux_PI=AIG_cut.findInput(current->getInputs()[1]->getId());
                    aux_AND->pushInput(aux_PI,current->getInputPolarities()[1]);
                }
                else
                    aux_AND->pushInput(AIG_cut.findAnd(current->getInputs()[1]->getId()),current->getInputPolarities()[0]);
                
                current->setSignal(1);
                stack.pop_back();
            }
        }
        AIG_cut.writeAIG();
        AIG_cut.writeAAG();
    }
}

void aigraph::setANDsProbabilities(binaryDS& mnist_obj){
    map<unsigned int,AND>::iterator it_and;
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        this->ANDs_probabilities.insert(pair<unsigned int,float>(it_and->second.getId(),0.0));
    
    cout<<"///////////Setting ANDs probabilities///////////"<<endl;
    map<unsigned int, float>::iterator it_probs;
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    
    int posY=0,posX=0;
    int offset=0;//mnist_obj.getAllBits().size();
    int num_imgs=mnist_obj.getAllBits().size();
    cout<<"PIs probs size:"<<mnist_obj.getPIsProbabilities().size()<<endl;
    cout<<"mnist_obj.getAllBits().size():"<<mnist_obj.getAllBits().size()<<endl;
    cout<<"mnist_obj.getAllBits()[0].size():"<<mnist_obj.getAllBits()[0].size()<<endl;
#if TEST == 0
    for(int counter=0;counter<num_imgs;counter=counter+BITS_PACKAGE_SIZE)
#else
    for(int counter=59968;counter<num_imgs;counter=counter+BITS_PACKAGE_SIZE)
#endif
        {
#if DEBUG>=2
        cout<<"Applying images from "<<offset<<" to "<<offset+BITS_PACKAGE_SIZE<<endl;
#endif
//        if(offset+BITS_PACKAGE_SIZE>num_imgs)
//            continue;
        bitset<BITS_PACKAGE_SIZE> bits;
        posY=0;
        posX=0;
        
#if DEBUG>=3
        cout<<"all inputs size:"<<all_inputs.size()<<endl;
        cout<<"mnist_obj.getAllBits().size():"<<num_imgs<<endl;
#endif
#if TEST == 1
        offset=59968;
#endif
        //getting input signals from MNIST image
        for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        {
            bits.reset();
            int u=offset;
            while(u<num_imgs && u<offset+BITS_PACKAGE_SIZE)
            {
//                cout<<"u:"<<u<<endl;
                bits.set(u-offset,(bool)mnist_obj.getBit(u,posY,posX));
                u++;
            }
               posX++;
               if(posX==posX_max)
               {
                   posY++;
                   posX=0;
                   if(posY==posY_max)
                       posY=0;
               }
            it_in->second.setBitVector(bits.to_ullong());
//            cout<<bits<<endl;
        }
        //initializing all ANDs with -1
        for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
            it_and->second.setSignal(-1);
        //initializing all Outputs with -1
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
            it_out->second.setSignal(-1);
        //////////////////////////////////////////////////////////////////////////////////////////
       /////////////////////////////////////DFS//////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////
#if DEBUG >=3        
        struct rusage buf; 
        int start,stop;
        if(getrusage(RUSAGE_SELF,&buf)==-1)
            cout<<"GETRUSAGE FAILURE!"<<endl;
        start=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
#endif
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
            it_out->second.PropagSignalDFS();
#if DEBUG >=3
        if(getrusage(RUSAGE_SELF,&buf)==-1)
            cout<<"GETRUSAGE FAILURE!"<<endl;
        stop=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;
        cout<<"Time to DFS:"<<stop-start<<endl;
#endif
       //inverting outputs depending on node's polarity
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++){
            if(it_out->second.getInputPolarity())
                it_out->second.setBitVector(~it_out->second.getBitVector());
        }
        unsigned long long int num,full_one,auxiliar;
        int repeat=0,auxx=0;
        //counting 1s in each AND's bit vector
        for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++){
            repeat=0;
            num=it_and->second.getBitVector();                        
            if(offset>=num_imgs-(num_imgs%BITS_PACKAGE_SIZE)){
                bitset<BITS_PACKAGE_SIZE> my_aux;
                auxx=0;
                while(auxx<(num_imgs%BITS_PACKAGE_SIZE))
                    {my_aux.set(auxx,1); auxx++;}
                auxiliar=my_aux.to_ullong();
                num=num&auxiliar;
            }
            while(num){
                num=num&(num-1);
                repeat++;
            }
            it_probs=ANDs_probabilities.find(it_and->second.getId());
            it_probs->second=it_probs->second+repeat;
        }
        
        offset=offset+BITS_PACKAGE_SIZE;
        if(offset>num_imgs)
            offset=num_imgs;
    }    
        map<unsigned int,float>::iterator probs_it;
        ofstream debuging,debuging2;
        
#if DEBUG >= 2
        debuging.open("ands_probs.txt");
        debuging2.open("ands_reps.txt");
        debuging2<<"num_imgs:"<<num_imgs<<endl;
        for(probs_it=ANDs_probabilities.begin();probs_it!=ANDs_probabilities.end();probs_it++)
            debuging2<<probs_it->first<<","<<probs_it->second<<endl;
#endif
        for(probs_it=ANDs_probabilities.begin();probs_it!=ANDs_probabilities.end();probs_it++)
            probs_it->second=(probs_it->second)/num_imgs;
#if DEBUG >= 2
        for(probs_it=ANDs_probabilities.begin();probs_it!=ANDs_probabilities.end();probs_it++)
            debuging<<probs_it->first<<","<<probs_it->second<<endl;
#endif
}

void aigraph::propagateAndDeleteAll(binaryDS& mnist_obj,int option,float min_th,int alpha,int LEAVE_CONSTANTS) {
#if COUT>=1
    if(LEAVE_CONSTANTS==0)
        cout<<endl<<"SIMPLIFING CIRCUIT: "<<this->name<<", THRESHOLD:"<<min_th<<endl;
    else
        cout<<endl<<"SETTING CONSTANTS: "<<this->name<<", THRESHOLD:"<<min_th<<endl;
#endif
    float th_inverted=0;
    int aux=0;
    string th_value;//,info_file_name;
    if(option==0)
    {
        th_value="_fixed_";
        th_value+=to_string(threshold);
    }
    else if(option>0)
    {
        if(option==1)
          th_value="_level_linear";
        else if (option==2)
            th_value="_level_root";
        else if (option==3)
            th_value="_level_exp";
        else if (option==4)
            th_value="_#of_nodes_linear";
        else if (option==5)
            th_value="_#of_nodes_root";
        else if (option==6)
            th_value="_#of_nodes_exp";
        th_value+="_min_";
        th_value+=to_string(min_th);
        if(option>1 && option<=3)
        {
            th_value+="_alpha_";
            th_value+=to_string(alpha);
        }
    }
    int PI_constant=0,posX=0,posY=0;
    map<unsigned int, input>::iterator it_in;
    map<unsigned int, output>::iterator it_out;
    map<unsigned int, AND>::iterator it_and;
//    info_file_name=this->name; info_file_name+="_"; info_file_name+=th_value; info_file_name+="_simplif_info.txt";
    ofstream dump1("dumps/dump1.txt"),dump2("dumps/dump2.txt"),dump3("dumps/dump3-renumbering.txt"),dump_probs("dumps/dump_probs.txt"),dump_PO("dumps/dump_PO.txt"),dump_hash("dumps/dump_hash.txt");
    ofstream dump_append("dumps/dump_append.txt",ios::app),removed_inputs("removed_inputs.txt");//,simpl_info(info_file_name);
    
    //dump1.close();dump2.close();dump3.close();dump_probs.close();dump_PO.close();dump_hash.close();dump_append.close(); removed_inputs.close();
    map<unsigned int,float>::iterator probs_it;
    vector<int> visits;
    visits.push_back(2);
    constant1.setSignal(1);
    constant1.setId(1);
    constant0.setSignal(0);
    constant0.setId(0);
    
    //creating structural hash
//    cout<<"Creating hash table for structural hash."<<endl;
//    map <unsigned long long int,unsigned int> structural_hash;
//    unsigned long long int result;
//    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
//    {
//        unsigned long long bits1,bits2;
//        if(it_and->second.getInputs()[0]->getId() < it_and->second.getInputs()[1]->getId())
//        {
//            bits1=(unsigned long long int)it_and->second.getInputs()[0]->getId();
//            bits2=(unsigned long long int)it_and->second.getInputs()[1]->getId();
//        }
//        else
//        {
//            bits2=(unsigned long long int)it_and->second.getInputs()[0]->getId();
//            bits1=(unsigned long long int)it_and->second.getInputs()[1]->getId();
//        }
//        
//        result=(bits1 << 32) | bits2;
//        if(it_and->second.getId()<=10000)
//        {
//            dump_hash<<result<<endl;
//            bitset<64> w(result),x(bits1),y(bits2);
//            dump_hash<<x<<" + "<<y<<" = "<<w<<endl;
//        }
//        if(structural_hash.find(result)==structural_hash.end())
//            structural_hash.insert(pair<unsigned long long int,unsigned int> (result,it_and->second.getId()));
//        else
//        {
//            if(it_and->second.getId()<=10000)
//            {
//                dump_hash<<"ERROR: unexpected same node in structural hash: ";
//    //            it_and->second.printNode();
//                all_ANDS.find(structural_hash.find(result)->second)->second.writeNode(dump_hash);
//                it_and->second.writeNode(dump_hash);
//            }
//       }
//        
//    }
    
    
    this->setDepthsInToOut();
    this->all_depths.push_back(0);
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        all_depths.push_back(0);
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        all_depths.push_back(it_and->second.getDepth());
    vector<int> nodes_in_level(this->graph_depth+1,0);
    for(int v=0;v<all_depths.size();v++)
        nodes_in_level[all_depths[v]]++;
    nodes_in_level[0]--;
#if WRITE_ORIGINAL_DEPTHS == 1
    ofstream original_nodes_level("original_nodes_level");
    for(int v=0;v<nodes_in_level.size();v++)
        original_nodes_level<<v<<","<<nodes_in_level[v]<<endl;
#endif
//    simpl_info<<"AIG depth:"<<this->graph_depth<<", nodes with such depth:";
//    for(int a=0;a<this->greatest_depths_ids.size();a++)
//        simpl_info<<greatest_depths_ids[a]<<",";
//    simpl_info<<endl; 
    
#if DEBUG >= 3
    ofstream all_depths_out;
    all_depths_out.open("all_depths.txt");
    for(int o=0;o<all_depths.size();o++)
        all_depths_out<<o<<":"<<all_depths[o]<<endl;
    all_depths_out.close();
#endif
    //Initializing nodes
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
    {
        visits.push_back(2);
        it_in->second.setSignal(2);
    }
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
    {
        visits.push_back(-1);
        it_and->second.setSignal(-1);
    }
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.setSignal(-1); 
    
    //Inputs with probability of being 0 less than threshold are set to zero
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
    {
//        if(mnist_obj.getPIsProbabilities()[posY][posX]<= th_inverted)
        if(mnist_obj.getPIsProbabilities()[posY][posX]<= 1-min_th)        
        {
//            dump3<<"input:"<<it_in->second.getId()<<" probab:"<<mnist_obj.getPIsProbabilities()[posY][posX]<<" <= th:"<<th_inverted<<endl;
//  valendo->          dump3<<"input:"<<it_in->second.getId()<<" probab:"<<mnist_obj.getPIsProbabilities()[posY][posX]<<" <= th:"<<(1-min_th)<<endl;
//                dump_append<<"input:"<<it_in->second.getId()<<", probab:"<<mnist_obj.getPIsProbabilities()[posY][posX]<<", min_th:"<<min_th<<", th_inverted (PI):"<<th_inverted<<endl;
//  valendo->              dump_append<<"input:"<<it_in->second.getId()<<", probab:"<<mnist_obj.getPIsProbabilities()[posY][posX]<<", min_th:"<<min_th<<", th_inverted (PI):"<<(1-min_th)<<endl;
#if TEST == 0
            for(int g=0;g<it_in->second.getOutputs().size();g++)
            {
                if(it_in->second.getOutputs()[g]->getInputs()[0]->getId()==it_in->second.getId())
                    it_in->second.getOutputs()[g]->replaceInput(0,&constant0,it_in->second.getOutputs()[g]->getInputPolarities()[0]);
                else if(it_in->second.getOutputs()[g]->getInputs()[1]->getId()==it_in->second.getId())
                    it_in->second.getOutputs()[g]->replaceInput(1,&constant0,it_in->second.getOutputs()[g]->getInputPolarities()[1]);
            }
            it_in->second.clearOutputs();
#endif
            PI_constant++;
        }   
        posX++;
        if(posX==posX_max)
        {
            posY++;
            posX=0;
            if(posY==posY_max)
                posY=0;
        }
    }
  cout<<"Threshold:"<<th_value<<", # of PI that pass threshold:"<<PI_constant<<endl;
  this->PIs_constant=PI_constant;
//  simpl_info<<"Threshold:"<<th_value<<endl;
//  simpl_info<<"# of PI that pass threshold:"<<PI_constant<<endl;
  
#if TEST == 1
  all_inputs.find(2)->second.setSignal(2);
  all_inputs.find(4)->second.setSignal(2);
//        for(int g=0;g<all_inputs.find(4)->second.getOutputs().size();g++)
//        {
//            if(all_inputs.find(4)->second.getOutputs()[g]->getInputs()[0]->getId()==all_inputs.find(4)->second.getId())
//                all_inputs.find(4)->second.getOutputs()[g]->replaceInput(0,&constant0,all_inputs.find(4)->second.getOutputs()[g]->getInputPolarities()[0]);
//            else if(all_inputs.find(4)->second.getOutputs()[g]->getInputs()[1]->getId()==all_inputs.find(4)->second.getId())
//                all_inputs.find(4)->second.getOutputs()[g]->replaceInput(1,&constant0,all_inputs.find(4)->second.getOutputs()[g]->getInputPolarities()[1]);
//        }
//  all_inputs.find(4)->second.clearOutputs();
  all_inputs.find(6)->second.setSignal(2);
//          for(int g=0;g<all_inputs.find(6)->second.getOutputs().size();g++)
//        {
//            if(all_inputs.find(6)->second.getOutputs()[g]->getInputs()[0]->getId()==all_inputs.find(1)->second.getId())
//                all_inputs.find(6)->second.getOutputs()[g]->replaceInput(0,&constant0,all_inputs.find(1)->second.getOutputs()[g]->getInputPolarities()[0]);
//            else if(all_inputs.find(6)->second.getOutputs()[g]->getInputs()[1]->getId()==all_inputs.find(1)->second.getId())
//                all_inputs.find(6)->second.getOutputs()[g]->replaceInput(1,&constant0,all_inputs.find(1)->second.getOutputs()[g]->getInputPolarities()[1]);
//        }
  all_inputs.find(8)->second.setSignal(2);;
  all_inputs.find(10)->second.setSignal(2);
#endif

  
#if PROBS_FROM_FILE ==1
    ANDs_probabilities.clear();
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        this->ANDs_probabilities.insert(pair<unsigned int,float>(it_and->second.getId(),0.0));
    string ands_probs_name,line;
    
    ands_probs_name="../ands_probs_";
    cout<<"ands probs file name:"<<ands_probs_name<<endl;    cout<<"graph file name:"<<this->name<<endl;
    if(this->name.find("A1")!=string::npos)
        ands_probs_name+="A1.txt";
    else if(this->name.find("A2")!=string::npos)
        ands_probs_name+="A2.txt";
    else if(this->name.find("A3")!=string::npos)
        ands_probs_name+="A3.txt";
    else if(this->name.find("A4")!=string::npos)
        ands_probs_name+="A4.txt";
    else
    {
        cout<<"ERROR, graph name has no A1-4"<<endl;
        ands_probs_name="ands_probs.txt";
        cout<<"Using:"<<ands_probs_name<<endl;
    }
//    dump1<<"circuit name:"<<this->name<<". Probs file name:"<<ands_probs_name<<endl;;
    ifstream in_file (ands_probs_name);

    it_and=all_ANDS.begin();
    for(probs_it=ANDs_probabilities.begin();probs_it!=ANDs_probabilities.end();probs_it++)
    {
        getline(in_file,line);
        line.erase(0,line.find(",")+1);
        probs_it->second=stof(line);
    }
#else
//    this->setANDsProbabilities(mnist_obj);
#endif
    //couting the repetitions of nodes in each logic level
    vector<int> depth_counter (this->graph_depth+1,0);
    depth_counter[0]=all_inputs.size();
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        depth_counter[all_depths[it_and->second.getId()/2]]++;
    int biggest_depth=0;
    biggest_depth=*max_element(begin(depth_counter),end(depth_counter));
#if DEBUG>=1
    dump2<<"depth,number of nodes"<<endl;
    for(int x=0;x<depth_counter.size();x++)
        dump2<<x<<","<<depth_counter[x]<<","<<endl;
    dump2<<"biggest depth:"<<biggest_depth<<endl;
#endif
    
    vector<float> new_ths(graph_depth+1,0);
    if(option>0)
    {
        if(option==1) //linear
        {
            for(int k=0;k<new_ths.size();k++)
                new_ths[k]=(((1-min_th)*k)/(graph_depth))+min_th;
        }
        else if (option==2) //root
        {
            for(int k=0;k<new_ths.size();k++)
                new_ths[k]=((1-min_th)*(pow((float)k/((float)graph_depth),(float)1/alpha)))+min_th;
        }
        else if (option==3) //exp
        {
            for(int k=0;k<new_ths.size();k++)
                new_ths[k]=((1-min_th)*(pow((float)k/((float)graph_depth),alpha)))+min_th;
        }
        else if (option==4) //number of nodes per level, linear
        {
            for(int k=0;k<new_ths.size();k++)
                new_ths[k]=((1-min_th)*((((float)-depth_counter[k])/(biggest_depth-1))+((float)biggest_depth/(biggest_depth-1))))+min_th;
        }
        else if (option==5 ) //number of nodes per level, root
        {
            for(int k=0;k<new_ths.size();k++)
                new_ths[k]=((1-min_th)*(-pow((((float)depth_counter[k])/(biggest_depth-1)),alpha)+((float)biggest_depth/(biggest_depth-1))))+min_th;
        }
        else if (option==6 ) //number of nodes per level, root
        {
            for(int k=0;k<new_ths.size();k++)
                new_ths[k]=((1-min_th)*(-pow((((float)depth_counter[k])/(biggest_depth-1)),(float)1/alpha)+((float)biggest_depth/(biggest_depth-1))))+min_th;
        }
    }
    else
    {
        for(int k=0;k<new_ths.size();k++)
            new_ths[k]=threshold;
    }
//    dump1<<probs_it->first<<":"<<all_depths[probs_it->first/2]<<", new_th:"<<new_th<<endl;
#if DEBUG >= 1
    dump1<<"new thresholds already set\ndepth,TH,1-TH"<<endl;
    for(int k=0;k<new_ths.size();k++)
    {
        dump1<<k<<","<<new_ths[k]<<",";
        dump1<<(1-new_ths[k])<<endl;
    }
#endif
    
    int one_count=0,zero_count=0;
    struct rusage buf; 
    int start,stop;
    AND* and_ptr; 
//    dump_append<<endl<<endl;
  //ANDs with probability of being 0 or 1 higher than threshold are set to constant    
    for(probs_it=ANDs_probabilities.begin();probs_it!=ANDs_probabilities.end();probs_it++)
    {
        and_ptr=&all_ANDS.find(probs_it->first)->second;
        if(probs_it->second<= (1- new_ths[this->all_depths[probs_it->first/2]]))
//        if(probs_it->second<= th_inverted)
        {
#if DEBUG >=3
            dump2<<"found constant0->probs_it->first:"<<probs_it->first<<",probs_it->second:"<<probs_it->second<<",(1- new_ths[this->all_depths[probs_it->first/2]]):"<<(1- new_ths[all_depths[probs_it->first/2]]);
//            dump2<<"0->probes_it->first:"<<probs_it->first<<", probes_it->second"<<probs_it->second<<",th_inverted:"<<th_inverted;
            dump2<<",depth:"<<all_depths[probs_it->first/2]<<endl;
//            dump_probs<<"0->probes_it->first:"<<probs_it->first<<",probs_it->second:"<<probs_it->second<<endl;
#endif
            if(all_outputs.find(probs_it->first)==all_outputs.end())
            {
                for(int j=0;j<and_ptr->getOutputs().size();j++)
                {
//                    and_ptr->getOutputs()[j];
                    if(and_ptr->getOutputs()[j]->getInputs()[0]->getId()==and_ptr->getId())
                       and_ptr->getOutputs()[j]->replaceInput(0,&constant0,and_ptr->getOutputs()[j]->getInputPolarities()[0]); 
                    else if (and_ptr->getOutputs()[j]->getInputs()[1]->getId()==and_ptr->getId())
                        and_ptr->getOutputs()[j]->replaceInput(1,&constant0,and_ptr->getOutputs()[j]->getInputPolarities()[1]);
                }
            }
           zero_count++;
        }        
        if(probs_it->second >= new_ths[this->all_depths[probs_it->first/2]])
        {
//            if(probs_it->first<=12800)
//                dump_append<<"AND:"<<probs_it->first<<", probab:"<<probs_it->second<<", new_th:"<<(new_ths[this->all_depths[probs_it->first/2]])<<", th_inverted (AND):"<<th_inverted<<endl;
#if DEBUG >=3
            dump2<<"found constant1-> probs_it->first:"<<probs_it->first<<",probs_it->second"<<probs_it->second<<",(new_ths[this->all_depths[probs_it->first/2]]):"<<(new_ths[this->all_depths[probs_it->first/2]]);
            dump2<<",depth:"<<all_depths[probs_it->first/2]<<endl;
//            dump_probs<<"1->probes_it->first:"<<probs_it->first<<",probs_it->second:"<<probs_it->second<<endl;
#endif
            
            if(all_outputs.find(probs_it->first)==all_outputs.end())
            {
                for(int j=0;j<and_ptr->getOutputs().size();j++)
                {
                    and_ptr->getOutputs()[j];
                    if(and_ptr->getOutputs()[j]->getInputs()[0]->getId()==and_ptr->getId())
                       and_ptr->getOutputs()[j]->replaceInput(0,&constant1,and_ptr->getOutputs()[j]->getInputPolarities()[0]); 
                    else if (and_ptr->getOutputs()[j]->getInputs()[1]->getId()==and_ptr->getId())
                        and_ptr->getOutputs()[j]->replaceInput(1,&constant1,and_ptr->getOutputs()[j]->getInputPolarities()[1]); 
                }
            }
        one_count++;
        }
    }

  cout<<"# of ANDs to be constant 1:"<<one_count<<endl;
  cout<<"# of ANDs to be constant 0:"<<zero_count<<endl;
  this->ANDs_constant=one_count+zero_count;
//  simpl_info<<"# of ANDs to be constant 1:"<<one_count<<endl;
//  simpl_info<<"# of ANDs to be constant 0:"<<zero_count<<endl;
//  for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
//      dump1<<it_and->second.getId()<<":"<<it_and->second.getSignal()<<endl;
  
    stack<nodeAig*>stackzin;
    vector<nodeAig*> AUX;
    nodeAig* current;
//    cout<<"POs size:"<<all_outputs.size()<<endl<<endl;
    bool polarity,pol_new_node;
    nodeAig* new_node;

    if(LEAVE_CONSTANTS == 0)
    {
//#if LEAVE_CONSTANTS == 0
    //DFS to propagate constants
    for(it_out=this->all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
#if DEBUG >= 3
        dump1<<"DFS on PO:"<<it_out->second.getId()<<endl;
#endif
        while(!stackzin.empty())
            stackzin.pop();
        //if PO's input is not a PI, push back on stack
        if(all_inputs.find(it_out->second.getInput()->getId())==all_inputs.end())
            stackzin.push(it_out->second.getInput());
        
        //-1 means not visited
        //2 means not a constant
        while(!stackzin.empty())
        {            
            current=stackzin.top();
            if(visits[current->getInputs()[0]->getId()/2]==-1)
                stackzin.push(current->getInputs()[0]);
            else if(visits[current->getInputs()[1]->getId()/2]==-1)
                stackzin.push(current->getInputs()[1]);            
            else if(visits[current->getInputs()[1]->getId()/2]>-1 && visits[current->getInputs()[0]->getId()/2]>-1)
            {
                if(current->getSignal()==-1)
                {
#if FIX_DOUBLED_NODES == 1
//                    if(current->getInputs()[0]->getId()>1)
//                    {
                    //current's inputs are the same node
                    if((current->getInputs()[0]->getId()==current->getInputs()[1]->getId()) && (current->getInputs()[0]->getId()>1))
                    {
                        //if they have the same polarity, means A*A=A, !(0^0)=1
                        if(!(current->getInputPolarities()[0])^(current->getInputPolarities()[1]))
                        {
//                            cout<<"SAME POLARITIES!!!!"<<endl;
                            if(all_outputs.find(current->getId())!=all_outputs.end())
                            {
                                dump_PO.open("drump_PO",ios::app); dump_PO<<"Replacing PO. Node: "<<current->getId()<<" has the same input:"<<current->getInputs()[0]->getId()<<endl; dump_PO.close();
                                all_outputs.find(current->getId())->second.pushInput(current->getInputs()[0],(bool)all_outputs.find(current->getId())->second.getInputPolarity());
                                all_outputs.find(current->getId())->second.setId(current->getInputs()[0]->getId());
                            }
                            else
                            {
                                new_node=current->getInputs()[0];
                                pol_new_node=current;
                                AUX=current->getOutputs();
                                for(int l=0;l<AUX.size();l++)
                                {
                                    if(AUX[l]->getInputs()[0]->getId()==current->getId())
                                    {
                                        polarity=(bool)(AUX[l]->getInputPolarities()[0]^current->getInputPolarities()[0]);
                                        AUX[l]->replaceInput(0,new_node,polarity);
                                    }
                                    else if (AUX[l]->getInputs()[1]->getId()==current->getId())
                                    {
                                        polarity=(bool)(AUX[l]->getInputPolarities()[1]^current->getInputPolarities()[0]);
                                        AUX[l]->replaceInput(1,new_node,polarity);
                                    }
                                    else
                                        cout<<"ERROR, this else statement should never be reached. (propagateAndDelete)"<<endl;
                                    new_node->pushOutput(AUX[l]);
                                }
                            }
                        }
                        //if polarities are different means A*!A=0, !(1^0)=0
                        else
                        {
//                            cout<<"DIFFERNT POLARITIES!!!! Current:";
//                            current->printNode();
                            if(all_outputs.find(current->getId())!=all_outputs.end())
                            {
                                dump_PO.open("drump_PO",ios::app);
                                dump_PO<<"PO is a constant 0, has the same input with inverted polarities:";
                                current->writeNode(dump_PO);
                                dump_PO<<current->getId()<<" probability:"<<ANDs_probabilities.find(current->getId())->second<<endl;
                                dump_PO.close();
//                                all_outputs.find(current->getId())->second.clearInput();
                                all_outputs.find(current->getId())->second.pushInput(&constant0,(bool)all_outputs.find(current->getId())->second.getInputPolarity());
                                all_outputs.find(current->getId())->second.setId(0);
                            }
                            current->setSignal(0);
                            recursiveRemoveOutput(current->getId(),current->getInputs()[0]);
                            recursiveRemoveOutput(current->getId(),current->getInputs()[1]);
                        }
                        current->clearOutputs();
                        current->getInputs()[0]->removeOutput(current->getId());
                        current->getInputs()[1]->removeOutput(current->getId());
                    }
//                    }
                    else
#endif
                    {
                        int first=0,second=0;
                        if(current->getInputs()[0]->getSignal()==2)
                            first=2;
                        else
                            first=(current->getInputs()[0]->getSignal())^(current->getInputPolarities()[0]);

                        if(current->getInputs()[1]->getSignal()==2)
                            second=2;
                        else
                            second=(current->getInputs()[1]->getSignal())^(current->getInputPolarities()[1]);
#if DEBUG >= 3
                dump1<<endl<<"current before setsignal:"<<current->getId()<<", inputs:"<<current->getInputs()[0]->getId()<<"->"<<first<<". "<<current->getInputs()[1]->getId()<<"->"<<second<<endl;
#endif
                        if(first==0 || second==0)
                        {
                             //treating if constant=0 node is a PO.
                            if(all_outputs.find(current->getId())!=all_outputs.end())
                            {
                                dump_PO.open("drump_PO",ios::app);
                                dump_PO<<"PO is a constant 0:";
                                current->writeNode(dump_PO);
                                dump_PO<<current->getId()<<" probability:"<<ANDs_probabilities.find(current->getId())->second<<endl;
                                dump_PO.close();
                                all_outputs.find(current->getId())->second.clearInput();
                                all_outputs.find(current->getId())->second.setId(0);
                            }
                            current->setSignal(0);
                            current->clearOutputs();
                            current->getInputs()[0]->removeOutput(current->getId());
                            current->getInputs()[1]->removeOutput(current->getId());
#if DEBUG >= 3
                            dump1<<"0 || 0:";
                            current->writeNode(dump1);
#endif
                        }
                        else if(first==1 && second==1)
                        {
                            if(all_outputs.find(current->getId())!=all_outputs.end())
                            {
#if DEBUG >= 1
                                dump_PO.open("dump_PO",ios::app);
                                dump_PO<<"PO is a constant 1:";
                                current->writeNode(dump_PO);
                                dump_PO<<current->getId()<<" probability:"<<ANDs_probabilities.find(current->getId())->second<<endl;
                                dump_PO.close();
#endif
        //                        simpl_info<<"PO is a constant 1:"<<current->getId()<<endl;;
                                all_outputs.find(current->getId())->second.clearInput();
                                all_outputs.find(current->getId())->second.setId(1);

                            }
                            current->setSignal(1);
                            current->clearOutputs();
                            current->getInputs()[0]->removeOutput(current->getId());
                            current->getInputs()[1]->removeOutput(current->getId());
#if DEBUG >= 3
                            dump1<<"1 & 1:";
                            current->writeNode(dump1);
#endif
                        }
                        else if(first==2 && second==2)
                        {
                            current->setSignal(2);
#if DEBUG >= 3
                            dump1<<"2 & 2:";
                            current->writeNode(dump1);
#endif
                        }
                        //reconnecting
                        else if(first==2 || second==2)
                        {
#if DEBUG >= 3
                            dump1<<"Removing:";
                            current->writeNode(dump1);
#endif
                            current->setSignal(2);
                            if(first!=2)
                            {
#if DEBUG >= 3
                                dump1<<current->getInputs()[0]->getId()<<":1 and "<<current->getInputs()[1]->getId()<<":2"<<endl;
#endif
                                new_node=current->getInputs()[1];
                                pol_new_node=current->getInputPolarities()[1];
                            }
                            else if (second!=2)
                            {
#if DEBUG >= 3
                                dump1<<current->getInputs()[0]->getId()<<":2 and "<<current->getInputs()[1]->getId()<<":1"<<endl;
#endif
                                new_node=current->getInputs()[0];
                                pol_new_node=current->getInputPolarities()[0];
                            }
                            else
                                    cout<<"ERROR, this if statement should not be reached1."<<endl;

                            AUX=current->getOutputs();
                            for(int l=0;l<AUX.size();l++)
                            {
#if DEBUG >= 3
                                dump1<<"current's output before:";
                                AUX[l]->writeNode(dump1);
#endif
                                if(AUX[l]->getInputs()[0]->getId()==current->getId())
                                {
#if DEBUG >= 3
                                    dump1<<"(int)pol_new_node:"<<(int)pol_new_node<<". AUX[l]->getInputPolarities()[0]:"<<AUX[l]->getInputPolarities()[0]<<endl;
                                    dump1<<"(bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[0]):"<<((bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[0]))<<endl;
#endif
                                    polarity=(bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[0]);
                                    AUX[l]->replaceInput(0,new_node,polarity);
                                }
                                else if (AUX[l]->getInputs()[1]->getId()==current->getId())
                                {
#if DEBUG >= 3
                                    dump1<<"(int)pol_new_node:"<<(int)pol_new_node<<". AUX[l]->getInputPolarities()[1]:"<<AUX[l]->getInputPolarities()[1]<<endl;
                                    dump1<<"(bool)((int)pol_new_nodepolarity)^(AUX[l]->getInputPolarities()[1]):"<<((bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[1]))<<endl;
#endif
                                    polarity=(bool)((int)pol_new_node)^(AUX[l]->getInputPolarities()[1]);
                                    AUX[l]->replaceInput(1,new_node,polarity);
                                }
                                else
                                    cout<<"ERROR, this else statement should never be reached. (propagateAndDelete)"<<endl;
                                new_node->pushOutput(AUX[l]);
#if DEBUG >= 3
                                dump1<<AUX[l]->getId()<<" reciving new node:"<<new_node->getId()<<endl;
                                dump1<<"current's output after:";
                                AUX[l]->writeNode(dump1);
#endif
                            }
                            current->clearOutputs();
                            current->getInputs()[0]->removeOutput(current->getId());
                            current->getInputs()[1]->removeOutput(current->getId());
                            //treating if current to be removed is a PO
                            if(all_outputs.find(current->getId())!=all_outputs.end())
                            {
#if DEBUG >= 3                      
                                dump_PO<<"Renumbering output:"<<current->getId()<<" with:"<<new_node->getId()<<endl;
#endif
                                all_outputs.find(current->getId())->second.setId(new_node->getId());
                                polarity=((int)pol_new_node)^all_outputs.find(current->getId())->second.getInputPolarity();
                                all_outputs.find(current->getId())->second.pushInput(new_node,polarity);
                            }
                        }
                        else
                            dump1<<"ERROR, this if statement should not be reached2."<<endl;
                        }
                        visits[stackzin.top()->getId()/2]=1;
                        stackzin.pop();
                }
            }
        }
        if(it_out->second.getId()>1)
            it_out->second.setSignal(it_out->second.getInput()->getSignal());
        
        if(it_out->second.getSignal()!=2)
        {
            cout<<"WARNING: output "<<it_out->second.getId()<<" has signal after constant propagation:"<<it_out->second.getSignal()<<endl;
            dump_PO.open("dump_PO",ios::app); dump_PO<<"WARNING: output "<<it_out->second.getId()<<" has signal after constant propagation:"<<it_out->second.getSignal()<<endl; dump_PO.close();
        }
    }
    cout<<"Signal propagation doneeeeeeeeeeeeeeee."<<endl; 
    
    //making sure outputs wont be deleted, by adding themselfs to their fanout list
    cout<<"Making sure outputs wont be deleted, by adding themselfs to their fanout list"<<endl;
    for(it_out=this->all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
        if(it_out->second.getId()>1)
            it_out->second.getInput()->pushOutput(it_out->second.getInput());
    }
    
    //Setting ANDs that wasn't reached in the DFS to be removed.
    cout<<"Setting ANDs that wasn't reached in the DFS to be removed."<<endl;
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
    {
        if(it_and->second.getSignal()==-1)
        {
            it_and->second.clearOutputs();
            it_and->second.getInputs()[0]->removeOutput(it_and->second.getId());
            it_and->second.getInputs()[1]->removeOutput(it_and->second.getId());
        }
    }

    //Removing ANDs with 0 fanouts
    cout<<"Removing ANDs with 0 fanouts"<<endl;
    cout<<"graph depth:"<<this->graph_depth<<endl;
    int ands_removed=0,PIs_removed=0,id=0;
    vector<int> removed_nodes_counter_by_depth(graph_depth+1,0);
#if DEBUG >= 3
    dump1<<"graph depth:"<<this->graph_depth<<endl;
    dump1<<"removed_nodes_counter_by_depth size:"<<removed_nodes_counter_by_depth.size()<<endl;
#endif    
    it_and=all_ANDS.begin();
    while(it_and!=all_ANDS.end())
    {
        it_and->second.printNode();
        if(it_and->second.getOutputs().size()==0) //&& all_outputs.find(it_and->first)==all_outputs.end())
        {
#if DEBUG >= 1
            ofstream write2("removed_ANDs.txt"); 
            write2<<it_and->first<<",";
            dump1<<"AND:"<<it_and->second.getId()<<", depth:"<<this->all_depths[it_and->second.getId()/2]<<", depth counter:"<<removed_nodes_counter_by_depth[this->all_depths[it_and->second.getId()/2]]<<endl;
#endif
            
            removed_nodes_counter_by_depth[this->all_depths[it_and->second.getId()/2]]++;
            it_and=all_ANDS.erase(it_and);
            ands_removed++;
        }
        else
            it_and++;
    }   
    
    it_in=all_inputs.begin();
    //COUNTING inputs with 0 fanouts
    cout<<"COUNTING inputs with 0 fanouts"<<endl;
    while(it_in!=all_inputs.end())
    {
        if(it_in->second.getOutputs().size()==0)
            PIs_removed++;
        it_in++;
    }
#if REMOVE_PI ==1
    it_in=all_inputs.begin();
    //Removing inputs with 0 fanouts
    while(it_in!=all_inputs.end())
    {
        if(it_in->second.getOutputs().size()==0)
        {
            removed_inputs<<it_in->first<<endl;
            it_in=all_inputs.erase(it_in);
//            PIs_removed++;
        }
        else
            it_in++;
    }
#endif
    
#if DEBUG >= 1
    int sum=0;
    for(int t=0;t<removed_nodes_counter_by_depth.size();t++)
        sum+=removed_nodes_counter_by_depth[t];
    string file_name;
    file_name="Nodes_in_level.csv";
    ofstream nodes_per_level(file_name,ios::app);
//    dump1<<file_name<<":"<<sum<<","<<ands_removed<<endl;
    nodes_per_level<<this->name<<","<<th_value;
    for(int a=0;a<removed_nodes_counter_by_depth.size();a++)
        nodes_per_level<<","<<nodes_in_level[a]-removed_nodes_counter_by_depth[a];
    nodes_per_level<<endl;
    nodes_per_level.close();
#endif
    
    this->setDepthsInToOut();
//    cout<<"ANDs removed:"<<ands_removed<<endl;
//    cout<<"Inputs removed:"<<PIs_removed<<endl;
//    simpl_info<<"ANDs removed:"<<ands_removed<<endl;
//    simpl_info<<"Inputs removed:"<<PIs_removed<<endl;
//    simpl_info<<"all_ands.size():"<<all_ANDS.size()<<endl;
//    simpl_info<<"new depth:"<<this->graph_depth<<endl;
    
//    if(mnist_obj.getAllBits().size()==60000 && this->name.find("train")==string::npos)
//        this->name+="_train";
//    else if (mnist_obj.getAllBits().size()==10000 && this->name.find("test")==string::npos)
//        this->name+="_test";
//    else
//        cout<<"mnist size unknown"<<endl;
    
    ofstream csv_final;
    csv_final.open("todos_scores.csv",ios::app);
//    simpl_info<<endl<<th_value<<","<<PI_constant<<","<<PIs_removed<<","<<one_count<<","<<zero_count<<",,"<<ands_removed<<","<<all_ANDS.size()<<",,"<<graph_depth<<",,"<<endl;
    csv_final<<this->name<<","<<th_value<<","<<PI_constant<<","<<PIs_removed<<","<<one_count<<","<<zero_count<<",,"<<ands_removed<<","<<all_ANDS.size()<<",,"<<graph_depth<<",,,,";
#if APPLY_MNIST == 0
    csv_final<<endl;
#endif
csv_final.close();
#if DEBUG >= 3
    dump3<<"ANDs removed:"<<ands_removed<<endl;
    dump3<<"Inputs removed:"<<PIs_removed<<endl;
   dump3<<endl<<endl<<"Renunmbering Inputs:"<<endl;
#endif
#if RENUMBER == 1
    id=1;
   for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++,id++)
   {
#if DEBUG >= 3
       dump3<<it_in->first<<"==>"<<id*2<<endl;
#endif
       it_in->second.setId(id*2);
   }
   
#if DEBUG >= 3
   dump3<<endl<<endl<<"Renumbering ANDS:"<<endl;
#endif
   //Renumbering ANDs
   for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++,id++)
   {
#if DEBUG >= 3
       dump3<<it_and->first<<"==>"<<id*2<<endl;
#endif
       it_and->second.setId(id*2);
   }
   
#if DEBUG >= 3
   dump3<<endl<<endl;
#endif

   int constant_POs=0;
   //Renumbering POs
   for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
   {
       if(it_out->second.getId()>1)
       {
//#if DEBUG >= 3;
//        dump3<<it_out->second.getId()<<"==>"<<it_out->second.getInput()->getId()<<endl;
//#endif
        it_out->second.setId(it_out->second.getInput()->getId());
       }
       else
           constant_POs++;
   }
//   cout<<"Constant POs:"<<constant_POs<<endl;
#endif
}
//#endif
    
    //Reordering AND's inputs (bigger first)
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.invertInputs();

    if(LEAVE_CONSTANTS==1)
        this->name+="_WITH_CONSTANTS_";
    else
        this->name+="_NO_CONSTANTS_ANDs_removed_";
    if(FIX_DOUBLED_NODES==1)
        this->name+="FIXED_";
//   this->name+=to_string(1-threshold);
//#if WRITE_AIG == 1
//    cout<<"Writing output file (AIG):"<<this->name<<endl;
//    this->writeAIG();
//#endif
//#if WRITE_AAG == 1
//    cout<<"Writing output file (AAG):"<<this->name<<endl;
//    this->writeAAG();
//#endif
}



void aigraph::recursiveRemoveOutput(unsigned int id_to_remove, nodeAig* remove_from){
    for(int i=0;i<remove_from->getOutputs().size();i++)
    {
        if(remove_from->getOutputs()[i]->getId()==id_to_remove)
        {
            remove_from->removeOutput(id_to_remove);
//            remove_from->getOutputs().erase(remove_from->getOutputs().begin()+i);
            break;
        }
    }
    
//    if(all_inputs.find(remove_from->getId())==all_inputs.end())
    if(remove_from->getId()>(all_inputs.size()*2))
    {
        if(remove_from->getOutputs().size()==0 && all_outputs.find(remove_from->getId())==all_outputs.end())
        {
            if(remove_from->getInputs()[0]->getOutputs().size()>0)
                recursiveRemoveOutput(remove_from->getId(),remove_from->getInputs()[0]);
            if(remove_from->getInputs()[1]->getOutputs().size()>0)
                recursiveRemoveOutput(remove_from->getId(),remove_from->getInputs()[1]);
        }
    }
}

void aigraph::setDepthsInToOut(){
    cout<<"Setting AIG depths, NODE TO -->PI<--"<<endl;
    int retval,depth=0;
    map<unsigned int,output>::iterator it_out;
    map<unsigned int,AND>::iterator it_and;
    map<unsigned int,input>::iterator it_in;
    ofstream write;
    
    //Initializing depths
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        it_in->second.setDepth(-1);
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.setDepth(-1);
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.setDepth(-1);
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
#if DEBUG >= 3
    write.open("dumps/log2.txt",ios::app);
    write<<"OUTPUT BFS:"<<it_out->first<<endl;
#endif
        depth=it_out->second.computeDepthInToOut();
        it_out->second.setDepth(depth);
    }
    int greater=-1;
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
    {
        if(it_out->second.getDepth()>greater)
        {
            greater=it_out->second.getDepth();
            greatest_depths_ids.clear();
            greatest_depths_ids.push_back(it_out->second.getId());
        }
        else if (it_out->second.getDepth()==greater)
            greatest_depths_ids.push_back(it_out->second.getId());
    }
    this->graph_depth=greater;
    
#if DEBUG >= 3
    write.open("dumps/Depths.txt");
    write<<this->name<<","<<greater<<endl;
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        write<<it_and->second.getId()<<":"<<it_and->second.getDepth()<<endl;
    write.close();
#endif
    cout<<"setting depths done."<<endl;
}

void aigraph::setShortestDistanceToPO(){
    cout<<"Setting AIG depths, -->PO<--"<<endl;
    int depth=0;
    map<unsigned int,output>::iterator it_out;
    map<unsigned int,AND>::iterator it_and;
    map<unsigned int,input>::iterator it_in;
    ofstream write;
    //Initializing depths
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        it_in->second.setDepth(-1);
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        it_and->second.setDepth(-1);
    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        it_out->second.setDepth(-1);
    
    //THIS IS REQUIRED TO WORK PROPERLY
//    for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
//    {
//        it_out->second.computeDepthOutToIn(0);
//    }
    
    
    
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        cout<<it_and->second.getId()<<":"<<it_and->second.getDepth()<<endl;
    int greater=-1;
    for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
    {
        cout<<it_in->second.getId()<<":"<<it_in->second.getDepth()<<endl;
        if(it_in->second.getDepth()>greater)
        {
            greater=it_in->second.getDepth();
            greatest_depths_ids.clear();
            greatest_depths_ids.push_back(it_in->second.getId());
        }
        else if (it_in->second.getDepth()==greater)
            greatest_depths_ids.push_back(it_in->second.getId());
    }
    this->graph_depth=greater;
    cout<<"graph depth:"<<this->graph_depth<<endl;
    
    write.open("dumps/Depths.txt");
    write<<this->name<<","<<greater<<endl;
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        write<<it_and->second.getId()<<":"<<it_and->second.getDepth()<<endl;
    write.close();
}

void aigraph::printDepths(){
    map<unsigned int, AND>::iterator it;
    map<unsigned int, input>::iterator it2;
    map<unsigned int, output>::iterator it3;
    
    cout<<"Inputs' depths:"<<endl;
    for(it2=all_inputs.begin();it2!=all_inputs.end();it2++)
        cout<<it2->second.getId()<<":"<<it2->second.getDepth()<<endl;
    cout<<endl;
    
    cout<<"Outputs' depths:"<<endl;
    for(it3=all_outputs.begin();it3!=all_outputs.end();it3++)
        cout<<it3->second.getId()<<":"<<it3->second.getDepth()<<endl;
    cout<<endl;
                
    cout<<"ANDs' depths:"<<endl;
    for(it=all_ANDS.begin();it!=all_ANDS.end();it++)
        cout<<it->second.getId()<<":"<<it->second.getDepth()<<"|";//<<endl;
    cout<<endl;
}


void aigraph::writeProbsHistogram(){
    ANDs_probabilities.clear();
    map<unsigned int, AND>::iterator it_and;
    map<unsigned int, output>::iterator it_out;
    map<unsigned int,float>::iterator probs_it;
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        this->ANDs_probabilities.insert(pair<unsigned int,float>(it_and->second.getId(),0.0));
    string ands_probs_name,line;
    
    ands_probs_name="../ands_probs_";
    if(this->name.find("A1")!=string::npos)
        ands_probs_name+="A1.txt";
    else if(this->name.find("A2")!=string::npos)
        ands_probs_name+="A2.txt";
    else if(this->name.find("A3")!=string::npos)
        ands_probs_name+="A3.txt";
    else if(this->name.find("A4")!=string::npos)
        ands_probs_name+="A4.txt";
    else
        cout<<"ERROR, graph name has no A1-4"<<endl;
    ifstream in_file (ands_probs_name);
    
    it_and=all_ANDS.begin();
    vector<int> probs_concentration (12,0);
    for(probs_it=ANDs_probabilities.begin();probs_it!=ANDs_probabilities.end();probs_it++)
    {
        getline(in_file,line);
        line.erase(0,line.find(",")+1);
        probs_it->second=stof(line);
        if(probs_it->second==0)
            probs_concentration[0]++;
        if(probs_it->second>0 && probs_it->second<=0.1)
            probs_concentration[1]++;
        if(probs_it->second>0.1 && probs_it->second<=0.2)
            probs_concentration[2]++;
        if(probs_it->second>0.2 && probs_it->second<=0.3)
            probs_concentration[3]++;
        if(probs_it->second>0.3 && probs_it->second<=0.4)
            probs_concentration[4]++;
        if(probs_it->second>0.4 && probs_it->second<=0.5)
            probs_concentration[5]++;
        if(probs_it->second>0.5 && probs_it->second<=0.6)
            probs_concentration[6]++;
        if(probs_it->second>0.6 && probs_it->second<=0.7)
            probs_concentration[7]++;
        if(probs_it->second>0.7 && probs_it->second<=0.8)
            probs_concentration[8]++;
        if(probs_it->second>0.8 && probs_it->second<=0.9)
            probs_concentration[9]++;
        if(probs_it->second>0.9 && probs_it->second<1)
            probs_concentration[10]++;
        if(probs_it->second==1)
            probs_concentration[11]++;
    }
    
    string file_name="Prob(1)_histogram_";
    file_name+=this->name;
    file_name+=".csv";
    ofstream write(file_name);
    write<<"Prob Range,# Nodes"<<endl;
    write<<"0,"<<probs_concentration[0]<<endl<<"]0-10],"<<probs_concentration[1]<<endl<<"]10-20],"<<probs_concentration[2]<<endl<<"]20-30],"<<probs_concentration[3]<<endl<<"]30-40],"<<probs_concentration[4]<<endl<<"]40-50],"<<probs_concentration[5]<<endl<<"]50-60],"<<probs_concentration[6]<<endl<<"]60-70],"<<probs_concentration[7]<<endl<<"]70-80],"<<probs_concentration[8]<<endl<<"]80-90],"<<probs_concentration[9]<<endl<<"]90-100[,"<<probs_concentration[10]<<endl<<"100,"<<probs_concentration[11]<<endl;
    int total=0;
    for(int a=0;a<probs_concentration.size();a++)
        total+=probs_concentration[a];
    write<<"total sum:"<<total<<",all_ands size:"<<this->all_ANDS.size()<<endl;
    write.close();
    
    this->setDepthsInToOut();
    vector<int> depth_counter (this->graph_depth,0);
    for(it_and=all_ANDS.begin();it_and!=all_ANDS.end();it_and++)
        depth_counter[it_and->second.getDepth()]++;
    
    file_name="Depth_histogram_";
    file_name+=this->name;
    file_name+=".csv";
    ofstream write2(file_name);
    for(int a=0;a<depth_counter.size();a++)
        write2<<a<<","<<depth_counter[a]<<endl;
    write2.close();
}

void aigraph::evaluateScorseAbcCommLine21(int ds_start,int ds_end){
//    string cifar_path=fs::current_path();
#if cifarv2 != 1
    string my_path="../../data/";
#else
    string my_path="../../data/";
#endif
    string cifar_path="./";
    cifar_path=cifar_path+my_path;
    int ANDs_size=0, correct_count=0; string line;
    ifstream input_file;
    string abc_aigs_path="abc_stuff/";
    system(("rm "+abc_aigs_path+"temp.aig").c_str());
    {
        this->writeAIG("./"+abc_aigs_path,"temp.aig");
        ANDs_size=0; float correct_count=0;
        string cifar_full_name;
#if cifarv2 != 1
        for (int q=ds_start;q<=ds_end;q++)
        {
            cifar_full_name=cifar_path+"data_batch_"+to_string(q+1)+".bin";
#else
        {
            int q=ds_start;
            if(ds_start==ds_end){
                cifar_full_name=cifar_path+"mini_test_batch.bin";
                #if COUT >= 1
                cout<<"Using MINI TEST SET! path:"<<cifar_full_name<<endl;
                #endif
            }
            else{
                #if local_test_run != 1
                cifar_full_name=cifar_path+"red_data_batch.bin";
                #else
                cifar_full_name="../../data/data_batch_1.bin";
                #endif
                #if COUT >= 1
                cout<<"Using reduced TRAIN SET (without mini test)! path:"<<cifar_full_name<<endl;
                #endif
                
            }
#endif
//            cout<<"full name:"<<full_name<<endl;
            abcCall21("./"+abc_aigs_path, "temp",cifar_full_name);
            //reading ABC output
            input_file.close();
            input_file.open(abc_aigs_path+"logAbc.txt");
            string aux;
            while(getline(input_file,line))
            {
                if(q==ds_start && line.find("and = ")!=string::npos)
                {
                    aux=line;
                    aux.erase(0,aux.find("and = ")+6);
                    aux.erase(aux.find("lev")-9,aux.back());
                    ANDs_size+=atoi(aux.c_str());
//                    cout<<"---------ANDS string:"<<aux<<"!!!"<<endl;
                }
                if(line.find("Correct =")!=string::npos)
                {
//                    cout<<"---------score string:"<<line<<endl;
                    line.erase(0,line.find("Correct =")+9);
//                    cout<<"---------score string:"<<line<<endl;
                    line.erase(line.find(". ("),line.size());
//                    cout<<"--------count string:"<<line<<endl;
                    correct_count+=atoi(line.c_str());
                }
            }
        }
            this->size=ANDs_size;
#if cifarv2 != 1
        this->train_score=correct_count/((ds_end-ds_start+1)*10000);
#else
        if(ds_start==ds_end)
            this->test_score=correct_count/(1500);
        else
            #if local_test_run != 1
            this->train_score=correct_count/(48500);
            #else
            this->train_score=correct_count/(10000);
            #endif
#endif
        
//#if COUT >=1
        cout<<"Evaluation with ABC, test score "<<this->test_score<<", train score"<<this->train_score<<", size:"<<this->size<<endl;
//#endif
    }
}
    
float aigraph::getTestScore(){
    return this->test_score;
}

float aigraph::getTrainScore(){
    return this->train_score;
}

int aigraph::getSize(){
    return this->size;
}

int aigraph::getPiConstantsSize(){
    return this->PIs_constant;
}

int aigraph::getAndsConstantsSize(){
    return this->ANDs_constant;
}
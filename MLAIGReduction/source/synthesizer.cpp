/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <typeinfo>

#include "AIG.h"

//
synthesizer::synthesizer(){}

synthesizer::~synthesizer(){}

void synthesizer::create(int num_PI,int function, bool balance,int enumeration){
    unsigned int cell_input;
    int start,stop;
    
    //circuit parameters and variables
    I=num_PI;  
    L=0; 
    O=1; //TODO: next version should accpet more outputs.
    A=0;
    AND_index=I;
    
#if DEBUG>=3
    log.open("memory info",ios::trunc);
    log<<"#Node \t\t\t\t Max mem(KB) \t\t\t Virtual mem(KB) \t\t\t ru_stime \t\t\t ru_utime"<<endl;
#endif
    
    

    struct rusage buf; 
    if(getrusage(RUSAGE_SELF,&buf)==-1)
        cout<<"GETRUSAGE FAILURE!"<<endl;
    start=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;

   //instantiate inputs and add to deque (FIFO).
    for(int i=I;i>0;i--)
    {
        input input_obj(i*2);
        circ_deque.push_back(this->pushPI(i*2,input_obj));
    }


    cell_input=num_PI;
    if(function==1)
        addAND(cell_input,balance);
    else if(function==2)
        addNAND(cell_input,balance);
    else if(function==3)
        addOR(cell_input,balance);
    else if(function==4)
        addNOR(cell_input,balance);
    else if(function==5)
        addXOR(cell_input,balance);
    else if(function==6)
        addXNOR(cell_input,balance);

  
    //Instatiate the circuit OUTPUT
    output output_obj(circ_deque.back()->fixLSB()->getId());
    output_obj.pushInput(circ_deque.back()->fixLSB(),((int)getThisPtrPolarity(circ_deque.back())));
//    cout<<"CIRC_DEQUE.back():"<<circ_deque.back()->fixLSB()->getId()+(int)getThisPtrPolarity(circ_deque.back())<<endl;
    this->pushPO(circ_deque.back()->fixLSB()->getId(),output_obj);
    
    if(getrusage(RUSAGE_SELF,&buf)==-1)
     cout<<"GETRUSAGE FAILURE!"<<endl;
    stop=buf.ru_stime.tv_sec+buf.ru_utime.tv_sec;

    circ_deque.clear();
    A=AND_index-I;
    M=I+L+A;
    
    //setting circuit name
    string circuit_name;
    if(function==1)
        circuit_name="AND_";
    else if(function==2)
        circuit_name="NAND_";
    else if(function==3)
        circuit_name="OR_";
    else if(function==4)
        circuit_name="NOR_";
    else if(function==5)
        circuit_name="XOR_";
    else if(function==6)
        circuit_name="XNOR_";
        
    circuit_name.append(to_string(M+3));
    if(balance)
        circuit_name.append("_bal_");
    else
        circuit_name.append("_unbal_");
    
    circuit_name.append(to_string(stop-start));
    this->setName(circuit_name);
    
    cout<<"Circuit structured succesfully."<<endl;
#if DEBUG==1
    cout<<"M:"<<M<<",I:"<<I<<",L:"<<L<<",O:"<<O<<",A:"<<A<<endl;
    cout<<"M:"<<M<<",I:"<<all_inputs.size()<<",L:"<<all_latches.size()<<",O:"<<all_outputs.size()<<",A:"<<all_ANDS.size()<<endl;
#endif
    
    //Handling nodes' enumeration
    enumerateGraph(enumeration);
    
    
            
    //write output file
    //writeAAG();
    if(enumeration==0)
        writeAIG();
    
    else
    {
    
#if DEBUG>=3
    struct rusage buf; 
    if(getrusage(RUSAGE_SELF,&buf)==-1)
                cout<<"GETRUSAGE FAILURE!"<<endl;
    log<<"After clear:"<<buf.ru_maxrss<<endl;
#endif
    
    writeAAG();
    ifstream read( "OUT.aag", std::ifstream::in );
    readAAG(read,"OUT2.aag");
    writeAIG();
    
    this->clearCircuit();
#if DEBUG>=3
    if(getrusage(RUSAGE_SELF,&buf)==-1)
                cout<<"GETRUSAGE FAILURE!"<<endl;
    log<<"After clear:"<<buf.ru_maxrss<<endl;
    
    log.close();
#endif
    }

}



void synthesizer::addAND(int num_inputs, bool balance){
    deque<nodeAig*> cell_deque;
    cell_deque=buildCellDeque(num_inputs,false);
    struct rusage buf; 

    AND* AND_aux;
    if(balance)
    {
        while(cell_deque.size()>1)
        {
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);
//            system("free -m | awk '{print $3}' | sed 's/free//g' | tr '\n' ' ' >> log.txt");
//            write_log<<AND_index<<endl;
            
#if DEBUG >= 3
            if(getrusage(RUSAGE_SELF,&buf)==-1)
                cout<<"GETRUSAGE FAILURE!"<<endl;
            
            log.clear();
            if(AND_index % 1000 ==0)
                log<<AND_index<<"\t\t\t\t"<<buf.ru_maxrss<<"\t\t\t\t"<<getValue()<<"\t\t\t\t"<<buf.ru_stime.tv_sec<<"\t\t\t\t"<<buf.ru_utime.tv_sec<<endl;
            log.clear();
#endif
            
            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;
        }
    }
    else
    {
        while(cell_deque.size()>1)
        {
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;
        }
    }
    
    circ_deque.push_back(AND_aux);
        
}

void synthesizer::addNAND(int num_inputs,bool balance){

    deque<nodeAig*> cell_deque;
    cell_deque=buildCellDeque(num_inputs,false);

//    cout<<"CELL_DEQUE BEFORE:";
//        for(int x=0;x<cell_deque.size();x++)
//            cout<<cell_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(cell_deque[x])<<"|";
//        cout<<endl;
    AND* AND_aux;
    if(balance)
    {
        while(cell_deque.size()>1)
        {
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;
        }
    }
    else
    {
        while(cell_deque.size()>1)
        {
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;
        }
    }
        
//        cout<<"CELL_DEQUE AFTER:";
//        for(int x=0;x<cell_deque.size();x++)
//            cout<<cell_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(cell_deque[x])<<"|";
//        cout<<endl;
        
    
    //NAND output is negated.
//    AND_aux->setId(AND_aux->getId()+1);
    circ_deque.push_back(AND_aux->forceInvert());
//    cout<<"OUTPUT:"<<AND_aux->getId()+(int)getThisPtrPolarity(AND_aux->forceInvert())<<endl;
}


void synthesizer::addOR(int num_inputs,bool balance){
    deque<nodeAig*> cell_deque;
    cell_deque=buildCellDeque(num_inputs,true);
    
    AND* AND_aux;
    if(balance)
    {
        while(cell_deque.size()>1)
        {
            bool polarity;
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;

        }
    }
    else
    {
        while(cell_deque.size()>1)
        {
            bool polarity;
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;
        }
    }
    //OR output is negated.
    circ_deque.push_back(AND_aux->forceInvert());
}


void synthesizer::addNOR(int num_inputs,bool balance){
  deque<nodeAig*> cell_deque;
    cell_deque=buildCellDeque(num_inputs,true);
    
    AND* AND_aux;
    if(balance)
    {
        while(cell_deque.size()>1)
        {
            bool polarity;
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            connectNodes(cell_deque.front(),AND_ptr,false);
            cell_deque.pop_front();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;

        }
    }
    else
    {
        while(cell_deque.size()>1)
        {
            bool polarity;
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            connectNodes(cell_deque.back(),AND_ptr,false);
            cell_deque.pop_back();

            cell_deque.push_back(AND_ptr);
            AND_aux=AND_ptr;
        }
    }
    //NOR output is regular
    circ_deque.push_back(AND_aux);
}

void synthesizer::addXOR(int num_inputs,bool balance){
    deque<nodeAig*> cell_deque,aux_deque;
    aux_deque=buildCellDeque(num_inputs,false);
    
//    cout<<"AUX_DEQUE BEFORE:";
//    for(int x=0;x<aux_deque.size();x++)
//        cout<<aux_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(aux_deque[x])<<"|";
//    cout<<endl;

    cell_deque.push_back(aux_deque.front());
    aux_deque.pop_front();
    cell_deque.push_back(aux_deque.front());
    aux_deque.pop_front();
    


//    cout<<"CELL_DEQUE BEFORE:";
//        for(int x=0;x<cell_deque.size();x++)
//            cout<<cell_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(cell_deque[x])<<"|";
//        cout<<endl;
    AND* AND_aux;
    
    while(cell_deque.size()>1)
    {
//        if(!balance)
//        {
            AND_index++;
            AND AND_obj(AND_index*2);
            AND* AND_ptr;
            AND_ptr=this->pushAnd(AND_index*2,AND_obj);

            AND_index++;
            AND AND_obj2(AND_index*2);
            AND* AND_ptr2;
            AND_ptr2=this->pushAnd(AND_index*2,AND_obj2);

    //        cout<<cell_deque.front()->fixLSB()->getId()<<":"<<polarity<<endl;
    //        cout<<"fora do metodo:"<<cell_deque.front()b->fixLSB()->getId()<<"->"<<polarity<<endl;
            connectNodes(cell_deque.front(),AND_ptr,false);
            connectNodes(cell_deque.front(),AND_ptr2,true);
            cell_deque.pop_front();

    //        cout<<cell_deque.front()->fixLSB()->getId()<<":"<<polarity<<endl;
    //        cout<<"fora do metodo:"<<cell_deque.front()->fixLSB()->getId()<<"->"<<polarity<<endl;
            connectNodes(cell_deque.front(),AND_ptr,false);
            connectNodes(cell_deque.front(),AND_ptr2,true);
            cell_deque.pop_front();


            cell_deque.push_back(AND_ptr);
            cell_deque.push_back(AND_ptr2);

            AND_index++;
            AND AND_obj3(AND_index*2);
            AND* AND_ptr3;
            AND_ptr3=this->pushAnd(AND_index*2,AND_obj3);

            connectNodes(cell_deque.front(),AND_ptr3,true);
            cell_deque.pop_front();
            connectNodes(cell_deque.front(),AND_ptr3,true);
            cell_deque.pop_front();

            if(!balance)
            {
                cell_deque.push_back(AND_ptr3);

                if(aux_deque.size()>=1)
                {
                    cell_deque.push_back(aux_deque.front());
                    aux_deque.pop_front();
                }
            }
            else
            {
                aux_deque.push_back(AND_ptr3);
                if(aux_deque.size()>=1)
                {
                    cell_deque.push_back(aux_deque.front());
                    aux_deque.pop_front();
                }
                if(aux_deque.size()>=1)
                {
                    cell_deque.push_back(aux_deque.front());
                    aux_deque.pop_front();
                }
            }

            AND_aux=AND_ptr3;
    //        cout<<"AAA"<<endl;
    //        cout<<"CELL_DEQUE AFTER:";
    //        for(int x=0;x<cell_deque.size();x++)
    //            cout<<cell_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(cell_deque[x])<<"|";
    //        cout<<endl;
//        }

    }
    circ_deque.push_back(AND_aux);
}


void synthesizer::addXNOR(int num_inputs,bool balance){
    deque<nodeAig*> cell_deque,aux_deque;
    for(int i=0;i<num_inputs;i++)
    {
        aux_deque.push_back(circ_deque.front());
        circ_deque.pop_front();
    }
//    cout<<"AUX_DEQUE BEFORE:";
//    for(int x=0;x<aux_deque.size();x++)
//        cout<<aux_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(aux_deque[x])<<"|";
//    cout<<endl;
    
    cell_deque.push_back(aux_deque.front());
    aux_deque.pop_front();
    cell_deque.push_back(aux_deque.front());
    aux_deque.pop_front();
    


//    cout<<"CELL_DEQUE BEFORE:";
//        for(int x=0;x<cell_deque.size();x++)
//            cout<<cell_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(cell_deque[x])<<"|";
//        cout<<endl;
    AND* AND_aux;
    
    while(cell_deque.size()>1)
    {
        
        AND_index++;
        AND AND_obj(AND_index*2);
        AND* AND_ptr;
        AND_ptr=this->pushAnd(AND_index*2,AND_obj);
        
        AND_index++;
        AND AND_obj2(AND_index*2);
        AND* AND_ptr2;
        AND_ptr2=this->pushAnd(AND_index*2,AND_obj2);

//        cout<<cell_deque.front()->fixLSB()->getId()<<":"<<polarity<<endl;
//        cout<<"fora do metodo:"<<cell_deque.front()->fixLSB()->getId()<<"->"<<polarity<<endl;
        connectNodes(cell_deque.front(),AND_ptr,false);
        connectNodes(cell_deque.front(),AND_ptr2,true);
        cell_deque.pop_front();
        
//        cout<<cell_deque.front()->fixLSB()->getId()<<":"<<polarity<<endl;
//        cout<<"fora do metodo:"<<cell_deque.front()->fixLSB()->getId()<<"->"<<polarity<<endl;
        connectNodes(cell_deque.front(),AND_ptr,false);
        connectNodes(cell_deque.front(),AND_ptr2,true);
        cell_deque.pop_front();

        
        cell_deque.push_back(AND_ptr);
        cell_deque.push_back(AND_ptr2);
        
        AND_index++;
        AND AND_obj3(AND_index*2);
        AND* AND_ptr3;
        AND_ptr3=this->pushAnd(AND_index*2,AND_obj3);
        
        connectNodes(cell_deque.front(),AND_ptr3,true);
        cell_deque.pop_front();
        connectNodes(cell_deque.front(),AND_ptr3,true);
        cell_deque.pop_front();
        
        if(!balance)
        {
            cell_deque.push_back(AND_ptr3);

            if(aux_deque.size()>=1)
            {
                cell_deque.push_back(aux_deque.front());
                aux_deque.pop_front();
            }
        }
        else
        {
            aux_deque.push_back(AND_ptr3);
            if(aux_deque.size()>=1)
            {
                cell_deque.push_back(aux_deque.front());
                aux_deque.pop_front();
            }
            if(aux_deque.size()>=1)
            {
                cell_deque.push_back(aux_deque.front());
                aux_deque.pop_front();
            }
        }
        
        AND_aux=AND_ptr3;
//        cout<<"AAA"<<endl;
//        cout<<"CELL_DEQUE AFTER:";
//        for(int x=0;x<cell_deque.size();x++)
//            cout<<cell_deque[x]->fixLSB()->getId()+(int)getThisPtrPolarity(cell_deque[x])<<"|";
//        cout<<endl;
    }
    circ_deque.push_back(AND_aux->forceInvert());
}



void synthesizer::connectNodes(nodeAig* in, nodeAig* destination, bool invert){
    bool polarity;
    in->fixLSB()->pushOutput(destination);
    polarity=getThisPtrPolarity(in);
//    cout<<"ID:"<<in->fixLSB()->getId()+(int)getThisPtrPolarity(in)<<endl;
//    cout<<"before(invert^polarity):"<<invert<<"^"<<polarity<<endl;
    polarity=invert^polarity;
//    cout<<"after:::::::::::::::::::"<<polarity<<endl;
//    cout<<"node "<<in->fixLSB()->getId()<<" recieves pol:"<<polarity<<" on destination "<<destination->getId()<<endl;
    destination->pushInput(in->fixLSB(),polarity);
}


deque<nodeAig*> synthesizer::buildCellDeque(int num_inputs,bool input_inverted){
    deque<nodeAig*> cell_deque;
    if(!input_inverted)
    {
        for(int i=0;i<num_inputs;i++)
        {
            cell_deque.push_back(circ_deque.front());
            circ_deque.pop_front();
        }
    }
    else
    {
        for(int i=0;i<num_inputs;i++)
        {
            cell_deque.push_back(circ_deque.front()->forceInvert());
            circ_deque.pop_front();
        }
    }
    return cell_deque;
}



void synthesizer::enumerateGraph(int enumeration){

    deque<nodeAig*> aux_deque;
    deque<nodeAig*> stack;
    set<std::reference_wrapper<nodeAig*> > s;
    map<unsigned int,output>::iterator it_out;
    map<unsigned int,input>::iterator it_in;
    unsigned int index=I;
    
    if(enumeration!=0)
        clearIDs();

    //1 for DFS starting from outputs
    if(enumeration==1)
    {
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
        {
            aux_deque.push_back(it_out->second.getInput());
            while(!aux_deque.empty())
            {
                if(aux_deque.back()->getInputs()[0]->getId()!=0 && aux_deque.back()->getInputs()[1]->getId()!=0)
                {
                    index++;
                    aux_deque.back()->setId(index*2);
                    aux_deque.pop_back();
                }
                else if(aux_deque.back()->getInputs()[0]->getId()==0)
                    aux_deque.push_back(aux_deque.back()->getInputs()[0]);
                else if(aux_deque.back()->getInputs()[1]->getId()==0)
                    aux_deque.push_back(aux_deque.back()->getInputs()[1]);
                
            }
            it_out->second.setId(it_out->second.getInput()->getId());
        }
    }


    //2 for BFS starting from outputs
    else if (enumeration==2)
    {
        for(it_in=all_inputs.begin();it_in!=all_inputs.end();it_in++)
        {
            for(int a=0;a<it_in->second.getOutputs().size();a++)
            {
                //if both intputs of the node are PIs, push the node to the queue.
                if(it_in->second.getOutputs()[a]->getInputs()[0]->getId()!=0 && it_in->second.getOutputs()[a]->getInputs()[1]->getId()!=0)
                {
                    if (s.find(it_in->second.getOutputs()[a]) == s.end())
                    {
                        aux_deque.push_back(it_in->second.getOutputs()[a]);
                        s.insert(std::ref(aux_deque.back())); 
                    }
                }
            }
//            aux_deque.push_back(it_in->second);
        }
        
        while(!aux_deque.empty())
        {
            index++;
            aux_deque.front()->setId(index*2);
            for(int b=0;b<aux_deque.front()->getOutputs().size();b++)
            {
                if(aux_deque.front()->getOutputs()[b]->getInputs()[0]->getId()!=0 && aux_deque.front()->getOutputs()[b]->getInputs()[1]->getId()!=0)
                if (s.find(aux_deque.front()->getOutputs()[b]) == s.end())
                {
                    aux_deque.push_back(aux_deque.front()->getOutputs()[b]);
                    s.insert(std::ref(aux_deque.back())); 
                }
            }
            aux_deque.pop_front();
        }
        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
            it_out->second.setId(it_out->second.getInput()->getId());
    }
//        for(it_out=all_outputs.begin();it_out!=all_outputs.end();it_out++)
//        {
//            aux_deque.push_back(it_out->second.getInput());
//            while(!aux_deque.empty())
//            {
//                if(aux_deque.front()->getInputs()[0]->getId()!=0 && aux_deque.front()->getInputs()[1]->getId()!=0)
//                {
//                    index++;
//                    aux_deque.front()->setId(index*2);
//                    aux_deque.pop_front();
//                }
//                else
//                {
//                    if(aux_deque.front()->getInputs()[0]->getId()==0)
//                        aux_deque.push_back(aux_deque.front()->getInputs()[0]);
//                    if(aux_deque.front()->getInputs()[1]->getId()==0)
//                        aux_deque.push_back(aux_deque.front()->getInputs()[1]);
//
//                    stack.push_back(aux_deque.front());
//                    aux_deque.pop_front();
//                }
//            }
//            while(!stack.empty())
//            {
//                index++;
//                stack.back()->setId(index*2);
//                stack.pop_back();
//            }
//            it_out->second.setId(it_out->second.getInput()->getId());
//        }

}

void synthesizer::clearIDs(){
    for(map<unsigned int,output>::iterator it2=all_outputs.begin();it2!=all_outputs.end();it2++)
        it2->second.setId(0);
    
    //write ANDs
    for(map<unsigned int,AND>::iterator it3=all_ANDS.begin();it3!=all_ANDS.end();it3++)
        it3->second.setId(0);
    
    
}





int synthesizer::parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int synthesizer::getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

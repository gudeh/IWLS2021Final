/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mnist.cpp
 * Author: augusto
 * 
 * Created on 24 de Janeiro de 2019, 14:16
 */

#include "binaryDS.h"
#include "AIG.h"


using namespace std;
binaryDS::binaryDS() {
}

binaryDS::binaryDS(const binaryDS& orig) {
}

binaryDS::~binaryDS() {
}


void binaryDS::readIdx(ifstream& file,string imgs_name){
    string labels_name=imgs_name;
    labels_name.replace(labels_name.find_first_of("-"),7,"-labels");
    int char_count=0;
#if MNIST_DS != 1
    file.close();
#if local_test_run != 1
    file.open("../../data/red_data_batch.bin",ifstream::binary);
#else
    file.open("../../data/data_batch_1.bin",ifstream::binary);
//    file.open("../../data/mini_test_batch.bin",ifstream::binary);
#endif
#endif
    cout<<endl<<"Reading dataset file "<<endl;
    if(file.is_open())
        cout<<"Dataset file is opened!"<<endl;
    else
        cout<<"Dataset file NOT OPENED!!!!"<<endl;
    
    #if MNIST_DS == 1  
    labels_name.at(labels_name.find_last_of("3"))='1';
    cout<<"Labels file:     "<<labels_name<<endl;
    file.seekg(16); //jumping the header line    
    #endif
    
    int line,column,byte;
//    vector< vector<int> > counter(28, vector<int>(224));
    vector< vector<int> > counter(posY_max, vector<int>(posX_max));
    char c;
    

    line=0;
    column=0;
    for(int a=0;a<posY_max;a++)
    {
        for(int b=0;b<posX_max;b++)
            counter[a][b]=0;
    }
    while(file.get(c))
    {
        #if MNIST_DS != 1
        if(char_count==0){
            int num=0;
            num=(int)c;
            if(num<0)
                num=num+256;
            #if DEBUG >=4
            bitset<8>b(c);
            cout<<"label:"<<num<<"("<<b<<")"<<endl;
            #endif  
            labels.push_back(num);
        }
        else 
        #endif
        {
            byte=(int)c;
            if(byte<0)
                byte=byte+256;
            int bit;
    #if DEBUG >=4
            cout<<"byte:"<<byte<<":";
    #endif
//#if MNIST_DS != 1
//            for(int k=7; k>=0; k--)
//#elif
            for(int k=0;k<=7;k++)
//#endif
            {
                bit=((byte & ( 1 << k )) >> k);

    #if DEBUG >=4
            cout<<bit<<",";
    #endif
            if(bit==1)
                counter[line][column]=1;
                column++;
                if(column==posX_max){
                    column=0;
                    line++;
                    if(line==posY_max){
                        column=0;
                        line=0;
                        all_bits.push_back(counter);
                        for(int a=0;a<posY_max;a++){
                            for(int b=0;b<posX_max;b++)
                                counter[a][b]=0;
                        }
                    }
                }
            }
#if DEBUG >=4
//            cout<<endl;
            cout<<"|||";
#endif
        }
        #if MNIST_DS != 1
        char_count++;
        if (char_count==((posY_max*posX_max)/8)+1){
            char_count=0;
        }
        #endif
    }
    
#if DEBUG >=4
    int contador=0;
    for(int a=0;a<all_bits.size();a++)
    {
        for(int b=0;b<all_bits[a].size();b++)
        {
            for(int c=0;c<all_bits[a][b].size();c++)
            {
                contador++;
                cout<<contador<<":"<<all_bits[a][b][c]<<"\t";
            }
            cout<<endl;
        }
        cout<<endl;
    }
#endif
#if DEBUG>=4
    cout<<"all_bits size:"<<all_bits.size()<<endl;
    cout<<"all_bits[0] size:"<<all_bits[0].size()<<endl;
    cout<<"all_bits[0][0] size:"<<all_bits[0][0].size()<<endl;
    cout<<"all_bits[1] size:"<<all_bits[1].size()<<endl;
    cout<<"all_bits[1][0] size:"<<all_bits[1][0].size()<<endl;
    cout<<"all_bits[2] size:"<<all_bits[2].size()<<endl;
    cout<<"all_bits[2][0] size:"<<all_bits[2][0].size()<<endl;
    cout<<"all_bits[3] size:"<<all_bits[3].size()<<endl;
   cout<<"all_bits[3][0] size:"<<all_bits[3][0].size()<<endl;
   cout<<"Last line in last image:"; 
   for(int a=0;a<all_bits.back().back().size();a++)
        cout<<all_bits.back().back()[a]; cout<<endl;
#endif
    
    
    
    
#if MNIST_DS == 1
   cout<<"Processing LABELS "<<labels_name<<endl;
    ifstream file_label(labels_name, ios::binary);
    file_label.seekg(8);
    int num=-1;
    while(file_label.get(c))
    {
        num=(int)c;
//        cout<<num<<",";
        labels.push_back(num);
    }    
#endif
    
#if DEBUG >=4
    cout<<"labels size:"<<labels.size()<<endl;
    cout<<"labels[0]:"<<labels[0]<<endl;
    cout<<"labels[1]:"<<labels[1]<<endl;
#endif
    
    
}


void binaryDS::setPIsBitsProbabilities(ifstream& file){
    cout<<"Setting input signals probabilities."<<endl;
     char c;
    int line,column,byte;
    int counter[posY_max][posX_max]; //28*8=224
    
    
    for(line=0;line<posY_max;line++)
    {
        for(column=0;column<posX_max;column++)
        {
            counter[line][column]=0;
        }
    }
//    if(file.is_open())
        
    line=0;
    column=0;
    file.clear();
#if MNIST_DS == 1 
    file.seekg(16); //jumping the header line
#else
    file.close();
#if local_test_run != 1
    file.open("../../data/red_data_batch.bin",ifstream::binary);
#else
    file.open("../../data/data_batch_1.bin",ifstream::binary);
//    file.open("../../data/mini_test_batch.bin",ifstream::binary);
#endif
#endif
    while(file.get(c))
    {
        byte=(int)c;
        if(byte<0)
            byte=byte+256;
        int bit;
//#if MNIST_DS != 1
//            for(int k=7; k>=0; k--)
//#elif
            for(int k=0;k<=7;k++)
//#endif
        {
            bit=((byte & ( 1 << k )) >> k);

        
        if(bit==1)
            counter[line][column]++;
        
            column++;
            if(column==posX_max)
            {
                column=0;
                line++;
                if(line==posY_max)
                {
                    column=0;
                    line=0;
                }
            }
        }
//    }
    }

    vector <float> aux;
    for(line=0;line<posY_max;line++)
    {
        aux.clear();
        for(column=0;column<posX_max;column++)
        {
//            aux.push_back((float)counter[line][column]/60000);
            aux.push_back((float)counter[line][column]/all_bits.size());
//            cout<<(float)counter[line][column]/10000<<" == "<<(float)counter[line][column]/all_bits.size()<<endl;
        }
        
        this->input_probabilities.push_back(aux);
//        cout<<line<<endl;
    }
    cout<<"input_probabilities.size():"<<input_probabilities.size()<<endl;
    cout<<"input_probabilities[0].size():"<<input_probabilities[0].size()<<endl;
    
    
#if DEBUG >=4
    ofstream out2("idVsprobs");
    for (int i=0;i<input_probabilities.size();i++)
    {
        for(int j=0;j<input_probabilities[i].size();j++)
            out2<<((((i)*(posX_max)) +j+1)*2)<<":"<<input_probabilities[i][j]<<endl;
    }
#endif
    
#if DEBUG >=1
    ofstream out3("PI_probs_density.csv");
    for (int i=0;i<input_probabilities.size();i++)
    {
        for(int j=0;j<input_probabilities[i].size();j++)
            out3<<input_probabilities[i][j]<<",";
        out3<<endl;
    }
#endif
}


int binaryDS::getBit(int img_index, int posY, int posX){
    return (bool)this->all_bits[img_index][posY][posX];
}



vector <vector <vector <int> > > binaryDS::getAllBits(){
    return this->all_bits;
}


int binaryDS::getLabel(int param){
    return this->labels[param];
}

vector<vector<float> > binaryDS::getPIsProbabilities() {
    return this->input_probabilities;
}


 void binaryDS::clearMnist(){
     this->all_bits.clear();
//     this->input_probabilities.clear();
     this->labels.clear();
 }
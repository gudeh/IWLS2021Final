/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CIFAR.cpp
 * Author: gudeh
 * 
 * Created on May 30, 2021, 7:03 PM
 */

#include "CIFAR.h"

CIFAR::CIFAR() {
}

CIFAR::CIFAR(const CIFAR& orig) {
}

CIFAR::~CIFAR() {
}


void CIFAR::readCifar(string dataset_path){
    this->name=dataset_path;
    this->PI_size=3072*8;
    ifstream file;
    string full_name;
    for (int q=0;q<1;q++)
    {
        full_name.clear();
        full_name=dataset_path+"data_batch_"+to_string(q+1)+".bin";
        file.open(full_name,ifstream::binary);
        cout<<endl<<"Reading CIFAR file "<<full_name<<endl;
        if(file.is_open())
            cout<<"File is opened!"<<endl;


        int line,column,byte;
        vector<bool> counter;
        vector<bool> label(10);
        char c;

        line=0;
        column=0;
        while(file.get(c))
        {     
            label[int(c)]=1;
//            cout<<"label:"<<int(c)<<"->";
//            for(int x=0;x<label.size();x++)
//                cout<<label[x]<<","; cout<<endl;
            for(int f=0;f<PI_size/8;f++)
            {
                file.get(c);
//                cout<<c<<",";
//                cout<<int(c)<<",";
                byte=(int)c;

                if(byte<0)
                    byte=byte+256;
                int bit;
                for(int k=7; k>=0; k--)
    //            for(int k=0;k<=7;k++)
                {
                    bit=((byte & ( 1 << k )) >> k);
//                    cout<<bit<<",";
                    counter.push_back((bool)bit);
                }
//                cout<<endl;SS
//                cout<<counter.size()<<endl;
            }
            all_images.push_back(counter);
            counter.clear();
            label_one_hot.push_back(label);
            label_int.push_back((unsigned long)c);
            label.clear(); label.resize(10);
//            cout<<"inside all_images size:"<<all_images.size()<<endl;            
        }
        file.close();
    }
    cout<<"all_images size:"<<all_images.size()<<endl;
}

int CIFAR::getPiSize(){
    return this->PI_size;
}

int CIFAR::getNumImages(){
    return this->all_images.size();
}

vector<vector<bool> >* CIFAR::getImages(){
    return &this->all_images;
}

vector<vector<bool> >* CIFAR::getLabels(){
    return &this->label_one_hot;
}

vector<unsigned long>* CIFAR::getLabelInt(){
    return &this->label_int;
}
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CIFAR.h
 * Author: gudeh
 *
 * Created on May 30, 2021, 7:03 PM
 */

#ifndef CIFAR_H
#define CIFAR_H


#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <bitset>
#include <algorithm>
#include "dataSet.h"
using namespace std;

class CIFAR : public dataSet{
private:
//    string name;
    int PI_size;
    vector<vector <bool> > all_images; //analog to all_combinations in PLA format
    vector<vector <bool> > label_one_hot; //inside vector always 10 on cifar10
    vector<unsigned long> label_int;
    
//    int batch_size;
//    vector<vector<bool> > batch_combinations;
//    vector<bool> right_answers_batch;
//    bool new_batch;
//    int batch_usage_count;
public:
    CIFAR();
    CIFAR(const CIFAR& orig);
    virtual ~CIFAR();
    
    void readCifar(string dataset_path);
    int getPiSize();
    int getNumImages();
    vector<vector<bool> >* getImages();
    vector<vector<bool> >* getLabels();
    vector<unsigned long>* getLabelInt();

//    //from MNIST
//    int getBit(int img_index, int posY, int posX);
//    int getLabel(int);
//    vector <vector <vector <int> > > getAllBits();
//    vector<vector<float> > getPIsProbabilities();
//    void readIdx(ifstream&,string);
//    void setBitsProbabilities(ifstream&);
//        
//    //from PLA
//    void readPLA(string name,int batch_size);
//    void setRandomBatch(mt19937& mt);
//    void setTrueNewBatch();
//    void setFalseNewBatch();
//    void setBatchSize(int size);
//    void operator++();
//    
//    string isNewBatch();
//    int getPiSize();
//    int getNumCombinations();
//    string getName();
//    vector<bool>* getIthCombination(int ith);
//    vector<bool>* getRightAnswers();
//    float getP1Data();
//    int getBatchSize();
//    vector<vector<bool> >* getBatch();
//    vector<bool>* getBatchAnswers();
//    int getBatchCounter();
//    
//    void writePLAdebug();
//    void clear();
//    void printPLA();
//    void printBatch();
//    int getBatchSizeFile(string pla_filepath);    
};

#endif /* CIFAR_H */


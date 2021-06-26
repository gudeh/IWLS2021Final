/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   binaryDS.h
 * Author: augusto
 *
 * Created on 24 de Janeiro de 2019, 14:16
 */

#ifndef MNIST_H
#define MNIST_H

//#include "gde.h"
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

class binaryDS {
    vector <vector <vector <int> > > all_bits;
    vector <int> labels;
    vector <vector <float> > input_probabilities;
    
public:
    binaryDS();
    binaryDS(const binaryDS& orig);
    virtual ~binaryDS();
    void clearMnist();
    
    int getBit(int img_index, int posY, int posX);
    int getLabel(int);
    vector <vector <vector <int> > > getAllBits();
    vector<vector<float> > getPIsProbabilities();
   
    
    void readIdx(ifstream&,string);
    void setPIsBitsProbabilities(ifstream&);
private:

};

#endif /* MNIST_H */


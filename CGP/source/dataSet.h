/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dataSet.h
 * Author: gudeh
 *
 * Created on June 3, 2021, 2:49 PM
 */

#ifndef DATASET_H
#define DATASET_H
#include <string>
using namespace std;
class dataSet {
protected:
    string name;
public:
    dataSet();
    dataSet(const dataSet& orig);
    virtual ~dataSet();
    string getName();
private:

};

#endif /* DATASET_H */


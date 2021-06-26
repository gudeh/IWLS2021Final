import argparse
import pickle
import numpy as np
from sklearn import decomposition, discriminant_analysis, linear_model, svm, tree, neural_network
from sklearn.metrics import f1_score, accuracy_score
from sklearn.ensemble import RandomForestClassifier
from tqdm import *
import matplotlib.pyplot as plt 
import cv2
from collections import Counter
from ctes_trees import *
from math import log,ceil,sqrt,floor
from sklearn.tree import *
from itertools import product
import boolean
import os
import copy
from copy import deepcopy
from sklearn.tree import _tree
from sklearn.model_selection import train_test_split

from sklearn.feature_selection import SelectKBest, SelectPercentile
from sklearn.feature_selection import chi2, f_classif, mutual_info_classif
from sklearn.model_selection import KFold
from sklearn.model_selection import cross_val_score

import graphviz
import pandas as pd

import platform
from copy import deepcopy
import time

function_mappings = {
	'chi2': chi2,
	'f_classif': f_classif,
	'mutual_info_classif': mutual_info_classif,
}

def debug_cifar():
	trn_data, trn_labels, bin_trn_data, bin_trn_labels = [], [], [], []

	batchName = '../data/mini_test_batch'
	binBatchName = '../data/mini_test_batch.bin'

	data = None
	data2 = None
	with open(batchName, 'rb') as fo:
		data = pickle.load(fo, encoding='latin1')

	with open(binBatchName, 'rb') as fo:
		data2 = {}
		data2['labels'] = []
		data2['data'] = []
		byte = 1
		_iter1 = 0
		while _iter1 < 1500:
			byte = fo.read(1)
			data2['labels'].append(int.from_bytes(byte, 'big'))

			_iter2 = 0
			line = []
			while _iter2 < 3072:
				byte = fo.read(1)

				intValue = int.from_bytes(byte, 'big')

				line.append(intValue)

				_iter2 += 1

			_iter1 += 1

			data2['data'].append(line)
	
	data['data'] = np.array(data['data'])
	data['labels'] = np.array(data['labels'])
	data2['data'] = np.asarray(data2['data']).astype('uint8')
	data2['labels'] = np.array(data2['labels'])

    
def gen_small_test():
  
	print("generating small test batch")
	trn_data, trn_labels, tst_data, tst_labels = [], [], [], []
	def unpickle_def(file):
		with open(file, 'rb') as fo:
			data = pickle.load(fo, encoding='latin1')
		return data

	for i in trange(5):
		batchName = '../data/data_batch_%d' % (i+1)
		unpickled = unpickle_def(batchName)
                
		trn_data.extend(unpickled['data'])
		trn_labels.extend(unpickled['labels'])
	trn_data = np.array(trn_data)
	trn_labels = np.array(trn_labels)
	
	Xtrain, Xtest, yTrain, yTest = train_test_split(trn_data, trn_labels, test_size = 0.03, random_state = 42)
	
	batchOutTest = "../data/mini_test_batch"
	batchOutTrain = "../data/red_data_batch"
	data_to_write = {}
	data_to_write['data'] = Xtrain
	data_to_write['labels'] = yTrain
    
	with open(batchOutTrain, 'wb') as fOut:
		pickle.dump(data_to_write, fOut)
        
	data_to_write2 = {}
	data_to_write2['data'] = Xtest
	data_to_write2['labels'] = yTest
    
	

	with open(batchOutTest, 'wb') as fOut:
		pickle.dump(data_to_write2, fOut)
        
	fOut = open("../data/red_data_batch.bin", "wb")
	idxy = 0
	for x in Xtrain:
		valueToWrite = yTrain[idxy]
		byteToWrite = (int(valueToWrite)).to_bytes(1, byteorder='big')
		fOut.write(byteToWrite)
		for feat in x:
			valueToWrite = feat
			byteToWrite = (int(valueToWrite)).to_bytes(1, byteorder='big')
			fOut.write(byteToWrite)
		idxy += 1
        
	fOut.close()
        
	fOut = open("../data/mini_test_batch.bin", "wb")
	idxy = 0
	for x in Xtest:
		valueToWrite = yTest[idxy]
		byteToWrite = (int(valueToWrite)).to_bytes(1, byteorder='big')
		fOut.write(byteToWrite)
		for feat in x:
			valueToWrite = feat
			byteToWrite = (int(valueToWrite)).to_bytes(1, byteorder='big')
			fOut.write(byteToWrite)
		idxy += 1
        
	fOut.close()
    

def load_dataset():
	trn_data, trn_labels = [], []
	
	def unpickle(file):
		with open(file, 'rb') as fo:
			data = pickle.load(fo, encoding='latin1')

		return data
      
	for i in range(5):
		batchName = '../data/data_batch_%d' % (i+1)

		unpickled = unpickle(batchName)
		trn_data.extend(unpickled['data'])
		trn_labels.extend(unpickled['labels'])
        
	trn_data = np.array(trn_data)
	trn_labels = np.array(trn_labels)

	return (trn_data, trn_labels)
  
  
def load_dataset_adjust():
	trn_data, trn_labels = [], []
	
	def unpickle(file):
		with open(file, 'rb') as fo:
			data = pickle.load(fo, encoding='latin1')

		return data
      
	batchName = '../data/red_data_batch'
    
	unpickled = unpickle(batchName)
	trn_data.extend(unpickled['data'])
	trn_labels.extend(unpickled['labels'])
        
	trn_data = np.array(trn_data)
	trn_labels = np.array(trn_labels)

	return (trn_data, trn_labels)
  
  
def load_test_adjust():
	tst_data, tst_labels = [], []
	
	def unpickle(file):
		with open(file, 'rb') as fo:
			data = pickle.load(fo, encoding='latin1')

		return data
      
	batchName = '../data/mini_test_batch'
    
	unpickled = unpickle(batchName)
	tst_data.extend(unpickled['data'])
	tst_labels.extend(unpickled['labels'])
        
	tst_data = np.array(tst_data)
	tst_labels = np.array(tst_labels)

	return (tst_data, tst_labels)

  
def gen_featureSizes(X, bits_precision):
	new_names = ['feat_%d' % (i) for i in range(X.shape[1])]
	
	featureSizes = []
	for col in new_names:
		featureSizes.append([col, bits_precision])
	featureSizes.append(['class', 4])
    
	return featureSizes
  
  
def custom_load_cifar(comb, bits_precision):
	trn_data, trn_labels, tst_data, tst_labels = [], [], [], []
    
	def unpickle(file):
		with open(file, 'rb') as fo:
			data = pickle.load(fo, encoding='latin1')

		return data

	for i in comb[:-1]:
		batchName = '../data/data_batch_%d' % (i)

		unpickled = unpickle(batchName)
		trn_data.extend(unpickled['data'])
		trn_labels.extend(unpickled['labels'])


	unpickled = unpickle('../data/data_batch_%d' % (comb[-1]))
	tst_data.extend(unpickled['data'])
	tst_labels.extend(unpickled['labels'])

	trn_data = np.array(trn_data)
	trn_labels = np.array(trn_labels)
	tst_data = np.array(tst_data)
	tst_labels = np.array(tst_labels)
    
	trn_data = quantize_dataset(trn_data, bits_precision)
	tst_data = quantize_dataset(tst_data, bits_precision)
    
	return (trn_data, trn_labels, tst_data, tst_labels)
    

def gen_eqn_aig(nrInputs, exprs, nameOut):
	
	fOut = open("EQNS/%s.eqn" % (nameOut), "w")
	fGenAig = open("%s/scriptGenAig" % (dirScripts), "w")

	print("INORDER = %s;" % (" ".join(["x%d" % (a) for a in range(nrInputs)])), file = fOut)
	print("OUTORDER = %s;" % (" ".join(["z%d" % (a) for a in range(outs['cifar10'])])), file = fOut)
	for idx, expr in enumerate(exprs):
		print("z%d = %s;" % (idx, expr.replace("and", "*").replace("or", "+").replace("not", "!")), file = fOut)
	
	print("read_eqn EQNS/%s.eqn" % (nameOut), file = fGenAig)
	print("strash", file = fGenAig)
	print("write_aiger AIGS/%s.aig" % (nameOut), file = fGenAig)


	fGenAig.close()
	fOut.close()

	os.system("./abc -F %s/scriptGenAig" % (dirScripts))


def run_aig(nameOut):
	with open("%s/scriptRunAig" % (dirScripts), "w") as fOut:
		print("&iwls21test AIGS/%s.aig ../data/data_batch_all.bin" % (nameOut), file = fOut)

	os.system("./abc -F %s/scriptRunAig" % (dirScripts))
    
  
def run_aig_adjust(nameOut):
	with open("%s/scriptRunAig" % (dirScripts), "w") as fOut:
		print("&iwls21test AIGS/%s.aig ../data/mini_test_batch.bin" % (nameOut), file = fOut)

	os.system("./abc -F %s/scriptRunAig" % (dirScripts))

    
def gen_aig(nameOut):
	with open("%s/scriptGenAig" % (dirScripts), "w") as fOut:
		print("read_eqn EQNS/%s.eqn" % (nameOut), file = fOut)
		print("strash", file = fOut)
		print("write_aiger AIGS/%s.aig" % (nameOut), file = fOut)
        
	os.system("./abc -F %s/scriptGenAig" % (dirScripts))
    

def readAndsNr(nameOut):
	with open("%s/scriptReadAnds" % (dirScripts), "w") as fOut:
		print("&r COMPRESS2RS_AIGS/%s.aig" % (nameOut), file=fOut)
		print("&ps", file=fOut)
	os.system("./abc -F %s/scriptReadAnds > %s/abc_output.txt" % (dirScripts, dirScripts))

	if platform.system() == "Darwin":
		os.system('sed -E "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2})?)?[mGK]//g" %s/abc_output.txt > %s/abc_output_parsed.txt' % (dirScripts, dirScripts))
	else:
		os.system('sed -r "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2})?)?[mGK]//g" %s/abc_output.txt > %s/abc_output_parsed.txt' % (dirScripts, dirScripts))
	
	with open("%s/abc_output_parsed.txt" % (dirScripts), "r") as fIn:
		lines = fIn.readlines()
		ands = lines[4].split()[8]
		lev = lines[4].split()[11]

	return ands
  
  
def compress2rs(nameOut, firstC2RS = 1):

	with open("%s/scriptCompress2rs" % (dirScripts), "w") as fOut:
		print("read_aiger %s/%s.aig" % ("AIGS" if firstC2RS == 1 else "COMPRESS2RS_AIGS", nameOut), file=fOut)
		print("strash", file=fOut)
		print("balance -l", file=fOut)
		print("resub -K 6 -l", file=fOut)
		print("rewrite -l", file=fOut)
		print("resub -K 6 -N 2 -l", file=fOut)
		print("refactor -l", file=fOut)
		print("resub -K 8 -l", file=fOut)
		print("balance -l", file=fOut)
		print("resub -K 8 -N 2 -l", file=fOut)
		print("rewrite -l", file=fOut)
		print("resub -K 10 -l", file=fOut)
		print("rewrite -z -l", file=fOut)
		print("resub -K 10 -N 2 -l", file=fOut)
		print("balance -l", file=fOut)
		print("resub -K 12 -l", file=fOut)
		print("refactor -z -l", file=fOut)
		print("resub -K 12 -N 2 -l", file=fOut)
		print("rewrite -z -l", file=fOut)
		print("balance -l", file=fOut)
		print("write_aiger COMPRESS2RS_AIGS/%s.aig" % (nameOut), file=fOut)

	os.system("./abc -F %s/scriptCompress2rs" % (dirScripts))



def fillWithZeroes(X, X_new):
	X_new2 = deepcopy(X)
	i = 0
	for column in X.T:
		found = 0
		for compColumn in X_new.T:
			if np.array_equal(column, compColumn):
				found = 1
				break
		if found == 0:
			X_new2[:,i] = 0
		i += 1

	return X_new2
  
  
def quantize_dataset(trn_data, bits_precision):
	if bits_precision != 8:
		divider = 8 - bits_precision
		cifar_trn_data = (trn_data / (2**divider)).astype(int)
		return cifar_trn_data
	else:
		cifar_trn_data = deepcopy(trn_data)
		return cifar_trn_data


def feature_selection(X, y, apply_SKB = 0, apply_SP = 0, score_f = 'chi2', k = 200, percent = 10):
	Xred = deepcopy(X)


	if apply_SKB == 1:
		selector = SelectKBest(function_mappings[score_f], k = k)
		selector.fit(X, y)
		Xred = selector.transform(X)
		Xred = fillWithZeroes(X, Xred)

	elif apply_SP == 1:
		selector = SelectPercentile(function_mappings[score_f], percentile = percent)
		selector.fit(X, y)
		Xred = selector.transform(X)
		Xred = fillWithZeroes(X, Xred)
      
	return Xred
  
  
def custom_cross_val(clf, numTrees, bits_precision, seed, colsIdx = []):

	combsCrossVal = [0,1,2,3,4]
	accs = []

	np.random.seed(seed)
    
	for comb in combsCrossVal:
		Xtr, ytr, Xtst, ytst = custom_load_cifar(possibleCombs[comb], bits_precision)
    
		clfs = None
		if useClassicRF == 1:
			np.random.seed(seed)
			clf = clf.fit(Xtr, ytr)
			clfs = clf
		else:
			clfs = []
			for i in range(numTrees):
				Xtr_sub = np.array(Xtr)
				Xtr_sub[:,colsIdx] = 1
				clfs.append(clf[i].fit(Xtr_sub, ytr))
                
		if numTrees == 1:
			yPredict = clfs.predict(Xtst)
			accs.append(accuracy_score(ytst, yPredict))
		else:
          
			votes = []
			for i in range(numTrees):
				votes.append(clf[i].predict(Xtst))
			votes = np.array(votes).T

			yPredict = []

			for vote in votes:
				srtdVote = sorted(vote)
				most_common = Counter(srtdVote).most_common()
				max_occ = max([x[1] for x in most_common])
				most_common = min([x[0] for x in most_common if x[1] == max_occ])
				yPredict.append(most_common)
                
			yPredict = np.array(yPredict)
			accs.append(accuracy_score(ytst, yPredict))
	return accs	
  
  
def trnTst(X, y, classifier, numTrees, maxDepth, bits_precision, seed, useClassicRF, featureSizes):
  
	print("starting crossval and fitting")
	clf = None
	crossValScore = 0.0
	if classifier == 'tree':
		np.random.seed(seed)
		clf = DecisionTreeClassifier(max_depth = maxDepth)
		crossValScore = cross_val_score(clf, X, y, cv=5)
		#crossValScore = custom_cross_val(clf, numTrees)
		clf = clf.fit(X, y)
	elif classifier == 'forest':
		if useClassicRF == 0:
			clf = []
			clfsNoFit = []
			numFeats = len(featureSizes[:-1])
			numFeatsSub = int(sqrt(numFeats))

			colsIdx = []
			for i in range(numTrees):
				colsIdx = np.random.choice(range(numFeats), numFeats - numFeatsSub)
				cifar_train_data_sub = np.array(X)
				cifar_train_data_sub[:,colsIdx] = 1
				_clf = DecisionTreeClassifier(max_depth = maxDepth)
				clfsNoFit.append(_clf)
				clf.append(_clf.fit(cifar_train_data_sub, y))

			crossValScore = custom_cross_val(clfsNoFit, numTrees, bits_precision, seed, colsIdx=colsIdx)
                
		else:
			np.random.seed(seed)
			clf = RandomForestClassifier(max_depth = maxDepth, n_estimators = numTrees)
			crossValScore = custom_cross_val(clf, numTrees, bits_precision, seed)
			np.random.seed(seed)
			clf = clf.fit(X, y)
                        
	print("finished crossval and fitting")
    
	print("starting predicting")
	
	yPredict = None
	yPredictTrain = None
    
	if useStdVote == 1:
		yPredict = clf.predict(X)
	else:
		if classifier == 'tree':
			yPredict = clf.predict(X)
		elif classifier == 'forest':
			votes = []
			for i in range(numTrees):
				votes.append(clf[i].predict(X))
			votes = np.array(votes).T

			yPredict = []

			for vote in votes:
				srtdVote = sorted(vote)
				most_common = Counter(srtdVote).most_common()
				max_occ = max([x[1] for x in most_common])
				most_common = min([x[0] for x in most_common if x[1] == max_occ])
				yPredict.append(most_common)
                
			yPredict = np.array(yPredict)
    
	score = accuracy_score(y, yPredict)
    
	print("finished predicting")

	clf2 = None
	if useClassicRF == 0:
		clf2 = clf
	elif classifier == 'tree':
		clf2 = clf
	elif classifier == 'forest':
		clf2 = []
		for i in clf.estimators_:
			clf2.append(i)
    
    
	return clf2, score, crossValScore
  
  
def trnTst_adjust(X, y, classifier, numTrees, maxDepth, bits_precision, seed, useClassicRF, featureSizes):
  
	print("starting fitting")
	clf = None
	crossValScore = [0.0]*5
	if classifier == 'tree':
		np.random.seed(seed)
		clf = DecisionTreeClassifier(max_depth = maxDepth)
		clf = clf.fit(X, y)
	elif classifier == 'forest':
		np.random.seed(seed)
		clf = RandomForestClassifier(max_depth = maxDepth, n_estimators = numTrees)
		np.random.seed(seed)
		clf = clf.fit(X, y)
                        
	print("finished crossval and fitting")
    
	print("starting predicting")
	
	yPredict = None
    
	Xtest, yTest = load_test_adjust()
    
	XnewTest = quantize_dataset(Xtest, bits_precision)
        
	if useStdVote == 1:
		yPredict = clf.predict(XnewTest)
	else:
		if classifier == 'tree':
			yPredict = clf.predict(XnewTest)
		elif classifier == 'forest':
			votes = []
			for i in range(numTrees):
				votes.append(clf[i].predict(XnewTest))
			votes = np.array(votes).T

			yPredict = []

			for vote in votes:
				srtdVote = sorted(vote)
				most_common = Counter(srtdVote).most_common()
				max_occ = max([x[1] for x in most_common])
				most_common = min([x[0] for x in most_common if x[1] == max_occ])
				yPredict.append(most_common)
                
			yPredict = np.array(yPredict)
    
	score = accuracy_score(yTest, yPredict)
    
	print("finished predicting")

	clf2 = None
	if useClassicRF == 0:
		clf2 = clf
	elif classifier == 'tree':
		clf2 = clf
	elif classifier == 'forest':
		clf2 = []
		for i in clf.estimators_:
			clf2.append(i)

	return clf2, score, crossValScore
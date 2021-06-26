from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier
from sys import argv
import numpy as np
from sklearn.tree import DecisionTreeClassifier, export_graphviz
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import graphviz 
import os
from sklearn.tree import _tree
import copy
from copy import deepcopy
import gen_decision_tree_eqn as eqn
import sys
import boolean
from ctes_trees import *
import pandas as pd
from itertools import product
import sklearn.datasets as ds
from math import log
from math import ceil
from math import sqrt
from math import floor
from sklearn.tree import *
from collections import Counter
from statistics import mean
from utils import *
import os.path
from sys import argv


def get_all_comparisons_ors_opt(tree, which_out = -1):
	tree_ = tree.tree_
	featureName = ["feat_%d" % (i) if i != _tree.TREE_UNDEFINED else "undefined!" for i in tree_.feature]

	comparisons = []

	ors = []

	def recurse(node, depth, expression):
		indent = "\t" * depth
		if tree_.feature[node] != _tree.TREE_UNDEFINED:
			name = featureName[node]
			threshold = tree_.threshold[node]

			if [name, threshold] not in comparisons:
				comparisons.append([name, threshold])

			recurse(tree_.children_left[node], depth + 1, deepcopy(expression + [[name, threshold, 'true']]))

			recurse(tree_.children_right[node], depth + 1, deepcopy(expression + [[name, threshold, 'false']]))
		else:
			if which_out == -1:
				if np.argmax(tree_.value[node]) == 1:
					ors.append(deepcopy(expression))
			else:
				if np.argmax(tree_.value[node]) == which_out:
					ors.append(deepcopy(expression))

	recurse(0, 1, [])

	return comparisons, ors


def convToExpr_eqn_sep(fHandler, ors, exprIdx):
	orIdx = 0
	andIdx = 0
	expr = ""
    
	if len(ors) == 0:
		print("expr_%s = 0;" % (exprIdx), file = fHandler)
	else:
		for idx, _or in enumerate(ors):
			expr = "(" + " * ".join(["%s(comp_%s_%s)" % ("!" if x[2] == 'false' else "", x[0], x[1]) for x in _or]) + ")"
			expr = expr.replace(".","_")
			print("expr_%s_or_%d = %s;" % (exprIdx, idx, expr), file = fHandler)

		orsIdx = range(len(ors))
		secOrsList = []
		for idx2, partialOrsIdx in enumerate(range(0, len(orsIdx), 30)):
			secOrsList.append(idx2)
			expr = " + ".join(["(expr_%s_or_%d)" % (exprIdx, idx) for idx in orsIdx[partialOrsIdx:partialOrsIdx + 30]])
			print("partial_expr_%s_%d = %s;" % (exprIdx, idx2, expr), file = fHandler)

		finalExpr = " + ".join(["(partial_expr_%s_%d)" % (exprIdx, idx) for idx in secOrsList])
		print("expr_%s = %s;" % (exprIdx, finalExpr), file = fHandler)
        
        
def convToExpr_eqn_sep_opt(fHandler, ors, exprIdx):
	orIdx = 0
	andIdx = 0
	expr = ""
    
	if exprIdx.split("_")[0] == "0":
		print("expr_%s = 0;" % (exprIdx), file = fHandler)
	elif len(ors) == 0:
		print("expr_%s = 0;" % (exprIdx), file = fHandler)
	else:
		for idx, _or in enumerate(ors):
			expr = "(" + " * ".join(["%s(comp_%s_%s)" % ("!" if x[2] == 'false' else "", x[0], x[1]) for x in _or]) + ")"
			expr = expr.replace(".","_")
			print("expr_%s_or_%d = %s;" % (exprIdx, idx, expr), file = fHandler)
            
		finalExpr = " + ".join(["(expr_%s_or_%d)" % (exprIdx, idx) for idx in range(len(ors))])
		print("expr_%s = %s;" % (exprIdx, finalExpr), file = fHandler)
    
    
def tree_to_eqn(clf, dataset, classifier, fHandler, numTrees, bits_precision):
	print("started translating")
    
	clfToUse = []
	if classifier == 'tree':
		clfToUse.append(clf)
	elif classifier == 'forest':
		clfToUse = clf
        
	nr_out = outs[dataset]
    
	eqn.gen_header(fHandler)
    
	global_comparisons = []

	for i in range(numTrees):
		for spec_out in range(nr_out):
			comparisons, ors = get_all_comparisons_ors_opt(clfToUse[i], spec_out)
            
			for comp in comparisons:
				if comp not in global_comparisons:
					global_comparisons.append(comp)
                    
			convToExpr_eqn_sep(fHandler, ors, "%d_%d" % (spec_out, i))
            
            
	signals = []
	comparators = []
	existingComparisons = []
    
	for comp in global_comparisons:

		templateBin = "{0:0%db}" % (bits_precision)
		constValue = templateBin.format(int(comp[1])).replace("-","")
		parsedConst = str(comp[1]).replace(".","_")
		eqn.genCte_eqn(fHandler, "const_%s_%s" % (comp[0], parsedConst), constValue)

		eqn.genComp_generic_eqn(fHandler, comp[0], "const_%s_%s" % (comp[0], parsedConst), "comp_%s_%s" % (comp[0], parsedConst), bits_precision) 

        
	exprs = []

	if classifier == 'forest':
		sizeBitsAdd = int(ceil(log(numTrees+1,2)))
		nrCatBits = sizeBitsAdd - 1
		for _out in range(nr_out):
			signal = {}
			signal['name'] = 'add_%d' % (_out)
			signal['size'] = sizeBitsAdd
			signals.append(deepcopy(signal))

		for _out in range(nr_out):
			for i in range(numTrees):
				eqn.extendSignal_eqn(fHandler, "expr_%d_%d" % (_out, i), 1, nrCatBits)
				
		for _out in range(nr_out):

			if numTrees == 2:
				eqn.genAdderNoCout_eqn(fHandler, "expr_%d_0" % (_out), "expr_%d_1" % (_out), "add_%d" % (_out), nrCatBits+1)
			elif numTrees == 3:
				eqn.genAdderNoCout_eqn(fHandler, "expr_%d_0" % (_out), "expr_%d_1" % (_out), "part_add_%d_0" % (_out), nrCatBits+1)
				j = 0
				eqn.genAdderNoCout_eqn(fHandler, "part_add_%d_%d" % (_out, j), "expr_%d_%d" % (_out, j+2), "add_%d" % (_out), nrCatBits+1)
			else:
				eqn.genAdderNoCout_eqn(fHandler, "expr_%d_0" % (_out), "expr_%d_1" % (_out), "part_add_%d_0" % (_out), nrCatBits+1)
				j = 0
				for j in range(numTrees-3):
					eqn.genAdderNoCout_eqn(fHandler, "part_add_%d_%d" % (_out, j), "expr_%d_%d" % (_out, j+2), "part_add_%d_%d" % (_out, j+1), nrCatBits+1)
                    
				eqn.genAdderNoCout_eqn(fHandler, "part_add_%d_%d" % (_out, j+1), "expr_%d_%d" % (_out, j+3), "add_%d" % (_out), nrCatBits+1)
			
		comps = []
		cmprsnsGlobal = []
		highestsGlobal = []
		groupings = 2
		l = ["add_%d" % (x) for x in range(nr_out)]
		cmprsns = [l[i:i+groupings] for i in range(0, len(l), groupings)]
		while len(cmprsns) > 1:
			nextCmprsns = []
			for elem in cmprsns:
				concatNrs = "_".join(["_".join(x.split("_")[1:]) for x in elem])
				if len(elem) < 2:
					nextCmprsns.append("add_%s" % (concatNrs))
					continue

				sizeBitsAdd = int(ceil(log(numTrees+1,2)))
                
				eqn.genCompHeq_eqn(fHandler, elem[0], elem[1], "comp_add_%s" % (concatNrs), sizeBitsAdd)
				eqn.mux21_eqn(fHandler, elem[0], elem[1], "comp_add_%s" % (concatNrs), "add_%s" % (concatNrs), sizeBitsAdd)

				comp = {}
				comp['elems'] = elem
                

				highestsGlobal.append('add_%s' % (concatNrs))
				cmprsnsGlobal.append('comp_add_%s' % (concatNrs))
				idxs = []
	
				nextCmprsns.append("add_%s" % (concatNrs))
				comps.append(deepcopy(comp))
	
			cmprsns = [nextCmprsns[i:i+groupings] for i in range(0,len(nextCmprsns),groupings)]
	
		comp = {}
		comp['elems'] = cmprsns[0]
        
		sizeBitsAdd = int(ceil(log(numTrees+1,2)))
		eqn.genCompHeq_eqn(fHandler, cmprsns[0][0], cmprsns[0][1], 'comp_add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])), sizeBitsAdd)
		eqn.mux21_eqn(fHandler, cmprsns[0][0], cmprsns[0][1], 'comp_add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])), 'add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])), sizeBitsAdd)
		#print(comp)
		comps.append(deepcopy(comp))
		cmprsnsGlobal.append('comp_add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])))
            

	if classifier == 'forest':
		for _out in range(nr_out):
			elemsContainOut = []
			for elem in cmprsnsGlobal:
				if str(_out) in elem.split("_")[2:]:
					elemsContainOut.append(elem)
            
			eqn.concatSignal_eqn(fHandler, "sel_decision_%d" % (_out), elemsContainOut)
		
			seqDecider = ""
			for code in elemsContainOut:
				for comp in comps:
					cats = "comp_add_" + "_".join(comp['elems'][0].split("_")[1:] + comp['elems'][1].split("_")[1:])
					add0 = comp['elems'][0].split("_")[1:]
					add1 = comp['elems'][1].split("_")[1:]
					if code == cats:
						if str(_out) in add0:
							seqDecider += "1"
						else:
							seqDecider += "0"
            
			eqn.trnsfrmLargeSelIn1bit_eqn(fHandler, "sel_decision_%d" % (_out), "sel_decision_%d_red" % (_out), seqDecider)
			eqn.attribSignal_eqn(fHandler, "decision_%d" % (_out), "sel_decision_%d_red" % (_out))

	if classifier == 'tree':
		for _out in range(nr_out):
			eqn.attribSignal_eqn(fHandler, "decision_%d" % (_out), "expr_%d_0" % (_out))


def tree_to_eqn_opt(clf, dataset, classifier, fHandler, numTrees, bits_precision):
	print("started translating")
    
	clfToUse = []
	if classifier == 'tree':
		clfToUse.append(clf)
	elif classifier == 'forest':
		clfToUse = clf
        
	nr_out = outs[dataset]
    
	eqn.gen_header(fHandler)
    
	global_comparisons = []

	for i in range(numTrees):
		for spec_out in range(nr_out):
			comparisons, ors = get_all_comparisons_ors_opt(clfToUse[i], spec_out)
            
			for comp in comparisons:
				if comp not in global_comparisons:
					global_comparisons.append(comp)
                    
			convToExpr_eqn_sep_opt(fHandler, ors, "%d_%d" % (spec_out, i))
            
            
	signals = []
	comparators = []
	existingComparisons = []
    
	for comp in global_comparisons:

		templateBin = "{0:0%db}" % (bits_precision)
		constValue = templateBin.format(int(comp[1])).replace("-","")
		parsedConst = str(comp[1]).replace(".","_")
		eqn.genCte_eqn(fHandler, "const_%s_%s" % (comp[0], parsedConst), constValue)

		eqn.genComp_generic_eqn(fHandler, comp[0], "const_%s_%s" % (comp[0], parsedConst), "comp_%s_%s" % (comp[0], parsedConst), bits_precision)
        
        
	exprs = []

	if classifier == 'forest':
		sizeBitsAdd = int(ceil(log(numTrees+1,2)))
		nrCatBits = sizeBitsAdd - 1
		for _out in range(nr_out):
			signal = {}
			signal['name'] = 'add_%d' % (_out)
			signal['size'] = sizeBitsAdd
			signals.append(deepcopy(signal))

		for _out in range(nr_out):
			for i in range(numTrees):
				eqn.extendSignal_eqn(fHandler, "expr_%d_%d" % (_out, i), 1, nrCatBits)
				
		for _out in range(nr_out):

			if numTrees == 2:
				eqn.genAdderNoCout_eqn(fHandler, "expr_%d_0" % (_out), "expr_%d_1" % (_out), "add_%d" % (_out), nrCatBits+1)
			elif numTrees == 3:
				eqn.genAdderNoCout_eqn(fHandler, "expr_%d_0" % (_out), "expr_%d_1" % (_out), "part_add_%d_0" % (_out), nrCatBits+1)
				j = 0
				eqn.genAdderNoCout_eqn(fHandler, "part_add_%d_%d" % (_out, j), "expr_%d_%d" % (_out, j+2), "add_%d" % (_out), nrCatBits+1)
			else:
				eqn.genAdderNoCout_eqn(fHandler, "expr_%d_0" % (_out), "expr_%d_1" % (_out), "part_add_%d_0" % (_out), nrCatBits+1)
				j = 0
				for j in range(numTrees-3):
					eqn.genAdderNoCout_eqn(fHandler, "part_add_%d_%d" % (_out, j), "expr_%d_%d" % (_out, j+2), "part_add_%d_%d" % (_out, j+1), nrCatBits+1)
                    
				eqn.genAdderNoCout_eqn(fHandler, "part_add_%d_%d" % (_out, j+1), "expr_%d_%d" % (_out, j+3), "add_%d" % (_out), nrCatBits+1)
			
		comps = []
		cmprsnsGlobal = []
		highestsGlobal = []
		groupings = 2
		l = ["add_%d" % (x) for x in range(nr_out)]
		cmprsns = [l[i:i+groupings] for i in range(0, len(l), groupings)]
		while len(cmprsns) > 1:
			nextCmprsns = []
			for elem in cmprsns:
				concatNrs = "_".join(["_".join(x.split("_")[1:]) for x in elem])
				if len(elem) < 2:
					nextCmprsns.append("add_%s" % (concatNrs))
					continue

				sizeBitsAdd = int(ceil(log(numTrees+1,2)))
                
				eqn.genCompHeq_eqn(fHandler, elem[0], elem[1], "comp_add_%s" % (concatNrs), sizeBitsAdd)
				eqn.mux21_eqn(fHandler, elem[0], elem[1], "comp_add_%s" % (concatNrs), "add_%s" % (concatNrs), sizeBitsAdd)

				comp = {}
				comp['elems'] = elem
                

				highestsGlobal.append('add_%s' % (concatNrs))
				cmprsnsGlobal.append('comp_add_%s' % (concatNrs))
				idxs = []
	
				nextCmprsns.append("add_%s" % (concatNrs))
				comps.append(deepcopy(comp))
	
			cmprsns = [nextCmprsns[i:i+groupings] for i in range(0,len(nextCmprsns),groupings)]
	
		comp = {}
		comp['elems'] = cmprsns[0]
        
		sizeBitsAdd = int(ceil(log(numTrees+1,2)))
		eqn.genCompHeq_eqn(fHandler, cmprsns[0][0], cmprsns[0][1], 'comp_add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])), sizeBitsAdd)
		eqn.mux21_eqn(fHandler, cmprsns[0][0], cmprsns[0][1], 'comp_add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])), 'add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])), sizeBitsAdd)
		#print(comp)
		comps.append(deepcopy(comp))
		cmprsnsGlobal.append('comp_add_%s' % ("_".join(["_".join(x.split("_")[1:]) for x in cmprsns[0]])))
            

	if classifier == 'forest':
		for _out in range(nr_out):
			elemsContainOut = []
			for elem in cmprsnsGlobal:
				if str(_out) in elem.split("_")[2:]:
					elemsContainOut.append(elem)
            
			eqn.concatSignal_eqn(fHandler, "sel_decision_%d" % (_out), elemsContainOut)
		
			seqDecider = ""
			for code in elemsContainOut:
				for comp in comps:
					cats = "comp_add_" + "_".join(comp['elems'][0].split("_")[1:] + comp['elems'][1].split("_")[1:])
					add0 = comp['elems'][0].split("_")[1:]
					add1 = comp['elems'][1].split("_")[1:]
					if code == cats:
						if str(_out) in add0:
							seqDecider += "1"
						else:
							seqDecider += "0"
            
			eqn.trnsfrmLargeSelIn1bit_eqn(fHandler, "sel_decision_%d" % (_out), "sel_decision_%d_red" % (_out), seqDecider)
			eqn.attribSignal_eqn(fHandler, "decision_%d" % (_out), "sel_decision_%d_red" % (_out))

	if classifier == 'tree':
		for _out in range(nr_out):
			eqn.attribSignal_eqn(fHandler, "decision_%d" % (_out), "expr_%d_0" % (_out))
            
        
def initializeSetup():
	os.system("mkdir -p EQNS")
	os.system("mkdir -p AIGS")
	os.system("mkdir -p COMPRESS2RS_AIGS")
	os.system("mkdir -p %s" % (dirScripts))


def convertToEqn(classifier, numTrees, maxDepth, dataset, bits_precision, clf):

	AfileOutEqn = open("EQNS/out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth.eqn" % (classifier, numTrees, maxDepth, bits_precision), 'w')
    
	if classifier == 'forest':
		tree_to_eqn(clf, dataset, classifier, AfileOutEqn, numTrees, bits_precision)
	elif classifier == 'tree':
		#tree_to_eqn(clf, dataset, classifier, AfileOutEqn, numTrees)
		tree_to_eqn_opt(clf, dataset, classifier, AfileOutEqn, numTrees, bits_precision)
    
	AfileOutEqn.close()


seed = int(argv[1])
adjustModels = int(argv[2])

np.random.seed(seed)
dataset = 'cifar10'
fHits = open("%s/hit_values.csv" % (folder_results), "a")

if not(os.path.isfile("../data/mini_test_batch")):
	gen_small_test()

initializeSetup()

for classifier in classifiers:
        
	X = None
	y = None
    
	if adjustModels == 0:
		X, y = load_dataset()
	else:
		X, y = load_dataset_adjust()
    
	#apply feature selection methods
	if applyFS == 1:
		Xcropped = feature_selection(X, y, use_SKB = useSKB, use_SP = useSP, score_f = score_function, k = k, percent = percent)
		X = Xcropped
	################################
		    
	for bits_precision in bits_precisions:
      
		featureSizes = gen_featureSizes(X, bits_precision)

		Xnew = quantize_dataset(X, bits_precision)
        
		_numTrees = [1]
		if classifier == 'tree':
			_numTrees = [1]
		elif classifier == 'forest':
			_numTrees = numTrees
        
		for numTree, maxDepth in product(_numTrees, maxDepths):
		
			print("started training...")
			np.random.seed(seed)
			if adjustModels == 0:
				clf, accuracy, crossValAccuracy = trnTst(Xnew, y, classifier, numTree, maxDepth, bits_precision, seed, useClassicRF, featureSizes)
			else:
				clf, accuracy, crossValAccuracy = trnTst_adjust(Xnew, y, classifier, numTree, maxDepth, bits_precision, seed, useClassicRF, featureSizes)
                
			print("finished training...")
                          
			ands = 0
		
			print("started conversion to eqn")
			convertToEqn(classifier, numTree, maxDepth, dataset, bits_precision, clf)
            
			print("generating aig from eqn")
			gen_aig("out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth" % (classifier, numTree, maxDepth, bits_precision))
            
			print("running aig")
			if adjustModels == 0:
				run_aig("out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth" % (classifier, numTree, maxDepth, bits_precision))
			else:
				run_aig_adjust("out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth" % (classifier, numTree, maxDepth, bits_precision))
    
			compress2rs("out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth" % (classifier, numTree, maxDepth, bits_precision))
            
			for i in range(nrExtraC2RS):
				compress2rs("out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth" % (classifier, numTree, maxDepth, bits_precision), firstC2RS = 0)
            
			ands = readAndsNr("out_%s_numTrees_%d_maxDepth_%d_bitsPrecision_%d_nosynth" % (classifier, numTree, maxDepth, bits_precision))
            
            
			print(",".join([str(x) for x in [dataset, classifier, maxDepth, numTree, bits_precision, accuracy, "_".join([str(x) for x in crossValAccuracy]), mean(crossValAccuracy), nrExtraC2RS+1, ands]]), file=fHits)		
        

fHits.close()

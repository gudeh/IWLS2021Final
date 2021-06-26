from os import system
import os

datasets = ['cifar10']

outs = {}
outs['cifar10'] = 10

folder_datasets = 'datasets'
folder_results = 'GEN_RESULTS'
curDir = os.getcwd()

possibleCombs = []
possibleCombs.append([2,3,4,5,1])
possibleCombs.append([1,3,4,5,2])
possibleCombs.append([1,2,4,5,3])
possibleCombs.append([1,2,3,5,4])
possibleCombs.append([1,2,3,4,5])

dirScripts = "abc_scripts"

system('mkdir -p %s' % (folder_results))

fHits = open("%s/hit_values.csv" % (folder_results), "w")
print(",".join(['Dataset', 'Classifier', 'MaxDepth', 'NumTrees', 'Precision', 'Acc', 'CrossValAccs', 'AvgCrossValAcc', 'NrC2RS', 'Ands']), file=fHits)
fHits.close()

nrExtraC2RS = 2
nrCrossVal = 5

useStdVote = 0
useClassicRF = 1 #not gonna support 0 here
applyFS = 0

classifiers = ['forest']
maxDepths = [5,6,7,8,9,10,11,12,13]
numTrees = [10,15,20,25,30,50,75]
bits_precisions = [3,4,5,6,7,8]

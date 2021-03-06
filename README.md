# IWLS2021Final

This code implements a solution for the IWLS 2021 contest, based on the CIFAR dataset as a Logic Synthesis challenge. It involves Decision Trees, Cartesian Genetic Programming, and approximate optimization with logic signal probabilities.

It was a pleasure to yet again participate in this exciting programming contest based on Machine Learning and Logic Synthesis. This year's problem is a more significant challenge, and we had less time to work on a solution. Nonetheless, the team quickly adapted our solution (with minor improvements also) and was able to participate once more.

For the solution to run properly, you need the abc executable (https://github.com/berkeley-abc/abc) compiled and present in the folders: ./sk/, ./CGP/source/ and ./MLAIGReduction/source, as well as the pickle (python) and binary (C) versions of the data batches of CIFAR10 (https://www.cs.toronto.edu/~kriz/cifar.html) in the ./data/ folder. To run all the processes implemented by the team a simple bash run in the ./run_all.sh file should suffice. 

Python requirements: sklearn, math, pickle, argparse, collections, itertools, tqdm, platform, time, graphviz, pandas, copy, boolean and statistics.
C++ requirements: GCC version 7.

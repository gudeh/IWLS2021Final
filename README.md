# IWLS2021Final

It was a pleasure to yet again participate in this exciting programming contest based on Machine Learning and Logic Synthesis. This year's problem is a more significant challenge, and we had less time to work on a solution. Nonetheless, the team quickly adapted our solution (with minor improvements also) and was able to participate once more. 

There was an early attempt to reproduce Neural Networks as AND-Inverter Graphs. However, the challenge seemed time demanding, and we opted to utilize Random Forets only. We intend to keep on with this work and make it feasible to build AIGs from NNs.

For the solution to run properly, you need the abc software (https://github.com/berkeley-abc/abc) compiled present in the folders: SUBMISSION/sk/, SUBMISSION/CGP/source/ and SUBMISSION/MLAIGReduction/source, as well as the regular and binary versions of the data batches of CIFAR10 in the ./data/ folder. To run all the processes implemented by the team a simple bash run in the ./run_all.sh file should suffice. 

Python requirements: sklearn, math, pickle, argparse, collections, itertools, tqdm, platform, time, graphviz, pandas, copy, boolean and statistics.
C++ requirements: GCC version 7.

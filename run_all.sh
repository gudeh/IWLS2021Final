#!/bin/bash

rm -rf data/mini*
rm -rf data/red*
rm -rf data/*all*
rm -f acc_crossval.csv
rm -f acc_minitest.csv
rm -rf AIGS_MINITEST/
rm -rf AIGS_ALLSET/
sh clean_all.sh
cat data/data_batch_1.bin data/data_batch_2.bin data/data_batch_3.bin data/data_batch_4.bin data/data_batch_5.bin > data/data_batch_all.bin
cd sk
python gen_dt.py 0 0
mv COMPRESS2RS_AIGS/ ../AIGS_ALLSET
mv GEN_RESULTS/hit_values.csv ../acc_crossval.csv
cd ..
sh clean_all.sh
cd sk
python gen_dt.py 30 1
mv COMPRESS2RS_AIGS/ ../AIGS
mv GEN_RESULTS/hit_values.csv ../acc_minitest.csv
cd ..
sh clean_all.sh

cd CGP/source/
sh runLocal.sh
cd ../../MLAIGReduction/source/
sh localRun.sh


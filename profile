#!/bin/bash

# SIMPLE script to set variables before playing with python version of AbsSynthe
PYTHONPATH="./pycosat-0.6.0":"./pycudd2.0.2/pycudd":$PYTHONPATH DYLD_LIBRARY_PATH="./pycudd2.0.2/cudd-2.4.2/lib":$DYLD_LIBRARY_PATH LD_LIBRARY_PATH="./pycudd2.0.2/cudd-2.4.2/lib":$LD_LIBRARY_PATH python -m cProfile "$@"

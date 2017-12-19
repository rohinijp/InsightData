#!/usr/bin/env bash

# one example of run.sh script for implementing the features using python
# the contents of this script could be replaced with similar files from any major language

# I'll execute my programs, with the input directory log_input and output the files in the directory log_output
set -x
g++ -g -std=c++11 -o insight ./src/insight.cpp
./insight ./log_input/log.txt

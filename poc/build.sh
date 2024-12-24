#!/bin/bash

clang++ -std=c++20 -fobjc-arc -o oslog_reader main.cpp OSLogWrapper.mm -framework Foundation -framework oslog

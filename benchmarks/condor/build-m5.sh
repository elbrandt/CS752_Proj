#!/bin/bash
# gem5 m5 build shell script

cd util/m5
scons build/x86/out/m5
cd ../..

tar -czf build_m5.tar util/m5/build/

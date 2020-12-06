#!/bin/bash
# running DAXPY in gem5 shell script

echo "mkdir m5out"
mkdir m5out

#build/X86/gem5.opt -r configs/example/se.py --cpu-type=MinorCPU --ruby -cdaxpy --options="$1"
echo "build/X86/gem5.opt sim-config.py --exe=daxpy --args $1"
build/X86/gem5.opt sim-config.py --exe=daxpy --args $1

echo "tar -czf m5out.tar m5out"
tar -czf m5out.tar m5out

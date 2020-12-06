#!/bin/bash
# gem5 build shell script

scons build/X86/gem5.opt -sQ -j$(nproc) CPU_MODELS=AtomicSimpleCPU,TimingSimpleCPU,O3CPU,MinorCPU

tar -czf build.tar build/

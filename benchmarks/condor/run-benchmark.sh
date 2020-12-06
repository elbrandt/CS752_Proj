#!/bin/bash
# running a benchmark in gem5 shell script
# usage: run-benchmark.sh [benchmark-name]
#
# where benchmark-name is one of
#  daxpy, libquantum, mempinch1, povray, sjeng

echo "mkdir m5out"
mkdir m5out

echo -n "Running benchmark $1 $2 $3..."
case $1 in
    daxpy)
        build/X86/gem5.opt $2.py --cpu $3 --exe daxpy --args 16384
        ;;
    libquantum)
        build/X86/gem5.opt $2.py --cpu $3 --exe libquantum --args 33 5
        ;;
    mempinch1)
        build/X86/gem5.opt $2.py --cpu $3 --exe mempinch1
        ;;
    povray)
        build/X86/gem5.opt $2.py --cpu $3 --exe povray --args povray_data/test/input/SPEC-benchmark-test.ini +Ipovray_data/test/input/SPEC-benchmark-test.pov +Lpovray_data/all/input +Oscene.tga +W20 +H20
        ;;
    sjeng)
        build/X86/gem5.opt $2.py --cpu $3 --exe sjeng --args sjeng_data/test/input/test_short.txt
        ;;
    *)
        echo "Unknown benchmark"
        ;;
esac

echo "tar -czf m5out_$1_$2_$3.tar m5out"
tar -czf m5out_$1_$2_$3.tar m5out

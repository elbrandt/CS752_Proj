#!/bin/bash

echo "Running benchmark $1..."
case $1 in
    daxpy)
        $GEM5_ROOT/build/X86/gem5.opt --debug-flags HWPrefetch config/sim-config.py --exe bin/daxpy --args 16384 > out.txt
        ;;
    libquantum)
        $GEM5_ROOT/build/X86/gem5.opt config/sim-config.py --exe bin/libquantum --args 33 5
        ;;
    mempinch1)
        $GEM5_ROOT/build/X86/gem5.opt config/sim-config.py --exe bin/mempinch1
        ;;
    povray)
        $GEM5_ROOT/build/X86/gem5.opt config/sim-config.py --exe bin/povray --args bin/povray_data/test/input/SPEC-benchmark-test.ini +Ibin/povray_data/test/input/SPEC-benchmark-test.pov +Lbin/povray_data/all/input +Oscene.tga +W20 +H20
        ;;
    sjeng)
        $GEM5_ROOT/build/X86/gem5.opt config/sim-config.py --exe bin/sjeng --args bin/sjeng_data/test/input/test_short.txt
        ;;
    *)
        echo "Unknown benchmark"
        ;;
esac
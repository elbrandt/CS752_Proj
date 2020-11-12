# -*- coding: utf-8 -*-
# Copyright (c) 2015 Mark D. Hill and David A. Wood
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Jason Power, Eric Brandt (CS752 Fall 2020, UW Madison)

""" CPU based on MinorCPU with options for a simple gem5 configuration script

This file contains a CPU model based on MinorCPU that allows for a few options
to be tweaked. 
Specifically, issue latency, op latency, and the functional unit pool.

See src/cpu/MinorCPU.py for MinorCPU details.

Modified by ELB to add configurable latency on Integer FU

"""

from m5.objects import MinorCPU, MinorFUPool
from m5.objects import MinorDefaultIntFU, MinorDefaultIntMulFU
from m5.objects import MinorDefaultIntDivFU, MinorDefaultFloatSimdFU
from m5.objects import MinorDefaultMemFU, MinorDefaultFloatSimdFU
from m5.objects import MinorDefaultMiscFU

class HW3IntFU(MinorDefaultIntFU):

    # From MinorDefaultIntFU
    # opLat = 3

    # From MinorFU
    # issueLat = 1

    def __init__(self, options=None):
        super(MinorDefaultIntFU, self).__init__()

        if options and options.int_operation_latency:
            self.opLat = options.int_operation_latency

        if  options and options.int_issue_latency:
            self.issueLat = options.int_issue_latency

class HW3FloatSIMDFU(MinorDefaultFloatSimdFU):

    # From MinorDefaultFloatSimdFU
    # opLat = 6

    # From MinorFU
    # issueLat = 1

    def __init__(self, options=None):
        super(MinorDefaultFloatSimdFU, self).__init__()

        if options and options.fpu_operation_latency:
            self.opLat = options.fpu_operation_latency

        if  options and options.fpu_issue_latency:
            self.issueLat = options.fpu_issue_latency


class HW3FUPool(MinorFUPool):

    def __init__(self, options=None):
        super(MinorFUPool, self).__init__()

        # Copied from src/mem/MinorCPU.py
        self.funcUnits = [# HW3 Int FU
                          HW3IntFU(options), HW3IntFU(options),
                          MinorDefaultIntMulFU(), MinorDefaultIntDivFU(),
                          MinorDefaultMemFU(), MinorDefaultMiscFU(),
                          # HW3 Float FU
                          HW3FloatSIMDFU(options)]


class HW3MinorCPU(MinorCPU):

    def __init__(self, options=None):
        super(MinorCPU, self).__init__()
        
        self.executeFuncUnits = HW3FUPool(options)
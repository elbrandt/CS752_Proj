# Eric Brandt - HW3, CS752 Fall 2020, UW Madison
import m5
from m5.objects import *
from caches import *
from optparse import OptionParser
from HW3CPU import *
import importlib

def vararg_callback(option, opt_str, value, parser):
    assert value is None
    value = []

    def floatable(str):
        try:
            float(str)
            return True
        except ValueError:
            return False

    for arg in parser.rargs:
        # stop on --foo like options
        if arg[:2] == "--" and len(arg) > 2:
            break
        # stop on -a, but not on -3 or -3.0
        if arg[:1] == "-" and len(arg) > 1 and not floatable(arg):
            break
        value.append(arg)

    del parser.rargs[:len(value)]
    setattr(parser.values, option.dest, value)

parser = OptionParser()
parser.add_option('--l1i_size', help="L1 instruction cache size")
parser.add_option('--l1d_size', help="L1 data cache size")
parser.add_option('--l2_size', help="Unified L2 cache size")
parser.add_option('--cpu', help="CPU Type [TimingSimpleCPU|MyMinorCPU]", default="TimingSimpleCPU")
parser.add_option('--mem', help="Memory Type [DDR3_1600_8x8|LPDDR2_S4_1066_1x32|DDR3_2133_8x8|HBM_1000_4H_1x64]", default="HBM_1000_4H_1x64")
parser.add_option('--freq', help="CPU Frequency", default="1GHz")
parser.add_option('--exe', help="executable name", default="bin/daxpy")
parser.add_option('--args', help="executable arguments", dest="exe_args", action="callback", callback=vararg_callback, default=[])
parser.add_option('--fpu_operation_latency', help="fpu operation latency in cycles", default=6)
parser.add_option('--fpu_issue_latency', help="cycles until another float instruction can be issued", default=1)
parser.add_option('--int_operation_latency', help="int operation latency in cycles", default=3)
parser.add_option('--int_issue_latency', help="cycles until another int instruction can be issued", default=1)
(options, args) = parser.parse_args()

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = options.freq

system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('512MB')]

if options.cpu == "HW3MinorCPU":
    system.cpu = HW3MinorCPU(options)
else:
    cpu_cls = getattr(m5.objects, options.cpu)
    system.cpu = cpu_cls()
system.membus = SystemXBar()

system.cpu.icache = L1ICache(options)
system.cpu.dcache = L1DCache(options)
system.cpu.icache.connectCPU(system.cpu)
system.cpu.dcache.connectCPU(system.cpu)

system.l2bus = L2XBar()
system.cpu.icache.connectBus(system.l2bus)
system.cpu.dcache.connectBus(system.l2bus)

system.l2cache = L2Cache(options)
system.l2cache.connectCPUSideBus(system.l2bus)
system.l2cache.connectMemSideBus(system.membus)
system.l2cache.prefetcher = SignaturePathPrefetcherV2()
system.l2cache.prefetch_on_access = False
system.l2cache.prefetcher.on_miss = True
system.l2cache.prefetcher.enablePPF = True
system.l2cache.prefetcher.prefetch_filter_threshold = 11.0

system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.master
system.cpu.interrupts[0].int_master = system.membus.slave
system.cpu.interrupts[0].int_slave = system.membus.master

system.system_port = system.membus.slave

mem_cls = getattr(m5.objects, options.mem)
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = mem_cls()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master

process = Process()
# add MKL to the LD_LIBRARY_PATH:
# process.env = ['LD_LIBRARY_PATH=/opt/intel/compilers_and_libraries_2019.5.281/linux/mkl/lib/intel64_lin']
if options.exe_args:
    process.cmd = [options.exe] + options.exe_args
else:
    process.cmd = [options.exe]

system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()

print('Exiting @ tick {} because {}'.format(m5.curTick(), exit_event.getCause()))

import sys
import datetime
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.ticker import StrMethodFormatter
import numpy as np
from scipy.stats import gmean
import os
import glob
import re
from operator import itemgetter
from tabulate import tabulate
import pandas as pd

def get_stats_files():
    fils = []
    columns = ['filename', 'benchmark', 'config', 'cpu']

    # glob_search = 'results/**/m5out/stats.txt'
    # re_search = 'results/m5out_([A-Za-z0-9]*)_([A-Za-z0-9\-]*)_([A-Za-z0-9]*)/m5out/stats.txt'
    glob_search = 'results_sgs/**/stats.txt'
    re_search = 'results_sgs/m5out_([A-Za-z0-9]*)_([A-Za-z0-9\-]*)_([A-Za-z0-9]*)/stats.txt'

    for fil in glob.glob(glob_search):
        m = re.search(re_search, fil)
        fils.append(dict(zip(columns, m.group(0,1,2,3))))
    return pd.DataFrame(fils)

def parse_stats_from_file(fname, stats, occurrence):
    visit = 0
    ret = {stat:0 for stat in stats}

    with open(os.path.join(fname)) as logfile:
        for _, line in enumerate(logfile):
            if 'Begin Simulation Statistics' in line:
                visit = visit + 1
                continue
            ln = line.split()
            if len(ln) > 1:
                for stat in stats:
                    if ln[0] == stat and visit == occurrence:
                        ret[stat] = float(ln[1])
    return ret

def load_stats(df, benchmark_section, stat_names):
    new_stats = {stat:[] for stat in stat_names}
    for _, rec in df.iterrows():
        fname = rec['filename']
        section_num = benchmark_section[rec['benchmark']]
        stat = parse_stats_from_file(fname, stat_names, section_num)
        for s in stat:
            new_stats[s].append(stat[s])
    for nm in stat_names:
        df[nm] = new_stats[nm]
    return df

def main():
    """main function"""

    benchmark_section = {'daxpy':2, 'libquantum':1, 'mempinch1':1, 'povray':1, 'sjeng':1}
    stat_names = [  'host_seconds', 
                    'sim_ticks', 
                    'system.cpu.committedInsts', 
                    'system.cpu.numCycles', 
                    'system.l2cache.prefetcher.pfIssued',           # number of hwpf issued
                    'system.l2cache.prefetcher.usefulPrefetches',    # number of useful hwpf
                    'system.l2cache.prefetcher.pfIdentified',              # number of prefetch candidates identified
                    'system.l2cache.prefetcher.pfBufferHit',               # number of redundant prefetches already in prefetch queue
                    'system.l2cache.prefetcher.pfInCache',              # number of redundant prefetches already in cache/mshr dropped
                    'system.l2cache.prefetcher.pfSpanPage',           # number of prefetches that crossed the page
                    'system.l2cache.prefetcher.power_state.pwrStateResidencyTicks::UNDEFINED',    # Cumulative time (in ticks) in various power states
                    'system.l2cache.tags.occ_percent::.l2cache.prefetcher',    # Average percentage of cache occupancy
                    'system.mem_ctrl.dram.bw_total::.l2cache.prefetcher'     # Total bandwidth to/from this memory (bytes/s)                  
                  ]

    stats = get_stats_files()
    stats = load_stats(stats, benchmark_section, stat_names)
    stats['ipc'] = stats['system.cpu.committedInsts'] / stats['system.cpu.numCycles']

    df_simple = stats[['benchmark', 'config', 'cpu', 'ipc']].sort_values(['cpu', 'benchmark', 'config'])
    df_simple.rename(columns={'config':'Configuration'}, inplace=True)
    df_simple.replace('sppv2', 'SPPv2', inplace=True)
    df_simple.replace('sppv2-ppf', 'SPPv2-PPF', inplace=True)
    ds2 = df_simple.pivot(index='benchmark', columns='Configuration', values='ipc')
    axs = ds2.plot(kind='bar')
    axs.set_ylabel("IPC")
    axs.set_xlabel("Benchmark")
    axs.set_title('IPC Comparison, SimpleTimingCPU')
    plt.xticks(rotation=0)
    plt.tight_layout()
    plt.show()
    print(df_simple)
    print(ds2)

if __name__ == '__main__':
    main()

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
    stat_names = ['host_seconds', 'sim_ticks', 'system.cpu.committedInsts', 'system.cpu.numCycles' ]

    stats = get_stats_files()
    stats = load_stats(stats, benchmark_section, stat_names)
    stats['ipc'] = stats['system.cpu.committedInsts'] / stats['system.cpu.numCycles']

    df_simple = stats[['benchmark', 'config', 'cpu', 'ipc']].sort_values(['cpu', 'benchmark', 'config'])
    print(df_simple)
    # q3
    # tbl = []
    # for alg in algs:
    #     ooo = int(ParseStat(f"results/q2_stats_{alg}.txt", 'sim_ticks', 2)) 
    #     ooo_opt = int(ParseStat(f"results-O3/q2_stats_{alg}.txt", 'sim_ticks', 2))
    #     if alg == 'daxpy':
    #         baseline = ooo
    #     row = {'Algoritm': alg, 'DerivO3CPU': baseline/ooo, 'DeriveO3CPU_Opt': baseline/ooo_opt}
    #     tbl.append(row)

    # df = pd.DataFrame(tbl)
    # print(df.to_latex(index=False, float_format="%.2f"))

if __name__ == '__main__':
    main()
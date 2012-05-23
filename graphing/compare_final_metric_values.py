#!/usr/bin/env python
"""
plots the metric values at the end of each slice registration
from 2 or more different directories.

If exactly 2 directories are given, then the differences between
the metric values are plotted as well.

Usage:
compare_final_metric_values dir1 [dir2 [dir3...]]
"""

from sys import argv
from metric_values import MetricValues

from numpy import *
import matplotlib.pyplot as plt

fig = plt.figure()
ax = fig.gca()

for i, directory in enumerate(argv[1:]):
    vals = MetricValues(directory)
    x = range(vals.number_of_slices())
    ax.plot(x, vals.final_values(), label=str(i+1) )

plt.grid(axis="y")
ax.legend()
plt.show()

# plot the differences if 2 directories are given
if len(argv) == 3:
    fig = plt.figure()
    ax = fig.gca()
    vals1 = MetricValues(argv[1]).final_values()
    vals2 = MetricValues(argv[2]).final_values()
    x = range(len(vals1))
    
    diffs = [vals1[i] - vals2[i] for i in x]
    ax.plot(x, diffs)
    plt.show()
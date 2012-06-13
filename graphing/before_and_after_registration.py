#!/usr/bin/env python
"""
plots the metric values at the start and end of each slice registration

Usage:
before_and_after_registration values_dir
"""

from sys import argv
from os import listdir
from metric_values import MetricValues

from numpy import *
import matplotlib.pyplot as plt

metric_values_dir = argv[1]
basenames = listdir(metric_values_dir)

# extract metric values
vals = MetricValues(metric_values_dir)

fig = plt.figure()
ax = fig.gca()
x = range(vals.number_of_slices())
ax.plot(x, vals.initial_values(), label="Initial values")
ax.plot(x, vals.final_values(), label="Final values")
plt.grid(axis="y")
ax.legend()
plt.show()

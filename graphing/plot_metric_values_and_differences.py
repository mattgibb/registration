#!/usr/bin/env python
from os.path import *
from os import listdir
from sys import argv
from metric_values import MetricValues

metric_values_dir = argv[1]
metric_values = MetricValues(metric_values_dir)
basenames = listdir(metric_values_dir)

# plot 3d line
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt

mpl.rcParams['legend.fontsize'] = 10

def plot_3d_lines(values):
    fig = plt.figure()
    ax = fig.gca(projection='3d')
    for i, slice in enumerate(values):
        x = range(len(slice))
        y = [i] * len(slice)
        ax.plot(x, y, slice, label=basenames[i])
    ax.legend()
    plt.show()
    
plot_3d_lines(metric_values.values())
plot_3d_lines(metric_values.delta_values())
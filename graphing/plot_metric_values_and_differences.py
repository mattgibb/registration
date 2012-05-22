#!/usr/bin/env python
"""
plots the intermediate metric values of each slice registration

Usage:
plot_metric_values_and_differences values_dir [slice]
plot_metric_values_and_differences values_dir --batch-size X

"""

from os.path import *
from os import listdir
from sys import argv
from numpy import genfromtxt
from metric_values import MetricValues

metric_values_dir = argv[1]

# plot 3d line
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt

mpl.rcParams['legend.fontsize'] = 10

if len(argv) == 3:
    # plot 3d lines in batches
    batch_size = argv[3]
        
    # plot particular slice
    def plot_2d_line(values):
        fig = plt.figure()
        ax = fig.gca()
        x = range(len(values))
        ax.plot(x, values)
        plt.grid(axis="y")
        plt.show()
    
    metric_values = genfromtxt(join(metric_values_dir, argv[2]))
    plot_2d_line(metric_values)
    plot_2d_line(metric_values[1:] - metric_values[:-1])
        
# plot all slices
else:
    def plot_3d_lines(values, basenames):
        fig = plt.figure()
        ax = fig.gca(projection='3d')
        for i, slice in enumerate(values):
            x = range(len(slice))
            y = [i] * len(slice)
            ax.plot(x, y, slice, label=basenames[i])
        ax.legend()
        plt.show()
    
    basenames = listdir(metric_values_dir)
    metric_values = MetricValues(metric_values_dir)
    
    # plot slices in batches
    if len(argv) == 4 and argv[2] == '--batch-size':
        batch_size = int(argv[3])
        for i in range(0,len(metric_values.values()), batch_size):
            plot_3d_lines(metric_values.values()[i:i+batch_size], basenames[i:i+batch_size])
    
    # plot the whole set of slices together    
    else:
        plot_3d_lines(metric_values.values())
        plot_3d_lines(metric_values.delta_values())    

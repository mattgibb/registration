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
from numpy import arange, genfromtxt
from metric_values import MetricValues

metric_values_dir = argv[1]

# plot 3d line
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt

mpl.rcParams['legend.fontsize'] = 10

if len(argv) == 3:
    # plot particular slice
    def plot_2d_line(values, ylabel):
        fig = plt.figure(figsize=(10, 6))
        ax = fig.add_axes([0.12,0.1,0.85,0.85])
        x = range(len(values))
        ax.plot(x, values)
        plt.xlabel('Iteration', fontsize='xx-large')
        plt.ylabel(ylabel, fontsize='xx-large')
        plt.tick_params(axis='both', which='major', labelsize=16)
        # plt.xlim([0,1500])
        # plt.xticks( arange(0,1501,500) )
        plt.grid(axis="y")
        plt.show()
    
    
    metric_values = genfromtxt(join(metric_values_dir, argv[2]))
    plot_2d_line(metric_values, 'Normalised Correlation')
    plot_2d_line(metric_values[1:] - metric_values[:-1], 'Delta Correlation')
    
# plot all slices
else:
    def plot_3d_lines(values, labels):
        fig = plt.figure(frameon=False)
        ax = fig.add_axes([0.0,0.0,1.0,1.0], projection='3d')
        for i, slice in enumerate(values):
            x = range(len(slice))
            y = [i] * len(slice)
            ax.plot(x, y, slice, label=labels[i])
        ax.legend()
        plt.xlabel('Iteration', fontsize='xx-large')
        plt.ylabel('Slice Number', fontsize='xx-large')
        ax.set_zlabel("Normalised Correlation", fontsize='xx-large')
        plt.show()
    
    basenames = listdir(metric_values_dir)
    labels = [basename + " (" + str(i+1) + ")" for i, basename in enumerate(basenames)]
    
    metric_values = MetricValues(metric_values_dir)
    
    # plot slices in batches
    if len(argv) == 4 and argv[2] == '--batch-size':
        batch_size = int(argv[3])
        for i in range(0,len(metric_values.values()), batch_size):
            plot_3d_lines(metric_values.values()[i:i+batch_size], labels[i:i+batch_size])
    
    # plot the whole set of slices together    
    else:
        plot_3d_lines(metric_values.values(), labels)
        plot_3d_lines(metric_values.delta_values(), labels)    

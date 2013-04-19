#!/usr/bin/env python
"""
plots the evolving ground truth errors of each slice

Usage:
plot_ground_truth_distances.py [--relative]

"""

from sys import argv
from os import listdir
from os.path import *
from numpy import *

# plot 3d line
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# construct error directory
errors_dir = "RelativeGroundTruthErrors" if argv[1] == "--relative" else "GroundTruthErrors"
print argv[1]
errors_path = "/Users/matt/Code/imaging/registration/results/dummy/200_alpha0.4rt/HiResPairs/" + errors_dir

# read errors
error_files = sorted(listdir(errors_path), key=int)
errors = transpose(array([genfromtxt(join(errors_path, file)) for file in error_files]))

# plot
def plot_3d_lines(values):
    fig = plt.figure(frameon=False)
    ax = fig.add_axes([0.0,0.0,1.0,1.0], projection='3d')
    for i, slice in enumerate(values):
        x = range(len(slice))
        y = [i] * len(slice)
        ax.plot(x, y, slice)
    plt.xlabel('Smoothing Iteration', fontsize='xx-large')
    plt.ylabel('Slice Number', fontsize='xx-large')
    ax.set_zlabel("Mean Euclidian Distance Error", fontsize='xx-large')
    plt.show()

plot_3d_lines(errors)

# 2d plots
def plot_2d_line(values):
    fig = plt.figure(figsize=(10, 6))
    ax = fig.add_axes([0.12,0.1,0.85,0.85])
    x = range(len(values))
    ax.plot(x, values)
    
# mean error
means = mean(errors, 0)
plot_2d_line(means)
plt.xlabel('Smoothing Iteration', fontsize='xx-large')
plt.ylabel("Mean Slice Error", fontsize='xx-large')
plt.tick_params(axis='both', which='major', labelsize=16)
plt.xticks( arange(len(means)) )
plt.ylim([0,1800])
plt.grid(axis="y")
plt.show()

# error reduction ratios
ratios = means[:-1] / means[1:]
plot_2d_line(ratios)
plt.xlabel('Smoothing Iteration', fontsize='xx-large')
plt.ylabel("Error Reduction Ratio", fontsize='xx-large')
plt.tick_params(axis='both', which='major', labelsize=16)
plt.xticks( arange(len(ratios)) )
# plt.ylim([0,1800])
plt.grid(axis="y")
plt.show()

#!/usr/bin/env python
from os.path import *
from os import listdir
from stack_metric_values import StackMetricValues

# construct arrays from metric value data
output_dir = abspath(join(__file__, "../../results", "Rat28", "deletemenow"))
metric_values_dir = join(output_dir, "MetricValues")
rigid_dir      = join(metric_values_dir, "CenteredRigid2DTransform")
similarity_dir = join(metric_values_dir, "CenteredSimilarity2DTransform")
affine_dir     = join(metric_values_dir, "CenteredAffineTransform")

# raise AssertionError if contents of 3 tranform dirs are not the same
# i.e. contain the same slices
assert listdir(rigid_dir) == listdir(similarity_dir) == listdir(affine_dir)
basenames = listdir(rigid_dir)

metric_values = StackMetricValues(rigid_dir)

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
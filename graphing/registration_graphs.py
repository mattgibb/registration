#!/usr/bin/env python

# external packages
import sys
from IPython.Shell import IPShellEmbed
ipshell = IPShellEmbed()

# my packages
from sources.stdout_reader import StdoutReader
from sources.ssv_reader import SsvReader
from sequences.matplotlib_plotter import MatplotlibPlotter
from sequences.mayavi_plotter import MayaviPlotter

# extract command line arguments
params_file, output_dir = sys.argv[1:3]

# parse input file
multires = SsvReader(params_file)

# parse single resolution file
singleres = StdoutReader('../images/test_results/cropped_output_500_iterations')
level              = singleres.level()
iteration          = singleres.iteration()
metric_value       = singleres.metric_value()
versor_params      = singleres.versor_params()
translation_params = singleres.translation_params()

# plotter = MatplotlibPlotter(output_dir, singleres)
plotter = MayaviPlotter(output_dir, singleres)
plotter.plot()
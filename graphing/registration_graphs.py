#!/usr/bin/env python

# external packages
import sys
import numpy as np
from IPython.Shell import IPShellEmbed
ipshell = IPShellEmbed(['-wthread'])
from enthought.mayavi import mlab
# ipshell()

# my packages
from sources.versor_reader import VersorReader
from sources.mhd_reader import MHDReader
from sequences.matplotlib_plotter import MatplotlibPlotter
from sequences.mayavi_plotter import MayaviPlotter

# extract command line arguments
if len(sys.argv) > 1:
    output_dir = sys.argv[1]
else:
    output_dir = "../../images/test_results"

# parse files
params3D = VersorReader(output_dir + "/output3D.txt")
stack_metadata = MHDReader(output_dir + "/stack.mhd")
unit_cube = np.asarray([[[0,0,0],
                         [1,0,0]],
                      
                        [[0,1,0],
                         [1,1,0]],
                      
                        [[0,0,1],
                         [1,0,1]],
                      
                        [[0,1,1],
                         [1,1,1]]])

stack_cuboid = np.tile(np.asarray(stack_metadata.image_size()), [4,2,1] ) * unit_cube

# plot edges of cuboid
for i in range(4):
    # mlab.plot3d(stack_cuboid[i,:,0],stack_cuboid[i,:,1],stack_cuboid[i,:,2])
    # mlab.plot3d(stack_cuboid[i,:,1],stack_cuboid[i,:,2],stack_cuboid[i,:,0])
    # mlab.plot3d(stack_cuboid[i,:,2],stack_cuboid[i,:,0],stack_cuboid[i,:,1])
    mlab.plot3d(unit_cube[i,:,0],unit_cube[i,:,1],unit_cube[i,:,2])
    mlab.plot3d(unit_cube[i,:,1],unit_cube[i,:,2],unit_cube[i,:,0])
    mlab.plot3d(unit_cube[i,:,2],unit_cube[i,:,0],unit_cube[i,:,1])

ipshell()
# plotter = MatplotlibPlotter(output_dir + "/movies", singleres)
# plotter = MayaviPlotter(output_dir + "/movies", params3d)
# plotter.plot()

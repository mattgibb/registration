#!/usr/bin/env python

# external packages
import os, sys
import matplotlib.pyplot as plt
from numpy import * 
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.axes import subplot_class_factory
from IPython.Shell import IPShellEmbed
ipshell = IPShellEmbed()

# my packages
from sources.stdout_reader import StdoutReader
from sources.ssv_reader import SsvReader

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

# generate movie of parameters
fig = plt.figure(figsize=(15,5))
Subplot3D = subplot_class_factory(Axes3D)

versor_ax      = Subplot3D(fig, 131, aspect='equal')
translation_ax = Subplot3D(fig, 132, aspect='equal')
metric_ax      = plt.subplot(133)

versor_ax.plot(versor_params[:,0],versor_params[:,1],versor_params[:,2])
translation_ax.plot(translation_params[:,0],translation_params[:,1],translation_params[:,2])
metric_ax.plot(metric_value)

versor_lims = versor_ax.get_w_lims()
translation_lims = translation_ax.get_w_lims()
metric_lims = metric_ax.get_xlim() + metric_ax.get_ylim()

for i in range(1,30):
# for i in range(1,len(iteration)):
	versor_ax.cla()
	translation_ax.cla()
	metric_ax.cla()
	
	versor_ax.plot(versor_params[:i,0],versor_params[:i,1],versor_params[:i,2])
	translation_ax.plot(translation_params[:i,0],translation_params[:i,1],translation_params[:i,2])
	metric_ax.plot(metric_value[:i])
	
	versor_ax.set_aspect('equal')
	translation_ax.set_aspect('equal')
	
	versor_ax.set_title('Versor Parameters')
	translation_ax.set_title('Translation Parameters')
	metric_ax.set_title('Metric Value')
	
	versor_ax.set_xlim3d(versor_lims[0:2])
	versor_ax.set_ylim3d(versor_lims[2:4])
	versor_ax.set_zlim3d(versor_lims[4:6])
	translation_ax.set_xlim3d(translation_lims[0:2])
	translation_ax.set_ylim3d(translation_lims[2:4])
	translation_ax.set_zlim3d(translation_lims[4:6])
	metric_ax.set_xlim(metric_lims[0:2])
	metric_ax.set_ylim(metric_lims[2:4])
	
	versor_ax.azim +=1
	translation_ax.azim +=1
	
	fname = '%s/_tmp%03d.png' % (output_dir, i)
	print 'Saving frame', fname
	fig.savefig(fname)
  
print 'Making movie animation.mpg - this may take a while'
os.system("cd %s && mencoder 'mf://_tmp*.png' -mf type=png:fps=10 -ovc lavc -lavcopts vcodec=mpeg4 -oac copy -o animation.avi" % output_dir )
print 'Removing png files...'
os.system('rm %s/*.png' % output_dir )
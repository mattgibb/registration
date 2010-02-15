#!/usr/bin/env python
import os, sys
import matplotlib.pyplot as plt
import numpy.random as random
import re

# read registration output
f = open(sys.argv[1])
lines = f.readlines()
print lines[0]

regexp = '([0-9]+) = '
match = re.search(regexp, lines[0])
print ''.join(['match.group(0) = \'', match.group(0), '\''])
print ''.join(['match.group(1) = \'', match.group(1), '\''])

# files = []
# fig = plt.figure(figsize=(5,5))
# ax = fig.add_subplot(111)
# for i in range(50):
# 	ax.cla()
# 	ax.imshow(random.rand(5,5), interpolation='nearest')
# 	fname = '_tmp%03d.png'%i
# 	print 'Saving frame', fname
# 	fig.savefig(fname)
# 	files.append(fname)
# 	
# print 'Making movie animation.mpg - this may take a while'
# os.system("mencoder 'mf://_tmp*.png' -mf type=png:fps=10 -ovc lavc -lavcopts vcodec=wmv2 -oac copy -o animation.avi")
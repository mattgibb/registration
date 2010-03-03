from numpy import *

# my files
from sources.iteration_data import IterationData

class SsvReader(IterationData):
    def construct_array(self, lines):
	    # parse lines
		lines = [line.split() for line in lines]
		
		# save array of parameters
		self.array = asarray(lines)
	

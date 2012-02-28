from os.path import *
from os import listdir
from numpy import genfromtxt, array

class StackMetricValues:
    def __init__(self, transform_dir):
        value_list = [genfromtxt(join(transform_dir, file)) for file in listdir(transform_dir)]
        self._values = array(value_list)
    
    def values(self):
        return self._values
    
    def delta_values(self):
        # subtract the previous value from each value
        return self._values[:,1:] - self._values[:,:-1]

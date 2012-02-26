from os.path import *
from os import listdir
from numpy import genfromtxt

class StackMetricValues:
    def __init__(self, transform_dir):
        self.values = [genfromtxt(join(transform_dir, file)) for file in listdir(transform_dir)]

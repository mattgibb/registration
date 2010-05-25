from numpy import *

class ReaderBase:
    def __init__(self, filename):
        # read registration output
        self.filename = filename
        f = open(filename)
        lines = f.readlines()
        
        # parse lines and save result as array
        self.construct_array(lines)
    
    def construct_array(self, lines):
        # parse lines
        lists = [line.split() for line in lines]

        # prepend each list with its resolution level
        level_change = [False] + [lists[i] <= lists[i-1] for i in range(1,len(lists))]
        levels = [sum(level_change[:i+1]) for i in range(len(level_change))]
        for i, params in enumerate(lists):
            params.insert(0, levels[i])

        # save array of parameters
        self.array = asarray(lists)
    
    def level(self):
        return self.array[:,0]
    
    def iteration(self):
        return self.array[:,1]
    
    def metric_value(self):
        return self.array[:,2]
    

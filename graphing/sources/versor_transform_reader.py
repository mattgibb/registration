from numpy import *

# my files
from sources.reader_base import ReaderBase

class VersorTransformReader(ReaderBase):
    def construct_array(self, lines):
        # parse lines
        lists = [line.split() for line in lines]
        level_change = [False] + [lists[i] <= lists[i-1] for i in range(1,len(lists))]
        levels = [sum(level_change[:i+1]) for i in range(len(level_change))]
        
        # prepend each list with its resolution level
        for i, params in enumerate(lists):
            params.insert(0, levels[i])
                
        # save array of parameters
        self.array = asarray(lists)
    
    def versor_params(self):
        return self.array[:,3:6]
    
    def translation_params(self):
        return self.array[:,6:9]
    

from numpy import * 

# my files
from sources.iteration_data import IterationData

class StdoutReader(IterationData):
    def construct_array(self, lines):
        # parse lines
        for line_index, line in enumerate(lines):
            cleaned_line = []
            line = line.rsplit()
            for word_index, word in enumerate(line):
                word = word.lstrip('[:=')
                word = word.rstrip(':=,]')
                if not len(word) == 0:
                    cleaned_line.append(float(word))
            lines[line_index] = cleaned_line
        
        # save array of parameters
        self.array = asarray(lines)
    

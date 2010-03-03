class IterationData:
    def __init__(self, filename):
        # read registration output
        self.filename = filename
        f = open(filename)
        lines = f.readlines()
        
        # parse lines and save result as array
        self.construct_array(lines)
    
    def level(self):
        return self.array[:,0]
    
    def iteration(self):
        return self.array[:,1]
    
    def metric_value(self):
        return self.array[:,2]
    
    def versor_params(self):
        return self.array[:,3:6]
    
    def translation_params(self):
        return self.array[:,6:9]
    

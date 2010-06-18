from sources.reader_base import ReaderBase

class CenteredRigidReader(ReaderBase):
    def angle_params(self):
        return self.array[:,3]
    
    def center_params(self):
        return self.array[:,4:6]
    
    def translation_params(self):
        return self.array[:,6:8]
    

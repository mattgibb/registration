from sources.reader_base import ReaderBase

class VersorReader(ReaderBase):
    def versor_params(self):
        return self.array[:,3:6]
    
    def translation_params(self):
        return self.array[:,6:9]
    

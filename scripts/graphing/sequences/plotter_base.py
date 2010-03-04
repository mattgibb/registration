class PlotterBase:
    def __init__(self, output_dir, *sources):
        self.sources = sources
        self.output_dir = output_dir
    

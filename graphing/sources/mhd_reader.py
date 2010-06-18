import re

class MHDReader:
    def __init__(self, filename):
        f = open(filename)
        header = f.read()
        
        # parse spacing
        m_spacing = re.search(
            """(?xm)
                ^
                    ElementSpacing\s=\s
                    (?P<x_spacing>\d+(\.\d*)?)\s
                    (?P<y_spacing>\d+(\.\d*)?)\s
                    (?P<z_spacing>\d+(\.\d*)?)
                $
            """,
            header
        )
        
        # save spacing array
        self.element_spacing = [float(m_spacing.group('x_spacing')),
                                float(m_spacing.group('y_spacing')),
                                float(m_spacing.group('z_spacing'))]
        
        # parse dim size
        m_dim_size = re.search(
            """(?xm)
                ^
                    DimSize\s=\s
                    (?P<x_dim_size>\d+)\s
                    (?P<y_dim_size>\d+)\s
                    (?P<z_dim_size>\d+)
                $
            """,
            header
        )
        
        # save dim size array
        self.dim_size = [float(m_dim_size.group('x_dim_size')),
                         float(m_dim_size.group('y_dim_size')),
                         float(m_dim_size.group('z_dim_size'))]

    def image_size(self):
        return [self.element_spacing[i] * self.dim_size[i] for i in range(len(self.element_spacing))]

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.axes import subplot_class_factory
import os

from sequences.plotter_base import PlotterBase

class MatplotlibPlotter(PlotterBase):
    def plot(self):
        self.setup_figure()
        self.calculate_lims()
        self.plot_sequence()
        self.generate_movie()
    
    def setup_figure(self):
        self.fig = plt.figure(figsize=(15,5))
        Subplot3D = subplot_class_factory(Axes3D)
        
        self.versor_ax      = Subplot3D(self.fig, 131, aspect='equal')
        self.translation_ax = Subplot3D(self.fig, 132, aspect='equal')
        self.metric_ax      = plt.subplot(133)
    
    def calculate_lims(self):
        versor_params      = self.sources[0].versor_params()
        translation_params = self.sources[0].translation_params()
        metric_value       = self.sources[0].metric_value()
        
        self.versor_ax.plot(versor_params[:,0],versor_params[:,1],versor_params[:,2])
        self.translation_ax.plot(translation_params[:,0],translation_params[:,1],translation_params[:,2])
        self.metric_ax.plot(metric_value)
        
        self.versor_lims      = self.versor_ax.get_w_lims()
        self.translation_lims = self.translation_ax.get_w_lims()
        self.metric_lims      = self.metric_ax.get_xlim() + self.metric_ax.get_ylim()
        
    def plot_sequence(self):
        versor_params      = self.sources[0].versor_params()
        translation_params = self.sources[0].translation_params()
        metric_value       = self.sources[0].metric_value()
		
    	# for i in range(1,30):
        number_of_iterations = len( self.sources[0].iteration() )
        for i in range(1,number_of_iterations):
            self.versor_ax.cla()
            self.translation_ax.cla()
            self.metric_ax.cla()
            
            self.versor_ax.plot(versor_params[:i,0],versor_params[:i,1],versor_params[:i,2])
            self.translation_ax.plot(translation_params[:i,0],translation_params[:i,1],translation_params[:i,2])
            self.metric_ax.plot(metric_value[:i])
            
            self.versor_ax.set_aspect('equal')
            self.translation_ax.set_aspect('equal')
            
            self.versor_ax.set_title('Versor Parameters')
            self.translation_ax.set_title('Translation Parameters')
            self.metric_ax.set_title('Metric Value')
            
            self.versor_ax.set_xlim3d(self.versor_lims[0:2])
            self.versor_ax.set_ylim3d(self.versor_lims[2:4])
            self.versor_ax.set_zlim3d(self.versor_lims[4:6])
            self.translation_ax.set_xlim3d(self.translation_lims[0:2])
            self.translation_ax.set_ylim3d(self.translation_lims[2:4])
            self.translation_ax.set_zlim3d(self.translation_lims[4:6])
            self.metric_ax.set_xlim(self.metric_lims[0:2])
            self.metric_ax.set_ylim(self.metric_lims[2:4])
            
            self.versor_ax.azim +=1
            self.translation_ax.azim +=1
            
            fname = '%s/_tmp%03d.png' % (self.output_dir, i)
            print 'Saving frame', fname
            self.fig.savefig(fname)

    def generate_movie(self):
        print 'Making movie animation.mpg - this may take a while'
        os.system("cd %s && mencoder 'mf://_tmp*.png' -mf type=png:fps=10 -ovc lavc -lavcopts vcodec=mpeg4 -oac copy -o animation.avi" % self.output_dir )
        print 'Removing png files...'
        os.system('rm %s/*.png' % self.output_dir )
            

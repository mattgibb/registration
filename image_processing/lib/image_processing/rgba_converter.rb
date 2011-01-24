require 'image_processing/config'
require 'image_processing/file_manager'

module ImageProcessing
  class RGBAConverter
    
    def initialize(rgba_dir, rgb_dir)
      @rgba_dir, @rgb_dir = rgba_dir, rgb_dir
    end
    
    def go
      until images_to_be_converted.empty?
        convert_rgba(images_to_be_converted[0])
        puts "Converted #{rgb_images.count} of #{rgba_images.count} images."
      end
      puts "All RGBA files have been converted!"
    end
        
    def convert_rgba(filename)
      print "Converting #{filename}..."
      command = [
        File.join(Config::ITK_DIR, "ConvertRGBAToRGB"),
        File.join(@rgba_dir, filename),
        File.join(@rgb_dir, filename)
      ].join(" ")
      
      # run command
      system command
      puts "done"
    end
    
  private
    def rgba_images
      Dir[File.join(@rgba_dir, '*.bmp')].map {|f| File.basename f }
    end
    
    def rgb_images
      Dir[File.join(@rgb_dir, '*.bmp')].map {|f| File.basename f }
    end
    
    def images_to_be_converted
      rgba_images - rgb_images
    end
    
  end
end

require 'image_processing/config'
require 'image_processing/ftp_adaptor'
require 'image_processing/file_manager'

module ImageProcessing
  class ImageFile
    attr_reader :name, :slice_number, :version_number
    
    def initialize(filename)
      raise "#{filename} doesn't seem to be an image file." unless filename =~ /\.bmp$/
      @name = filename
      m = @name.match(/(\d+)(\(\d+\))?.bmp$/)
      @slice_number = m[1].to_i
      @version_number = m[2].to_i if m[2]
    end                                    
    
  end
  
  class Picker
    def initialize(argv)
      @config = Config.new(argv)
      @ftp = FtpAdaptor.new(@config)
      @file_manager = FileManager.new(@config, @ftp)
      pick_files
    end
    
    def files
      @files.map {|f| f.name }
    end
    
  private  
    def pick_files      
      # select one every @config.downsample_ratio images
      available_files = @file_manager.remote_originals.sort.map {|f| ImageFile.new(f) }
      puts available_files.count
      puts available_files.first.name
      puts available_files.last.name
      slice_range = (available_files.first.slice_number)..(available_files.last.slice_number)
      @files = []
      
      slice_range.display
      
      slice_range.step(@config.downsample_ratio).each do |target_slice_number|
        # pick list of candidates
        candidates = available_files.take_while{ |file| file.slice_number < target_slice_number + @config.downsample_ratio }
        
        # raise an exception if there aren't any candidates
        raise "There aren't any candidates" if candidates.empty?
        
        # decide on an image to move and append its file name to the list
        @files << pick_image(candidates, target_slice_number)
        
        # next image must be above previous index and previous image
        cutoff = [@files.last.slice_number, target_slice_number].max
        available_files = available_files.drop_while {|file| file.slice_number <= cutoff }
      end
    end

    def pick_image(candidates, target_slice_number)
      # sort list of candidates in order of decreasing preference
      candidates.sort! do |a,b|
        # candidate minus index
        a_distance = a.slice_number - target_slice_number
        b_distance = b.slice_number - target_slice_number
        
        # compare absolute distances from index
        comp = (a_distance.abs <=> b_distance.abs)
        
        # if absolute distances are the same, put lower candidate first
        comp==0 ? a_distance <=> b_distance : comp
      end
      
      # if there is more than one version of the first slice, pick a version
      # otherwise, just use first slice
      if candidates.first.version_number
        versions = candidates.take_while {|candidate| candidate.slice_number == candidates.first.slice_number}
        return pick_version(versions)
      else
        return candidates.first
      end
    end
    
    def pick_version(versions)
      # simply pick the first one
      versions.first
    end
    
  end
end
require 'image_processing/config'
require 'image_processing/ftp_adaptor'
require 'image_processing/file_manager'
require 'fileutils'

module ImageProcessing
  class Downsampler
    include FileUtils
    
    def initialize(argv)
      @config = Config.new(argv)
      @ftp = FtpAdaptor.new(@config)
      @file_manager = FileManager.new(@config, @ftp)
    end
    
    def go
      @file_manager.check_local_dirs
      
      until @file_manager.originals_to_be_downsampled.empty?
        puts "Downsampled #{@file_manager.processed_files} of #{@file_manager.remote_downsamples} files..."
        unless @file_manager.originals_ready_to_be_downsampled.empty?
          downsample_file(@file_manager.originals_ready_to_be_downsampled[0])
        else
          puts "No files ready to downsample yet, sleeping..."
          sleep 10
        end
      end

      puts "All files have been downsampled!"
    end
    
    def downsample_file(filename)
      original   = File.join(@config.local_originals_dir,   filename)
      downsample = File.join(@config.local_downsamples_dir, filename)
      print "Downsampling #{filename}..."
      `../itk/ShrinkImage '#{original}' '#{downsample}' #{@config.downsample_ratio}`
      if $? == 0
        puts "done."
        print "Removing hi-res copy of #{filename}..."
        rm "#{original}"
        puts "done.\n\n"
      else
        @file_manager.add_error_file(filename)
      end
    end
  end
end

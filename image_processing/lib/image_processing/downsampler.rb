require 'image_processing/config'
require 'image_processing/ftp_adaptor'
require 'image_processing/file_manager'
require 'fileutils'

module ImageProcessing
  class Downsampler
    include FileUtils::Verbose
    
    def initialize(argv)
      @config = Config.new(argv)
      @ftp = FtpAdaptor.new(@config)
      @file_manager = FileManager.new(@config, @ftp)
    end
    
    def go
      @file_manager.check_local_dirs
      
      until @config.files_to_be_downsampled.empty?
        unless @config.fully_downloaded_files.empty?
          downsample_file(fully_downloaded_files[0])
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
        puts "Downsampling failed!"
      end
    end
    
  end
end

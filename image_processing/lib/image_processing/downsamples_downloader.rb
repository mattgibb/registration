require 'image_processing/config'
require 'image_processing/ftp_adaptor'
require 'image_processing/file_manager'

module ImageProcessing
  class DownsamplesDownloader
    
    def initialize(argv)
      @config = Config.new(argv)
      @ftp = FtpAdaptor.new(@config)
      @file_manager = FileManager.new(@config, @ftp)
    end
    
    def go
      @file_manager.check_local_dirs
      until @file_manager.downsamples_to_be_downloaded.empty?
        puts "Downloaded #{@file_manager.local_downsamples.count} of #{@file_manager.remote_downsamples.count} files..."
        download_downsample(@file_manager.downsamples_to_be_downloaded[0])
      end
      puts "All files have been downloaded!"
    end
        
    def download_downsample(filename)
      source = File.join(@config.remote_downsamples_dir, filename)
      destination = File.join(@config.local_downsamples_dir, filename)
      check_local_space
      @ftp.download_file(source, destination)
    end
    
    def check_local_space
      while @file_manager.available_gigabytes < 5
        puts "Not enough space, trying again in a minute..."
        sleep 60
      end
    end
    
  end
end

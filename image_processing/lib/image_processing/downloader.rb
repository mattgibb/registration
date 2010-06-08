require 'image_processing/config'
require 'image_processing/ftp_adaptor'
require 'image_processing/file_manager'
require 'fileutils'

module ImageProcessing
  class Downloader
    include FileUtils::Verbose
    
    def initialize(argv)
      @config = Config.new(argv)
      @ftp = FtpAdaptor.new(@config)
      @file_manager = FileManager.new(@config, @ftp)
    end
    
    def go
      check_local_dirs      
      download_file(@file_manager.files_to_be_downloaded[0]) until @file_manager.files_to_be_downloaded.empty?
      puts "All files have been downloaded!"
    end
    
    def check_local_dirs
      [@config.local_originals_dir, @config.local_downsamples_dir].each do |dir|
        unless Dir.exists?(dir)
          mkdir_p(dir)
        end 
      end
    end
    
    def download_file(filename)
      source = File.join(@config.remote_originals_dir, filename)
      destination = File.join(@config.local_originals_dir, filename)
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

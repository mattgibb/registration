require 'image_processing/config'
require 'image_processing/ftp_adaptor'
require 'image_processing/file_manager'

module ImageProcessing
  class Uploader
    def initialize(argv)
      @config = Config.new(argv)
      @ftp = FtpAdaptor.new(@config)
      @file_manager = FileManager.new(@config, @ftp)
    end
    
    def go
      @file_manager.check_remote_downsamples_dir
      
      until @file_manager.downsamples_to_be_uploaded.empty?
        source      = File.join(@config.local_downsamples_dir,  @file_manager.downsamples_to_be_uploaded[0])
        destination = File.join(@config.remote_downsamples_dir, @file_manager.downsamples_to_be_uploaded[0])
        @ftp.upload_file(source, destination)
      end

      puts "All available files have been uploaded!"
    end
  end
end

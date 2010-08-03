require 'image_processing/ftp_adaptor'

module ImageProcessing
  class DirectorySynchroniser
    attr_accessor :server
    
    def initialize(host, remote_dir, local_dir)
      @host, @remote_dir, @local_dir = host, remote_dir, local_dir
      @ftp = FtpAdaptor.new(@host, 'mef', 'meflab')
    end
    
    def download
      remote_files = @ftp.list(@remote_dir).map {|f| File.basename f }
      local_files = Dir.chdir(@local_dir) { Dir['*'] }
      files_remaining = remote_files - local_files
      
      files_remaining.each do |file|
        puts "Files left to download:", files_remaining, "\n"
        @ftp.download_file(@remote_dir + file, @local_dir + file)
      end
      
      puts "All available files have been uploaded!"
    end
  end
end

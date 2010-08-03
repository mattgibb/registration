require 'fileutils'
require 'image_processing/ftp_adaptor'

module ImageProcessing
  class DirectorySynchroniser
    include FileUtils::Verbose
        
    def initialize(host, remote_dir, local_dir)
      @host, @remote_dir, @local_dir = host, remote_dir, local_dir
      @ftp = FtpAdaptor.new(@host, 'mef', 'meflab')
      check_local_dir
    end
    
    def download      
      until files_to_download.empty?
        puts "Files left to download:", files_to_download, "\n"
        file = files_to_download.first
        @ftp.download_file(@remote_dir + file, @local_dir + file)
      end
      
      puts "All available files have been uploaded!\n\n"
    end
    
  private
    def check_local_dir
      unless Dir.exists?(@local_dir)
        mkdir_p(@local_dir)
      end
    end
    
    def remote_files
      @remote_files ||= @ftp.list(@remote_dir).map {|f| File.basename f }
    end
    
    def local_files
      Dir.chdir(@local_dir) { Dir['*'] }
    end
    
    def files_to_download
      remote_files - local_files
    end
  end
end

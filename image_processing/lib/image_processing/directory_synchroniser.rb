# Class to facilitate efficient 2-way syncing of local and ftp directories

require 'fileutils'
require 'image_processing/ftp_adaptor'

module ImageProcessing
  class DirectorySynchroniser
    include FileUtils::Verbose
        
    def initialize(host, remote_dir, local_dir)
      @host, @remote_dir, @local_dir = host, remote_dir, local_dir
      @ftp = FtpAdaptor.new(@host, 'mef', 'meflab')
    end
    
    def download
      check_local_dir
      dirs = []
      until (files_to_download - dirs).empty?
        puts "Files left to download:", (files_to_download - dirs).count, "\n"
        file = (files_to_download - dirs).first
        begin
          @ftp.download_file(@remote_dir + file, @local_dir + file)
        rescue Net::FTPPermError => e
          raise unless e.message =~ /550/ # only handle exception if file is not a regular file
          puts e
          puts "#{file} is not a proper file, skipping..."
          dirs << file
        end
      end
      
      puts "All available files have been downloaded!\n\n"
    end
    
    def upload
      check_remote_dir
      dirs = []
      until (files_to_upload - dirs).empty?
        puts "Files left to upload:", (files_to_upload - dirs).count, "\n"
        file = (files_to_upload - dirs).first
        begin
          @ftp.upload_file( File.join(@local_dir, file), File.join(@remote_dir, file) )
        rescue Net::FTPPermError => e
          raise unless e.message =~ /550/ # only handle exception if file is not a regular file
          puts e
          puts "#{file} is not a proper file, skipping..."
          dirs << file
        end
      end
      
      puts "All available files have been uploaded!\n\n"
    end

  private
    def check_local_dir
      unless Dir.exists?(@local_dir)
        mkdir_p(@local_dir)
        puts "Created local directory #{@local_dir}.\n\n"
      end
    end
  
    def check_remote_dir
      begin
        @ftp.mkdir @remote_dir
        puts "Created remote directory #{@remote_dir}.\n\n"
      rescue Net::FTPPermError
        puts "Remote directory already exists.\n\n"
      end
    end
  
    def remote_files
      @ftp.list(@remote_dir).map {|f| File.basename f }
    end
    
    def cached_remote_files
      @remote_files ||= @ftp.list(@remote_dir).map {|f| File.basename f }
    end
    
    def local_files
      Dir.chdir(@local_dir) { Dir['*'] }
    end
    
    def files_to_download
      cached_remote_files - local_files
    end
    
    def files_to_upload
      local_files - remote_files
    end
  end
end

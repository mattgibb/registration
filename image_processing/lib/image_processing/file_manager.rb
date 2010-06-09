require 'fileutils'

module ImageProcessing
  class FileManager
    include FileUtils::Verbose
    
    def initialize(config, ftp)
      @config, @ftp = config, ftp
    end
    
    def check_local_dirs
      [@config.local_originals_dir, @config.local_downsamples_dir].each do |dir|
        unless Dir.exists?(dir)
          mkdir_p(dir)
        end
      end
    end
    
    def check_remote_downsamples_dir
      begin
        @ftp.mkdir @config.remote_downsamples_dir
        puts "Created remote downsamples directory.\n\n"
      rescue Net::FTPPermError
        puts "Remote downsamples directory already exists.\n\n"
      end
    end
    
    def all_files
      @all_files ||= @ftp.list(@config.remote_originals_dir).select {|f| File.fnmatch? "*.bmp", f }.map {|f| File.basename f }
    end
    
    def fully_downloaded_files
      Dir[File.join(@config.local_originals_dir, '*.bmp')].map {|f| File.basename f }
    end

    def downsampled_files
      Dir[File.join(@config.local_downsamples_dir, '*.bmp')].map {|f| File.basename f }
    end

    def error_files
      File.read(File.join(@config.local_dataset_dir, "error_files.txt")).split
    end
    
    def files_to_be_downsampled
      all_files - downsampled_files - error_files
    end
    
    def files_to_be_downloaded
      files_to_be_downsampled - fully_downloaded_files
    end
    
    def files_ready_to_be_downsampled
      fully_downloaded_files - error_files
    end
    
    def fully_uploaded_files
      @ftp.list(@config.remote_downsamples_dir).select {|f| File.fnmatch? "*.bmp", f }.map {|f| File.basename f }
    end
    
    def files_to_be_uploaded
      downsampled_files - fully_uploaded_files
    end

    def available_gigabytes
      if ENV["HOSTNAME"] = "heart.comlab"
        `df`.split("\n")[-1].split[3].to_i / 1000000
      else
        puts "Warning: available space is not checked on this system."
        10
      end
    end
    
    def add_error_file(filename)
        File.open(File.join(@config.local_dataset_dir, "error_files.txt"), "a") {|f| f.puts filename }
        puts "Downsampling failed. Filename has been appended to 'error_files.txt'."
    end
  end
end
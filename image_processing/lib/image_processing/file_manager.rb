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
    
    def all_files
      @all_files ||= @ftp.list(@config.remote_originals_dir).select {|f| File.fnmatch? "*.bmp", f }.map {|f| File.basename f }
    end
    
    def fully_downloaded_files
      Dir[File.join(@config.local_originals_dir, '*.bmp')].map {|f| File.basename f }
    end

    def downsampled_files
      Dir[File.join(@config.local_downsamples_dir, '*.bmp')].map {|f| File.basename f }
    end

    def files_to_be_downsampled
      all_files - downsampled_files
    end
    
    def files_to_be_downloaded
      # Finds all files, and removes files that have been downsampled and
      # files that are already fully downloaded. Ignores and overwrites files
      # that are partially downloaded.
      files_to_be_downsampled - fully_downloaded_files
    end
    
    def available_gigabytes
      if ENV["HOSTNAME"] = "heart.comlab"
        `df`.split("\n")[-1].split[3].to_i / 1000000
      else
        puts "Warning: available space is not checked on this system."
        10
      end
    end
    
  end
end
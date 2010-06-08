module ImageProcessing
  class FileManager
    def initialize(config, ftp)
      @config, @ftp = config, ftp
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

    def files_to_be_downloaded
      # Finds all files, and removes files that have been downsampled and
      # files that are already fully downloaded. Ignores and overwrites files
      # that are partially downloaded.
      all_files - fully_downloaded_files - downsampled_files
    end

    def check_local_space
      while available_gigabytes < 5
        puts "Not enough space, trying again in a minute..."
        sleep 60
      end
    end
    
    def available_gigabytes
      `df`.split("\n")[-1].split[3].to_i / 1000000
    end
    
  end
end
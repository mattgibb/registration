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
    
    def remote_originals
      @remote_originals ||= image_basenames @ftp.list(@config.remote_originals_dir)
    end
    
    def local_originals
      image_basenames Dir[File.join(@config.local_originals_dir, '*')]
    end

    def local_downsamples
      image_basenames Dir[File.join(@config.local_downsamples_dir, '*')]
    end

    def remote_downsamples
      image_basenames @ftp.list(@config.remote_downsamples_dir)
    end
    
    def error_files
      error_list = File.join(@config.local_dataset_dir, "error_files.txt")
      File.exists?(error_list) ? File.read(error_list).split : []
    end
    
    def processed_files
      local_downsamples + error_files
    end

    def originals_to_be_downsampled
      remote_originals - processed_files
    end
    
    def originals_to_be_downloaded
      originals_to_be_downsampled - local_originals
    end
    
    def originals_ready_to_be_downsampled
      local_originals - error_files
    end
    
    def downsamples_to_be_uploaded
      local_downsamples - remote_downsamples
    end
    
    def downsamples_to_be_downloaded
      remote_downsamples - local_downsamples
    end
    
    def available_gigabytes
      if ENV["HOSTNAME"] == "heart.comlab"
        `df`.split("\n")[-1].split[3].to_i / 1000000
      elsif ENV["HOSTNAME"] == "orac"
        # check how much space in /data
        `df`.split("\n")[-1].split[2].to_i / 1000000
      else
        STDERR.puts "Warning: available space is not checked on this system."
        10
      end
    end
    
    def add_error_file(filename)
        File.open(File.join(@config.local_dataset_dir, "error_files.txt"), "a") {|f| f.puts filename }
        puts "Downsampling failed. Filename has been appended to 'error_files.txt'."
    end
    
    private
    
      def is_an_image?(file)
        (file =~ /\.(bmp|jpg)$/i) ? true : false
      end
    
      def image_basenames(files)
        files.select do |f|
          is_an_image? f
        end.map do |f|
          File.basename f
        end
      end
    
  end
end

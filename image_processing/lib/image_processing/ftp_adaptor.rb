require 'net/ftp'
require 'fileutils'

module ImageProcessing
  class FtpAdaptor
    include FileUtils
    
    def initialize(config)
      # set up the ftp connection
      @host, @user, @password = config.host, config.user, config.password
      @ftp = Net::FTP.new(@host, @user, @password)
      @ftp.passive = true
    end
    
    def list(dir)
      @ftp.nlst(dir)
    end
    
    def download_file(source, destination)
      temp_destination = destination + ".part"
      check_connection
      print "Downloading #{File.basename(source)}..."
      @ftp.getbinaryfile(source, temp_destination)
      puts "done."
      print "Removing '.part' extension..."
      mv temp_destination, destination
      puts "done.\n\n"
    end
  
    def upload_file(source, destination)
      temp_destination = destination + ".part"
      print "Uploading #{File.basename(source)}..."
      check_connection
      @ftp.putbinaryfile(source, temp_destination)
      puts "done."
      print "Removing '.part' extension..."
      @ftp.rename(temp_destination, destination)
      puts "done.\n\n"
    end
    
    def mkdir(dir)
      @ftp.mkdir(dir)
    end
    
  private
    def check_connection
      begin
        @ftp.noop
      rescue StandardError => e
        puts "\n\n#{e}\n\n"
        print "Reconnecting to FTP server..."
        @ftp.connect(@host)
        @ftp.login(@user, @password)
        puts "done\n\n"
      end
    end
    
  end
end
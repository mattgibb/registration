require 'net/ftp'
require 'fileutils'
include FileUtils

class FtpAdapter
  def initialize(local_dir, host, user = nil, password = nil, remote_dir = nil)
    # place to put downloaded files
    @local_dir = local_dir
    
    # set up the ftp connection
    @host, @user, @password, @remote_dir = host, user, password, remote_dir
    @ftp = Net::FTP.new(@host, @user, @password)
    @ftp.chdir @remote_dir if @remote_dir    
  end
  
  def download_file(basename)
    check_connection
    whole_name = File.join(@local_dir, "originals", "#{basename}.bmp")
    part_name  = whole_name + ".part"
    print "Downloading #{basename}.bmp..."
    @ftp.getbinaryfile("#{basename}.bmp", part_name)
    puts "done."
    print "Removing '.part' extension..."
    mv part_name, whole_name
    puts "done."
  end
  
  def check_connection
    begin
      @ftp.noop
    rescue Net::FTPPermError => e
      puts e + "\n\n"
      print "Reconnecting to FTP server..."
      @ftp.connect @host
      @ftp.chdir @remote_dir if @remote_dir
      puts "done"
    end
  end
  
end

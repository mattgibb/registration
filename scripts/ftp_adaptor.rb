require 'net/ftp'
require 'fileutils'
include FileUtils

class FtpAdapter
  def initialize(host, user, password, remote_dir, params = {})
    # set up folders
    @download_dir, @upload_dir = params[:download_dir], params[:upload_dir]
    
    # set up the ftp connection
    @host, @user, @password, @remote_dir = host, user, password, remote_dir
    @ftp = Net::FTP.new(@host, @user, @password)
    @ftp.chdir @remote_dir if @remote_dir
    @ftp.passive = true
  end
  
  def download_file(basename)
    raise "no download directory specified in FTP adaptor" unless @download_dir
    check_local_space
    whole_name = File.join(@download_dir, "#{basename}.bmp")
    part_name  = whole_name + ".part"
    print "Downloading #{basename}.bmp..."
    check_connection
    @ftp.getbinaryfile("#{basename}.bmp", part_name)
    puts "done."
    print "Removing '.part' extension..."
    mv part_name, whole_name
    puts "done.\n\n"
  end
  
  def upload_file(basename)
    raise "no upload directory specified in FTP adaptor" unless @upload_dir
    whole_name = File.join(@upload_dir, basename)
    part_name = basename + ".part"
    print "Uploading #{basename}..."
    check_connection
    @ftp.putbinaryfile(whole_name, part_name)
    puts "done."
    print "Removing '.part' exension..."
    @ftp.rename(part_name, basename)
    puts "done.\n\n"
  end
  
  def check_local_space
    while available_gigabytes < 5
      puts "Not enough space, trying again in a minute..."
      sleep 60
    end
  end
  
  def check_connection
    begin
      @ftp.noop
    rescue Net::FTPError => e
      puts "Oops, an #{e} was raised.\n\n"
      print "Reconnecting to FTP server..."
      @ftp.connect @host
      @ftp.chdir @remote_dir if @remote_dir
      puts "done"
    end
  end
  
  def available_gigabytes
    `df`.split("\n")[-1].split[3].to_i / 1000000
  end
  
  def remote_files
    @ftp.list.map {|f| f.split[-1]}
  end
  
end

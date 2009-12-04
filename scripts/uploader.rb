#!/usr/bin/env ruby

DOWNSAMPLES_DIR = '../../images/downsamples_16x64x64'

def downsampled_files
  Dir[DOWNSAMPLES_DIR + '/*'].map {|f| File.basename f }
end

def fully_uploaded_files(ftp)
  ftp.remote_files.select { |f| not ( f =~ /\.part$/ ) }
end

def files_to_upload(ftp)
  downsampled_files - fully_uploaded_files(ftp)
end

# Initialise FTP connection
require 'ftp_adapter'

ftp = FtpAdapter.new('nas-mef2.physiol.ox.ac.uk', 'mef', 'meflab',
                     '/NAS-MEF2/Rabbit/001/Histology/downsampled_16x64x64',
                     upload_dir: DOWNSAMPLES_DIR)

ftp.upload_file files_to_upload(ftp)[0] until files_to_upload(ftp).empty?

puts "All files have been uploaded!"


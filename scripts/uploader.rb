#!/usr/bin/env ruby

IMAGES_DIR = '../../images'

def downsampled_files
  Dir[IMAGES_DIR + '/downsamples/*'].map {|f| File.basename f }  
end

def fully_uploaded_files(ftp)
  ftp.remote_files.select { |f| not ( f =~ /\.part$/ ) }
end

def files_to_upload(ftp)
  downsampled_files - fully_uploaded_files(ftp)
end

# Initialise FTP connection
require 'ftp_adapter'

ftp = FtpAdapter.new(IMAGES_DIR, 'nas-mef2.physiol.ox.ac.uk', 'mef', 'meflab',
                     '/NAS-MEF2/Rabbit/001/Histology/downsampled_32x128x128')

ftp.upload_file files_to_upload(ftp)[0] until files_to_upload(ftp).empty?

puts "All files have been uploaded!"

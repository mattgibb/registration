#!/usr/bin/env ruby

IMAGES_DIR = '../../images'

def all_files
  File.read("../config/all_files.txt").split
end

def downsampled_files
  # this code is repeated in downsampler
  Dir[IMAGES_DIR + '/downsamples/*.raw'].map {|f| File.basename f, '.raw' }
end

def fully_downloaded_files
  # this is the same code as files_to_downsample
  Dir[IMAGES_DIR + '/originals/*.bmp'].map {|f| File.basename f, '.bmp' }
end

def files_to_download
  # Finds all files, and removes files that have been downsampled
  # and files that are already fully downloaded. Ignores and overwrites
  # files that are partially downloaded.
  all_files - fully_downloaded_files - downsampled_files
end

# Initialise FTP connection
require 'ftp_adapter'

ftp = FtpAdapter.new(IMAGES_DIR, 'nas-mef2.physiol.ox.ac.uk', 'mef', 'meflab',
                     'NAS-MEF2/Rabbit/001/Histology/HiRes/')

ftp.download_file files_to_download[0] until files_to_download.empty?

puts "All files have been downloaded!"

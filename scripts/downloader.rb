#!/usr/bin/env ruby

IMAGES_DIR = '../../images'
ORIGINALS_DIR = IMAGES_DIR + '/originals'
DOWNSAMPLES_DIR = IMAGES_DIR + '/downsamples'
CONFIG_DIR = '../config'

def all_files
  File.read("#{CONFIG_DIR}/all_files.txt").split
end

def downsampled_files
  # this code is repeated in downsampler
  Dir[DOWNSAMPLES_DIR + '/*.bmp'].map {|f| File.basename f, '.bmp' }
end

def fully_downloaded_files
  # this is the same code as files_to_downsample
  Dir[ORIGINALS_DIR + '/*.bmp'].map {|f| File.basename f, '.bmp' }
end

def error_files
  File.read(CONFIG_DIR + "/error_files.txt").split
end

def files_to_download
  # Finds all files, and removes files that have been downsampled,
  # files that are already fully downloaded and files that could not
  # be processed. Ignores and overwrites files that are partially downloaded.
  all_files - fully_downloaded_files - downsampled_files - error_files
end

# Initialise FTP connection
require 'ftp_adaptor'

ftp = FtpAdapter.new('nas-mef2.physiol.ox.ac.uk', 'mef', 'meflab',
                     'NAS-MEF2/Rabbit/001/Histology/HiRes/', download_dir: ORIGINALS_DIR)
                     
# ftp.passive = true

ftp.download_file files_to_download[0] until files_to_download.empty?

puts "All files have been downloaded!"

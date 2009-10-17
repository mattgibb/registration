#!/usr/bin/env ruby

require 'fileutils'
include FileUtils

IMAGES_DIR = '../../images'

def all_files
  File.read("../config/all_files.txt").split
end

def downsampled_files
  # this code is repeated in 
  downsampled_files = Dir[IMAGES_DIR + '/downsamples/*.raw'].map {|f| File.basename f, '.raw' }  
end

def files_ready_to_be_downsampled
  # this is the same code as fully_downloaded_files
  Dir[IMAGES_DIR + '/originals/*.bmp'].map {|f| File.basename f, '.bmp' }
end

def files_to_downsample
  all_files - downsampled_files
end

def downsample_file(basename)
  print "Downsampling #{basename}.bmp..."
  `../itk/ShrinkImage #{IMAGES_DIR}/originals/#{basename}.bmp #{IMAGES_DIR}/downsamples/#{basename}.mhd`
  puts "done."
  print "Removing hi-res copy of #{basename}.bmp..."
  rm "#{IMAGES_DIR}/originals/#{basename}.bmp"
  puts "done.\n\n"
end

until files_to_downsample.empty?
  unless files_ready_to_be_downsampled.empty?
    downsample_file(files_ready_to_be_downsampled[0])
  else
    puts "No files ready to downsample yet, sleeping..."
    sleep 10
  end
end

puts "All files have been downsampled!"

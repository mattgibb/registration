#!/usr/bin/env ruby

IMAGES_DIR = '../../images'
DOWNSAMPLES_DIR = IMAGES_DIR + '/downsamples'
DODGY_DIR = IMAGES_DIR + '/dodgy_downsamples'

def print_differences(files)
  files.each do |f|
    puts "differences for #{f}:", `diff #{DODGY_DIR}/#{f} #{DOWNSAMPLES_DIR}/#{f}`
  end
end

def remove_dodgy_files(files)
  files.each do |f|
    `rm #{DOWNSAMPLES_DIR}/#{f}` if `diff #{DODGY_DIR}/#{f} #{DOWNSAMPLES_DIR}/#{f}`.empty?
  end
end

def common_files(params)
  # generate lists of filenames
  downsamples = Dir.chdir(DOWNSAMPLES_DIR) { Dir['*'] }
  dodgy       = Dir.chdir(DODGY_DIR)       { Dir['*'] }
  
  # find common files, sorted by when they were downsampled
  @both = downsamples & dodgy
  
  case params[:sorted_by]
  when :name  then @both.sort
  when :mtime then @both.sort_by {|f| File.mtime "#{DOWNSAMPLES_DIR}/#{f}"}
  when :atime then @both.sort_by {|f| File.atime "#{DOWNSAMPLES_DIR}/#{f}"}
  when :size  then @both.sort_by {|f| File.size  "#{DOWNSAMPLES_DIR}/#{f}"}
  end
  
end

# print out differences between dodgy files and newly downsampled files
print_differences(common_files sorted_by: :mtime)
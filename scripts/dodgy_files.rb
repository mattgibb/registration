#!/usr/bin/env ruby

IMAGES_DIR = '../../images'
DOWNSAMPLES_DIR = IMAGES_DIR + '/downsamples'
DODGY_DIR = IMAGES_DIR + '/dodgy_downsamples'
CANDIDATES_DIR = IMAGES_DIR + '/candidate_downsamples'

def common_files(params={})
  # generate lists of filenames
  downsamples = Dir.chdir(DOWNSAMPLES_DIR) { Dir['*'] }
  dodgy       = Dir.chdir(DODGY_DIR)       { Dir['*'] }
  
  # find common files
  @both = downsamples & dodgy
  
  # sort files
  case params[:sorted_by]
  when :mtime then @both.sort_by {|f| File.mtime "#{DOWNSAMPLES_DIR}/#{f}"}
  when :atime then @both.sort_by {|f| File.atime "#{DOWNSAMPLES_DIR}/#{f}"}
  when :size  then @both.sort_by {|f| File.size  "#{DOWNSAMPLES_DIR}/#{f}"}
  else             @both.sort # sorted by name
  end
end

def dodgy_files(params={})
  common_files(params).select do |f|
    `diff #{DODGY_DIR}/#{f} #{DOWNSAMPLES_DIR}/#{f}`.empty?
  end
end

def candidate_files(params={})
  common_files(params) - dodgy_files(params)
end

def remove_dodgy_files
  dodgy_files.each do |f|
    `rm #{DODGY_DIR}/#{f}`
  end
end

def copy_candidate_files
  candidate_files.each do |f|
    `cp #{DOWNSAMPLES_DIR}/#{f} #{CANDIDATES_DIR}/`
  end
end

def remove_checked_candidates
  checked_candidates = Dir.chdir(CANDIDATES_DIR) {Dir['*']}
  checked_candidates.each do |f|
    `rm #{DODGY_DIR}/#{f} #{CANDIDATES_DIR}/#{f}`
  end
end

#!/usr/bin/env ruby

IMAGES_DIR = '../../images'
DOWNSAMPLES_DIR = IMAGES_DIR + '/downsamples'
DODGY_DIR = IMAGES_DIR + '/dodgy_downsamples'

# generate lists of filenames
downsamples = dodgy = nil
downsamples = Dir.chdir(DOWNSAMPLES_DIR) { downsamples = Dir['*'] }
dodgy       = Dir.chdir(DODGY_DIR)       { dodgy       = Dir['*'] }

# find common files, sorted by when they were downsampled
both = downsamples & dodgy
both = both.sort_by {|f| File.mtime "#{DOWNSAMPLES_DIR}/#{f}"}

# print out differences between dodgy files and newly downsampled files
both.each {|f| puts "differences for #{f}:", `diff #{DODGY_DIR}/#{f} #{DOWNSAMPLES_DIR}/#{f}` }

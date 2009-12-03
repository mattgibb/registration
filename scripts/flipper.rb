#!/usr/bin/env ruby

require 'yaml'

def project_root
  File.expand_path(File.join(File.dirname(__FILE__), '..'))
end

def slice_info
  slice_info_path = "#{project_root}/config/slice_info.yml"
  YAML::load_file slice_info_path
end

def flip_file(file)
  `#{project_root}/itk/FlipImage #{file} #{file}`
end



flipped_files = slice_info.select do |filename, hash|
  hash[:flipped]
end

flipped_files.each do |filename, hash|
  path = File.expand_path(File.join(project_root, "../images/downsamples_16x64x64", "#{filename}.bmp"))  
  flip_file(path)
end

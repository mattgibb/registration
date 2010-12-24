require 'rake'
require 'fileutils'
include FileUtils::Verbose

task :default => [:make]

desc "Build C++ source"
task :make do
  system "cd itk_build && make"
end

desc "Build registered rat volumes from 2D histology and block face images"
task :build_volumes => [:make] do
  sh "itk_build/BuildVolumes Rat24 BuildVolumes"
  sh "say done"
end

desc "Run refactored code and test output against original output"
task :test_refactor => [:make] do
  rm Dir['results/Rat24/BuildVolumes_refactor/*']
  sh "itk_build/BuildVolumes Rat24 BuildVolumes_refactor"
  diff_output = `diff -r -x .DS_Store results/Rat24/BuildVolumes{,_refactor}`
  if $?.success?
    `echo The refactoring worked\! | growlnotify Success\!`
    puts "\nrefactoring successful!"
    `say refactoring successful!`
  else
    `echo '#{diff_output}' | growlnotify The refactoring fucked something\.`
    puts "\nDifferences:"
    puts diff_output
   `say refactoring failed`
  end
end

desc "Generate graph movies of registration iteration data"
task :movies do
  sh "graphing/registration_graphs.py #{test_dir}"
end

desc "Run ImageRegistration5 example"
task :example => [:make] do |variable|
  sh "itk_build/ImageRegistration5 " +
     "images/Rat24/LoRes/downsamples_8/10000.png " +
     "images/Rat24/HiRes/downsamples_64/10000.png " +
     "results/ImageRegistration5/ImageRegistration5.png"
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")

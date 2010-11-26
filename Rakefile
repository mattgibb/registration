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
  sh "itk_build/BuildVolumes " +
     "images/Rat24/LoRes/downsamples_8/ " +
     "images/Rat24/HiRes/downsamples_64/ " +
     "results/Rat24/LoRes/downsamples_8/deleteme"
  sh "say done"
end

desc "Generate graph movies of registration iteration data"
task :movies do
  sh "graphing/registration_graphs.py #{test_dir}"
end

# desc "Run refactored code and test output against original output"
# task :run_refactor do
#   register_128(refactor_dir)
#   Rake::Task[:test].invoke
#   cp 'config/registration_parameters_128.yml', refactor_dir
# end
# 
# desc "Test refactored output against original output"
# task :test do
#   diff_output = `diff -r -x .DS_Store #{test_dir} #{refactor_dir}`
#   if $?.success?
#     `echo The refactoring worked\! | growlnotify Success\!`
#     puts "\nrefactoring successful!"
#     `say refactoring successful!`
#   else
#     `echo '#{diff_output}' | growlnotify The refactoring fucked something\.`
#     puts "\nDifferences:"
#     puts diff_output
#    `say refactoring failed`
#   end
# end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")

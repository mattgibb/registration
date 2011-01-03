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

namespace :test do
  task :default => [:brain, :refactor] do; end
  
  def test_success(diff_output)
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
  
  desc "Run test on brain slice"
  task :brain => [:make] do
    rm Dir['itk_source/test/data/results/*']
    sh "itk_build/test/Brain"
    diff_output = `diff -r -x .DS_Store -x .gitignore itk_source/test/data/{,expected_}results`
    test_success(diff_output)
  end
  
  desc "Run refactored code and test output against original output"
  task :refactor => [:make] do
    rm Dir['results/Rat24/BuildVolumes_refactor/*']
    sh "itk_build/BuildVolumes Rat24 BuildVolumes_refactor"
    diff_output = `diff -r -x .DS_Store results/Rat24/BuildVolumes{,_refactor}`
    test_success(diff_output)
  end
end

desc "Generate graph movies of registration iteration data"
task :movies do
  sh "graphing/registration_graphs.py #{test_dir}"
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")

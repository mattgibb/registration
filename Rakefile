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
  task :default => [:brain, :refactor]
  
  def test_success(diff_output)
    if $?.success?
      puts "\nrefactoring successful!"
      begin
        # On mac, visual and audible notification
        `echo The refactoring worked\! | growlnotify Success\!`
        `say refactoring successful!`
      rescue
      end
    else
      begin
        # On mac, visual and audible notification
        `echo '#{diff_output}' | growlnotify The refactoring fucked something\.`
        `say refactoring failed`
      rescue
      end
      puts "\nDifferences:"
      puts diff_output
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

namespace :output do
  desc "Delete qsub job outputs"
  task :clean do
    sh "find . | egrep '\.[eo][0-9]{5}$' | xargs rm"
  end
  
  desc "Print standard outputs"
  task :stdout do
    sh "find . | egrep '\.o[0-9]{5}$' | xargs cat"
  end
  
  desc "Print standard errors"
  task :stderr do
    sh "find . | egrep '\.e[0-9]{5}$' | xargs cat"
  end
  
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")

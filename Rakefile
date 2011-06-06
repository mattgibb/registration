require 'rake'
require 'fileutils'
include FileUtils::Verbose

task :default => [:make]

desc "Build C++ source"
task :make do
  build_dir = ENV['HOST'] == 'sal' ? 'itk_build_sal' : 'itk_build'
  system "cd #{build_dir} && make"
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
    %x{rm -rf results/Rat24/BuildVolumes_refactor/*}
    sh "itk_build/BuildVolumes Rat24 BuildVolumes_refactor"
    diff_output = `diff -r -x .DS_Store results/Rat24/BuildVolumes{,_refactor}`
    test_success(diff_output)
  end
end

desc "Generate graph movies of registration iteration data"
task :movies do
  sh "graphing/registration_graphs.py #{test_dir}"
end

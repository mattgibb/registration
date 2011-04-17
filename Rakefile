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
  sh "itk_build/BuildVolumes Rat24 BuildVolumes 0250.bmp"
  begin
    sh "say done"
  rescue
  end
end

task :deformable_registration => [:make] do
  sh "itk_build/DeformableRegistration Rat24 BuildVolumes 0250.bmp"
  begin
    sh "say done"
  rescue
  end
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
    sh "itk_build/BuildVolumes Rat24 BuildVolumes_refactor 0250.bmp"
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

namespace :jobs do
  desc "display my jobs"
  task :show do
    sh "qstat | grep mattgibb"
  end

  desc "display the number of jobs left to complete"
  task :count do
    sh "qstat | grep mattgibb | wc -l"
  end

  desc "display a running number of jobs left to complete"
  task :counting do
    sh "while [ 1 ]; do qstat | grep mattgibb | wc -l; sleep 1; done"
  end
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")

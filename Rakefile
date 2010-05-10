require 'rake'

task :default => [:test_refactor]

test_dir = "../images/test_results"
refactor_dir = "../images/refactored_results"

def register(dir)
  sh "itk/Register ../images/downsamples_64x64 '^Slack[0-9]+\.bmp$' 0 " +
     "../images/mri/heart_bin.mhd #{dir}/finalParameters3D.transform " +
     "#{dir}/registered_mri.mhd #{dir}/stack.mhd #{dir}/mask.mhd " +
     "#{dir}/output.txt"
end

desc "Run Register and save results in #{test_dir}"
task :run do
  register(test_dir)
  sh "say done"
end

desc "Test refactored output against original output"
task :test_refactor do
  register(refactor_dir)
  # test to see if refactored output matches original
  diff_output = `diff -r -x .DS_Store #{test_dir} #{refactor_dir}`
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
  sh "graphing/registration_graphs.py #{test_dir}/output.txt #{test_dir}"
end

require 'rake'

task :default => [:test_Register]

testdir = "../images/test_results"

desc "Run Register and save results in #{testdir}"
task :test_Register do
  sh "itk/Register ../images/downsamples_16x64x64 '^Slack[0-9]+\.bmp$' 0 " +
     "../images/mri/heart_bin.mhd #{testdir}/finalParameters3D.transform " +
     "#{testdir}/registered_mri.mhd #{testdir}/stack.mhd #{testdir}/mask.mhd " +
     "#{testdir}/output.txt"
end

desc "Generate graph movies of registration iteration data"
task :movies do
  sh "scripts/graphing/registration_graphs.py #{testdir}/cropped_output_500_iterations #{testdir}"
end

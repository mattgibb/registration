# Binary command-line interface for the supercomputing cluster

require File.expand_path("../init", __FILE__)
require 'fileutils'
include FileUtils::Verbose

class Qsub < Thor
  include Thor::Actions
  
  BUILD_DIR = File.join PROJECT_ROOT, 'itk_release_sal' 
  PBS_DIR = File.join PROJECT_ROOT, 'pbs_scripts', 'sal'

  desc "make", "Build C++ source"
  def make
    run "cd #{BUILD_DIR} && make", :capture => false
  end

  desc "register_volumes DATASET OUTPUT_DIR [SLICE]", "build registered rat volumes from 2D histology and block face images"
  method_option :blockDir, :type => :string
  def register_volumes(dataset, output_dir, image="")
    invoke :make, []
    
    image_list_file = File.join PROJECT_ROOT, 'config', dataset, 'image_lists', 'image_list.txt'
    image_list = image.empty? ? File.read(image_list_file).split.uniq.join(' ') : image
    job_output_dir = File.join PROJECT_ROOT, 'results', dataset, output_dir, 'job_output'
    block_dir_flag = options.blockDir? ? "--blockDir #{options[:blockDir]}" : ""
    command = %{
      mkdir -p #{job_output_dir}
      cd #{job_output_dir} && \
      for image in #{image_list}
        do echo #{File.join PBS_DIR, 'register_volumes'} #{dataset} #{output_dir} --slice $image #{block_dir_flag} --stopAfterRigid | qsub -V -l walltime=0:015:00 -l select=1:mpiprocs=8 -N $image
      done}
    run command, :capture => false
    run "cp #{File.join PROJECT_ROOT, 'config', dataset, 'registration_parameters.yml'} #{File.join PROJECT_ROOT, 'results', dataset, output_dir}", :capture => false
  end
  
  desc "build_lores_volume DATASET OUTPUT_DIR", "generate reference LoRes colour volume"
  def build_lores_volume(dataset, output_dir)
    invoke :make, []
    run "echo #{File.join PBS_DIR, 'build_colour_volume'} #{dataset} #{output_dir} \
       --loResTransformsDir LoResTransforms_1_8 -H \
       | qsub -V -l walltime=0:015:00 -l select=1:mpiprocs=8 -N LoRes_vol", :capture => false
  end
  
  desc "build_hires_volume DATASET OUTPUT_DIR HIRES_TRANSFORMS_DIR", "generate registered HiRes colour volume"
  def build_hires_volume(dataset, output_dir, hires_transforms_dir)
    invoke :make, []
    run "echo #{File.join PBS_DIR, 'build_colour_volume'} #{dataset} #{output_dir} \
       --hiResTransformsDir #{hires_transforms_dir} -L \
       | qsub -V -l walltime=0:015:00 -l select=1:mpiprocs=8 -N HiRes_vol", :capture => false
  end

  desc "clear_jobs", "qdel all pending jobs"
  def clear_jobs
    `qstat | grep mattgibb`.split("\n").map do |line|
      # extract job numbers
      File.basename line.split[0], ".sal"
    end.each do |job_number|
      run "qdel #{job_number}", :capture => false
    end
  end
end

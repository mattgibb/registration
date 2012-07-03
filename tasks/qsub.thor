# Binary command-line interface for the supercomputing cluster

require File.expand_path("../jobs", __FILE__)
require 'fileutils'

class Qsub < Jobs
  PBS_DIR = File.join PROJECT_ROOT, 'pbs_scripts', 'sal'

  desc "register_volumes DATASET OUTPUT_DIR [SLICE]", "build registered rat volumes from 2D histology and block face images"
  method_option :blockDir, :type => :string
  method_option :transform, :type => :string
  def register_volumes(dataset, output_dir, image="")
    invoke :make, []
    
    images = image.empty? ? image_list(dataset).join(' ') : image
    job_output_dir = File.join results_path(dataset, output_dir), 'job_output'
    block_dir_flag = options.blockDir? ? "--blockDir #{options[:blockDir]}" : ""
    transform_flags = case options[:transform]
    when "rigid"
      "--stopAfterRigid"
    when "similarity"
      "--loadRigid --stopAfterSimilarity"
    when "affine"
      "--loadSimilarity"
    end
    command = %{
      mkdir -p #{job_output_dir}
      cd #{job_output_dir} && \
      for image in #{images}
        do echo #{File.join PBS_DIR, 'register_volumes'} #{dataset} #{output_dir} --slice $image #{block_dir_flag} #{transform_flags} | qsub -V -l walltime=0:015:00 -l select=1:mpiprocs=8 -N $image
      done}
    run command, :capture => false
    run "cp #{File.join PROJECT_ROOT, 'config', dataset, 'registration_parameters.yml'} #{File.join PROJECT_ROOT, 'results', dataset, output_dir}", :capture => false
  end
  
  desc "register_hires_pairs DATASET OUTPUT_DIR ITERATION [FIXED_BASENAME MOVING_BASENAME]", "Register adjacent HiRes images to each other"
  def register_hires_pairs(dataset, output_dir, i, fixed_basename=nil, moving_basename=nil)
    raise if fixed_basename.nil? != moving_basename.nil?
    i = Integer(i)
    
    invoke :make, []
    
    hires_pairs_path = File.join results_path(dataset, output_dir), "HiResPairs"
    
    unless fixed_basename
      return unless yes?("Are you sure you want to clear the old results?")
      
      # clear any old results
      %W(FinalTransforms IntermediateTransforms MetricValues OutputVolumes).each do |dir|
        rm_rf File.join(hires_pairs_path, dir, "CenteredAffineTransform_#{i}")
      end
      
      # if necessary, copy original registration transforms to AdjustedTransforms dir
      if i == 1
        run "mkdir -p #{hires_pairs_path}/AdjustedTransforms"
        run "cp -r #{results_path(dataset, output_dir)}/HiResTransforms_1_8/CenteredAffineTransform/ #{hires_pairs_path}/AdjustedTransforms/CenteredAffineTransform_0", :capture => false
      end
    end
    
    # construct array of pairs
    if fixed_basename
      image_pairs = [[fixed_basename, moving_basename]]
    else
      list = image_list(dataset)
      image_pairs = list[0..-2].zip list[1..-1]
    end
    
    job_output_path = File.join hires_pairs_path, "job_output/#{i}"
    run "mkdir -p #{job_output_path}", :capture => false
    command = lambda do |moving, fixed|
      "cd #{job_output_path} && " +
      "echo #{File.join PBS_DIR, 'register_hires_pairs'} #{dataset} #{output_dir} " +
      "HiResPairs/AdjustedTransforms/CenteredAffineTransform_#{i - 1} " +
      "CenteredAffineTransform_#{i} " +
      "--fixedBasename #{fixed} --movingBasename #{moving} " +
      "| qsub -V -l walltime=0:010:00 -l select=1:mpiprocs=8 -N #{moving}_#{fixed}"
    end
    
    # submit jobs
    image_pairs.each do |pair|
      run command.call(*pair), :capture => false
    end
    
    # copy parameters file to results
    run "cp #{File.join PROJECT_ROOT, 'config', dataset, 'HiRes_pair_parameters.yml'} #{hires_pairs_path}", :capture => false
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
       | qsub -V -l walltime=0:020:00 -l select=1:mpiprocs=8 -N HiRes_vol", :capture => false
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
  
  private
  def build_dir
    File.join PROJECT_ROOT, 'itk_release_sal' 
  end
  
end

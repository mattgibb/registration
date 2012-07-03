# Binary command-line interface for a mac
require 'yaml'
require File.expand_path("../jobs", __FILE__)

class Bin < Jobs
  desc "register_volumes DATASET OUTPUT_DIR [SLICE]", "build registered rat volumes from 2D histology and block face images"
  method_option :blockDir, :type => :string
  method_option :transform, :type => :string
  def register_volumes(dataset, output_dir, image="")
    invoke :make, []
    
    images = image.empty? ? image_list(dataset).join(' ') : image
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
      for image in #{images}
        do #{build_dir}/RegisterVolumes #{dataset} #{output_dir} $image #{block_dir_flag} #{transform_flags}
      done
    }
    
    run command, :capture => false
    run "say done"
    run "cp #{File.join PROJECT_ROOT, 'config', dataset, 'registration_parameters.yml'} #{File.join PROJECT_ROOT, 'results', dataset, output_dir}", :capture => false
  end
  
  desc "deformable_registration DATASET OUTPUT_DIR", "perform deformable registration, starting from affine bulk transforms"
  def deformable_registration(dataset, output_dir)
    invoke :make, []
    run "#{build_dir}/DeformableRegistration #{dataset} #{output_dir}", :capture => false
    run "say done"
  end
  
  desc "register_roi DATASET OUTPUT_DIR", "register region of interest"
  def register_roi(dataset, output_dir)
    invoke :make, []
    run "#{build_dir}/RegisterROI #{dataset} #{output_dir}", :capture => false
    run "say done"
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
      
      # on the first iteration, copy transforms from original registration to CenteredAffineTransform_0
      if i == 1
        say "Copying original registration results to CenteredAffineTransform_0..."
        run "mkdir -p #{hires_pairs_path}/AdjustedTransforms", capture: false
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
    
    command = lambda do |moving, fixed|
      "#{build_dir}/RegisterHiResPairs #{dataset} #{output_dir} " +
      "HiResPairs/AdjustedTransforms/CenteredAffineTransform_#{i - 1} " +
      "CenteredAffineTransform_#{i} " +
      "--fixedBasename #{fixed} --movingBasename #{moving}"
    end
    
    # run pair jobs
    image_pairs.each do |pair|
      run command.call(*pair), :capture => false
    end
    
    unless fixed_basename
      # copy parameters file to results
      run "cp #{File.join PROJECT_ROOT, 'config', dataset, 'HiRes_pair_parameters.yml'} #{hires_pairs_path}", :capture => false
      
      # build colour volume of new results
      invoke :compute_adjusted_transforms
      run "#{build_dir}/BuildColourVolume #{dataset} #{output_dir} -L --hiResTransformsDir HiResPairs/AdjustedTransforms/CenteredAffineTransform_#{i - 1}", :capture => false
    end
  end
  
  
  
  
  
  desc "compute_adjusted_transforms DATASET OUTPUT_DIR ITERATION", "compute adjusted transforms from registration results"
  def compute_adjusted_transforms(dataset, output_dir, i)
    i = Integer(i)
    invoke :make, []
    run "#{build_dir}/ComputeDiffusionTransforms #{dataset} #{output_dir} CenteredAffineTransform_#{i} --alpha=0.4", :capture => false
    run "#{build_dir}/ComposeTransformSeries #{dataset} #{output_dir} CenteredAffineTransform #{i}", :capture => false
  end
  
  desc "generate_all_noisy_transforms RESULTS_PREFIX", "generate transforms for identity (plain noise), translation signal, rotation signal and both"
  def generate_all_noisy_transforms(prefix)
    invoke :make, ["GenerateNoisyTransforms"]
    run "#{build_dir}/GenerateNoisyTransforms #{prefix} -n", :capture => false
    run "#{build_dir}/GenerateNoisyTransforms #{prefix}r -nr", :capture => false
    run "#{build_dir}/GenerateNoisyTransforms #{prefix}t -nt", :capture => false
    run "#{build_dir}/GenerateNoisyTransforms #{prefix}rt -nrt", :capture => false
    
    run "cd #{BUILD_DIR}; for result in #{prefix}{,r,t,rt}; do make BuildColourVolume && ./BuildColourVolume dummy $result -L --hiResTransformsDir HiResTransforms_#{downsample_suffix}/CenteredAffineTransform/; done", capture: false
  end
  
  private
  def downsample_suffix
    ratios_file = File.join PROJECT_ROOT, 'config/dummy/downsample_ratios.yml'
    ratios = YAML::load( File.open ratios_file )
    "#{ratios["LoRes"]}_#{ratios["HiRes"]}"
  end
  
  def build_dir
    dir = File.join PROJECT_ROOT, 'itk_release'
    dir += "_sal" if ENV["HOST"] == "sal"
    dir
  end
end

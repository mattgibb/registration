# Binary command-line interface for a mac

require File.expand_path("../init", __FILE__)

class Bin < Thor
  include Thor::Actions
  BUILD_DIR = File.join PROJECT_ROOT, 'itk_release'
  BUILD_DIR += "_sal" if ENV["HOST"] == "sal"
  
  desc "make", "Build C++ source"
  def make
    run "cd #{BUILD_DIR} && make", :capture => false
  end
  
  desc "register_volumes DATASET OUTPUT_DIR [SLICE]", "build registered rat volumes from 2D histology and block face images"
  def register_volumes(dataset, output_dir, slice="")
    invoke :make, []
    run "#{BUILD_DIR}/RegisterVolumes #{dataset} #{output_dir} #{slice}", :capture => false
    run "say done"
  end

  desc "deformable_registration DATASET OUTPUT_DIR", "perform deformable registration, starting from affine bulk transforms"
  def deformable_registration(dataset, output_dir)
    invoke :make, []
    run "#{BUILD_DIR}/DeformableRegistration #{dataset} #{output_dir}", :capture => false
    run "say done"
  end

  desc "register_roi DATASET OUTPUT_DIR", "register region of interest"
  def register_roi(dataset, output_dir)
    invoke :make, []
    run "#{BUILD_DIR}/RegisterROI #{dataset} #{output_dir}", :capture => false
    run "say done"
  end
  
  desc "diffusion_registration DATASET OUTPUT_DIR ITERATION", "iteratively apply diffusion registration"
  def diffusion_registration(dataset, output_dir, i)
    i = Integer(i)
    invoke :make, []
    # on the first iteration, copy transforms from original registration to CenteredAffineTransform_diffusion_0
    if i == 1
      run "mkdir -p #{results_path(dataset, output_dir)}/HiResPairs/AdjustedTransforms", capture: false
      run "cp -r #{results_path(dataset, output_dir)}/{HiResTransforms_1_8/CenteredAffineTransform/,HiResPairs/AdjustedTransforms/CenteredAffineTransform_diffusion_#{i - 1}}", :capture => false
    end
    run "#{BUILD_DIR}/RegisterHiResPairs #{dataset} #{output_dir} HiResPairs/AdjustedTransforms/CenteredAffineTransform_diffusion_#{i - 1}/ CenteredAffineTransform_diffusion_#{i}", :capture => false
    invoke :compute_adjusted_transforms
    run "#{BUILD_DIR}/BuildColourVolume #{dataset} #{output_dir} -L --hiResTransformsDir HiResPairs/AdjustedTransforms/CenteredAffineTransform_diffusion_#{i - 1}", :capture => false
  end
  
  desc "compute_adjusted_transforms DATASET OUTPUT_DIR ITERATION", "compute adjusted transforms from registration results"
  def compute_adjusted_transforms(dataset, output_dir, i)
    i = Integer(i)
    invoke :make, []
    run "#{BUILD_DIR}/ComputeDiffusionTransforms #{dataset} #{output_dir} CenteredAffineTransform_diffusion_#{i} --alpha=0.4", :capture => false
    run "#{BUILD_DIR}/ComposeTransformSeries #{results_path(dataset, output_dir)}/HiResPairs/{AdjustedTransforms/CenteredAffineTransform_diffusion_#{i - 1},{Diffusion,Adjusted}Transforms/CenteredAffineTransform_diffusion_#{i}}", :capture => false
  end
  
  private
  def results_path(dataset, output_dir)
    File.join PROJECT_ROOT, 'results', dataset, output_dir
  end
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")


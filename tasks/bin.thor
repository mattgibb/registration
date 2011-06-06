# Binary command-line interface for a mac

require './init'

class Bin < Thor
  include Thor::Actions
  BUILD_DIR = File.join PROJECT_ROOT, 'itk_build' 

  desc "make", "Build C++ source"
  def make
    run "cd #{BUILD_DIR} && make", :capture => false
  end
  
  desc "build_volumes DATASET OUTPUT_DIR [SLICE]", "build registered rat volumes from 2D histology and block face images"
  def build_volumes(dataset, output_dir, slice="")
    invoke :make
    run "#{BUILD_DIR}/BuildVolumes #{dataset} #{output_dir} #{slice}", :capture => false
    run "say done"
  end

  desc "deformable_registration DATASET OUTPUT_DIR", "perform deformable registration, starting from affine bulk transforms"
  def deformable_registration(dataset, output_dir)
    invoke :make
    run "#{BUILD_DIR}/DeformableRegistration #{dataset} #{output_dir}", :capture => false
    run "say done"
  end

  desc "register_roi DATASET OUTPUT_DIR", "register region of interest"
  def register_roi(dataset, output_dir)
    invoke :make
    run "#{BUILD_DIR}/RegisterROI #{dataset} #{output_dir}", :capture => false
    run "say done"
  end
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")


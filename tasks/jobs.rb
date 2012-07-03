require File.expand_path("../init", __FILE__)

class Jobs < Thor
  include FileUtils::Verbose
  include Thor::Actions
  
  desc "make", "Build C++ source"
  def make(*args)
    run "cd #{build_dir} && make #{args.join(" ")}", :capture => false
  end
  
  private
  def results_path(dataset, output_dir)
    File.join PROJECT_ROOT, 'results', dataset, output_dir
  end
  
  def image_list(dataset)
    image_list_file = File.join PROJECT_ROOT, 'config', dataset, 'image_lists', 'image_list.txt'
    File.read(image_list_file).split.uniq
  end
  
end

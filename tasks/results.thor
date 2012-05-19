require File.expand_path '../init', __FILE__

class Results < Thor
  include Thor::Actions

  RESULTS_DIR = File.join PROJECT_ROOT, 'results' 
  
  desc "pull [PATH]", "sync supercomputing results with local results directory"
  def pull(path="")
    run "rsync -azvPh -e 'ssh -t work ssh sal' :/home/comp-card/mattgibb/registration/results/#{path}/ #{RESULTS_DIR}/#{path}/", :capture => false
  end
end

class Qstat < Thor
  desc "show", "display my jobs"
  method_options :force => :boolean
  def show
    run "qstat | grep mattgibb", :capture => false
  end
  
  desc "count", "display the number of jobs left to complete"
  def count
    run "qstat | grep mattgibb | wc -l", :capture => false
  end
  
  desc "counting", "display a running number of jobs left to complete"
  def counting
    run "while [ 1 ]; do qstat | grep mattgibb | wc -l; sleep 2; done", :capture => false
  end
end

# String to label results folders with: Time.now.utc.strftime("%Y%m%d%H%M%S")

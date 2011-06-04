class Output < Thor
  include Thor::Actions
  
  desc "find [-e | -o]", "find qsub job output files."
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def find
    # second backslash needed to escape the backslash in the ruby string
    run "find . | egrep '\\.[#{regexp}][0-9]{5}$'", :capture => false
  end
  
  desc "clean", "Delete qsub job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def clean
    run "find . | egrep '\\.[#{regexp}][0-9]{5}$' | xargs rm", :capture => false
  end
  # 
  # desc "Print standard outputs"
  # task :stdout do
  #   sh "find . | egrep '\.o[0-9]{5}$' | xargs cat"
  # end
  # 
  # desc "Print standard errors"
  # task :stderr do
  #   sh "find . | egrep '\.e[0-9]{5}$' | xargs cat"
  # end
  # 
  private
  def regexp
    regexp = "eo"
    regexp = "e" if options.e?
    regexp = "o" if options.o?
    regexp
  end
end

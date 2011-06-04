class Output < Thor
  include Thor::Actions
  
  desc "find [-e | -o]", "find job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def find
    # second backslash needed to escape the backslash in the ruby string
    run "find . | egrep '\\.[#{regexp}][0-9]{5}$'", :capture => false
  end
  
  desc "print [-e | -o]", "print job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def print
    run "find . | egrep '\\.[#{regexp}][0-9]{5}$' | xargs cat", :capture => false
  end

  desc "clean [-e | -o]", "delete job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def clean
    run "find . | egrep '\\.[#{regexp}][0-9]{5}$' | xargs rm", :capture => false
  end
  
private
  def regexp
    regexp = "e" if options.e?
    regexp = "o" if options.o?
    regexp ||= "eo"
  end
end

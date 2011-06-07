require File.expand_path("../init", __FILE__)

class Output < Thor
  include Thor::Actions
  desc "find [-e | -o]", "find job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def find
    # second backslash needed to escape the backslash in the ruby string
    puts __FILE__
    run find_command, :capture => false
  end
  
  desc "show [-e | -o]", "print job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def show
    run find_command + " | xargs cat | less", :capture => false
  end

  desc "clean [-e | -o]", "delete job output files"
  method_options %w(stderr -e) => :boolean, %w(stdout -o) => :boolean
  def clean
    run find_command + " | xargs rm", :capture => false
  end
  
private
  def find_command
    "find #{PROJECT_ROOT} -not -path '*/.git/*' | egrep '\\.[#{regexp}][0-9]{5}$'"
  end

  def regexp
    regexp = "e" if options.stderr?
    regexp = "o" if options.stdout?
    regexp ||= "eo"
  end
end

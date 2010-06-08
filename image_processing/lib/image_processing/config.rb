require 'optparse'
require 'yaml'

module ImageProcessing
  class Config
    PROJECT_ROOT_DIR = File.expand_path(File.join(File.dirname(__FILE__), "../../.."))
    IMAGES_DIR = File.expand_path(File.join(PROJECT_ROOT_DIR, "..", "images"))
    CONFIG_DIR = File.join(PROJECT_ROOT_DIR, "config")
    
    attr_reader :host, :user, :password,
                :remote_originals_dir, :local_originals_dir,
                :remote_downsamples_dir, :local_downsamples_dir
    
    def initialize(argv)
      parse(argv)
      parse_server_config
      
      @local_originals_dir    = File.join(IMAGES_DIR, @dataset, "originals")
      @local_downsamples_dir  = File.join(IMAGES_DIR, @dataset, "downsamples_#{@downsample_ratio}")
      @remote_downsamples_dir = "#{@remote_originals_dir}_downsamples_#{@downsample_ratio}"
    end
    
  private
    def parse(argv)
      OptionParser.new do |opts|
        opts.banner = "Usage: downsampler [ options ]"
        opts.on("-h", "--help", "Show this message") do
          puts opts
          exit
        end
        opts.on('-z', "This is a z option") { puts "the z option was used" }
        begin
          # uncomment below if non-option arguments are required
          argv = ["-h"] if argv.empty?
          opts.parse!(argv)
        rescue OptionParser::ParseError => e
          STDERR.puts e.message, "\n", opts
          exit(-1)
        end
      end
      
      # argv is stripped of options by parse! method
      @dataset = argv[0]
      @downsample_ratio = argv[1].to_i
    end
    
    def parse_server_config
      server_config_path = File.join(CONFIG_DIR, @dataset, "server.yml")
      server = YAML::load_file(server_config_path)
      @host                 = server[:host]
      @user                 = server[:user]
      @password             = server[:password]
      @remote_originals_dir = server[:dir]
    end
  end
end
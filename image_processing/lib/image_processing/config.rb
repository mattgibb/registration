require 'optparse'
require 'yaml'

module ImageProcessing
  class Config
    PROJECT_ROOT_DIR = File.expand_path(File.join(File.dirname(__FILE__), "../../.."))
    ITK_DIR = File.join(PROJECT_ROOT_DIR, "itk_build")
    IMAGES_DIR = File.join(PROJECT_ROOT_DIR, "images")
    CONFIG_DIR = File.join(PROJECT_ROOT_DIR, "config")
    RESULTS_DIR = File.join(PROJECT_ROOT_DIR, "results")
    
    attr_reader :host, :user, :password,
                :remote_originals_dir, :local_originals_dir,
                :remote_downsamples_dir, :local_downsamples_dir,
                :local_dataset_dir, :local_output_dir,
                :downsample_ratio
    
    def initialize(argv)
      parse(argv)
      parse_server_config
      
      @local_dataset_dir      = File.join(IMAGES_DIR, @dataset)
      @local_originals_dir    = File.join(@local_dataset_dir, "originals")
      @local_downsamples_dir  = File.join(@local_dataset_dir, "downsamples_#{@downsample_ratio}")
      @local_output_dir       = File.join(RESULTS_DIR, @dataset, "downsamples_#{downsample_ratio}", "latest")
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

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "itkRGBPixel.h"
#include "itkCovariantVector.h"

// my files
#include "VolumeSplitter.hpp"

namespace po = boost::program_options;
using namespace boost::filesystem;

po::variables_map parse_arguments(int argc, char *argv[]);

int main( int argc, char *argv[] )
{
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
  // Process command line arguments
  create_directories(vm["outputDir"].as<string>());
  
  if(vm["pixelType"].as<string>() == "rgb")
  {
    VolumeSplitter< itk::RGBPixel< unsigned char > > splitter(vm);
    splitter.Split();
  }
  
  if(vm["pixelType"].as<string>() == "covariantVector")
  {
    VolumeSplitter< itk::CovariantVector< float > > splitter(vm);
    splitter.Split();
  }
  
  return EXIT_SUCCESS;
}


po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputFile", po::value<string>(), "input volume")
      ("outputDir", po::value<string>(), "directory to contain output slices")
      ("pixelType,p", po::value<string>()->default_value("rgb"), "type of image pixel e.g. rgb, unsigned char, float etc.")
      ("sliceDimension,d", po::value<unsigned int>()->default_value(0), "dimension perpendicular to slices")
      ("outputExtension,e", po::value<string>()->default_value("bmp"), "filetype extension of output slices")
      ("latex,l", po::bool_switch(), "shrink pixel spacings so images fit in a Latex document")
      ("slice,s", po::value<unsigned int>(), "pick a single slice number to output")
  ;
  
  po::positional_options_description p;
  p.add("inputFile", 1)
   .add("outputDir", 1);
  
  // parse command line
  po::variables_map vm;
	try
	{
  po::store(po::command_line_parser(argc, argv)
            .options(opts)
            .positional(p)
            .run(),
            vm);
	}
	catch (std::exception& e)
	{
	  cerr << "caught command-line parsing error" << endl;
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  po::notify(vm);
  
  // if help is specified, or positional args aren't present,
  // or more than one loadX flag
  if(    vm.count("help")
     || !vm.count("inputFile")
     || !vm.count("outputDir")
    )
  {
    cerr << "Usage: "
      << argv[0] << " [--inputFile=]volume.mha [--outputDir=]slices [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  // check sliceDim is 0, 1 or 2
  if(vm["sliceDimension"].as<unsigned int>() > 2)
  {
    cerr << "sliceDim must be 0, 1 or 2." << endl;
    exit(EXIT_FAILURE);
  }
  
  return vm;
}


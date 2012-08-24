// Rescale the intensity of a scalar image

// boost
#include "boost/program_options.hpp"

// itk
#include "itkRescaleIntensityImageFilter.h"

// my files
#include "IOHelpers.hpp"

namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // typedefs
  typedef unsigned char PixelType;
  typedef itk::Image< PixelType, 2 > ImageType;
  typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescalerType;
  
  // read image
  ImageType::Pointer image = readImage< ImageType >(vm["inputImage"].as<string>());
  
  // set up composer
  RescalerType::Pointer rescaler = RescalerType::New();
  rescaler->SetInput(image);
  rescaler->SetOutputMinimum(0);
  rescaler->SetOutputMaximum(vm["maximumIntensity"].as<unsigned int>());
  
  // save image
  ImageType::Pointer output = rescaler->GetOutput();
  writeImage< ImageType >(output, vm["outputImage"].as<string>());
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
    ("help,h", "produce help message")
    ("inputImage", po::value<string>(), "an image of the red channel intensities")
    ("outputImage", po::value<string>(), "the composed rgb image")
    ("maximumIntensity", po::value<unsigned int>()->default_value(255), "limit of intensity scaling")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
   .add("maximumIntensity", 1);
  
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
  
  // if help is specified, or positional args aren't present
  if (vm.count("help") ||
     !vm.count("inputImage") ||
     !vm.count("outputImage"))
  {
    cerr << "Usage: "
      << argv[0] << " [--inputImage=]dark.bmp [--outputImage=]bright.bmp [[--maximumIntensity=]output.bmp]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

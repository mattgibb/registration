// boost
#include "boost/program_options.hpp"

// itk
#include "itkImage.h"
#include "itkScalarToRGBColormapImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkRGBPixel.h"

// me
#include "IOHelpers.hpp"
 
namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

int main( int argc, char *argv[])
{
  po::variables_map vm = parse_arguments(argc, argv);
  
  // pixels and images
  typedef itk::RGBPixel<unsigned char>    RGBPixelType;
  typedef itk::Image<RGBPixelType, 2>  RGBImageType;
  typedef itk::Image<float, 2>  FloatImageType;
  
  // read in scalar image
  FloatImageType::Pointer image = readImage<FloatImageType>(vm["inputImage"].as<string>());
  
  typedef itk::ScalarToRGBColormapImageFilter<FloatImageType, RGBImageType> RGBFilterType;
  RGBFilterType::Pointer rgbFilter = RGBFilterType::New();
  rgbFilter->SetInput(image);
  rgbFilter->SetColormap( RGBFilterType::Jet );
  
  RGBImageType::Pointer colormap = rgbFilter->GetOutput();
  writeImage<RGBImageType>(colormap, vm["outputColormap"].as<string>());
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputImage", po::value<string>(), "input scalar image")
      ("outputImage", po::value<string>(), "output colormap")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
     ;
  
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
  if(vm.count("help") ||
  !vm.count("inputImage") ||
  !vm.count("outputImage")) {
    cerr << "Usage: "
      << argv[0] << " [--inputImage=]scalar.mha [--outputIntensity=]colormap.png "
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

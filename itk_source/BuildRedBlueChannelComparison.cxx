// Build an RGB image with pixel intensities from two scalar input images

#include "itkRGBPixel.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkComposeRGBImageFilter.h"

// boost
#include "boost/program_options.hpp"

// my files
#include "IOHelpers.hpp"

namespace po = boost::program_options;

// typedefs
typedef unsigned char ScalarPixelType;
typedef itk::RGBPixel< ScalarPixelType > RGBPixelType;
typedef itk::Image< ScalarPixelType, 2 > ScalarImageType;
typedef itk::Image< RGBPixelType, 2 > RGBImageType;
typedef itk::RGBToLuminanceImageFilter< RGBImageType, ScalarImageType > LuminanceFilterType;
typedef itk::ComposeRGBImageFilter< ScalarImageType, RGBImageType > ComposerType;

ScalarImageType::Pointer create_zero_image(ScalarImageType::RegionType region, ScalarImageType::SpacingType spacing);

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // read input images
  RGBImageType::Pointer redRGB  = readImage< RGBImageType >(vm["redInputImage"].as<string>());
  RGBImageType::Pointer blueRGB = readImage< RGBImageType >(vm["blueInputImage"].as<string>());
  
  // get red and blue luminances
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(redRGB);
  try{luminanceFilter->Update();}
  catch(itk::ExceptionObject& e)
  {
    cerr << "Exception caught while applying luminance filter" << endl;
    cerr << e.what();
    exit(EXIT_FAILURE);
  }
  
  ScalarImageType::Pointer red = luminanceFilter->GetOutput();
  red->DisconnectPipeline();
  
  luminanceFilter->SetInput(blueRGB);
  try{luminanceFilter->Update();}
  catch(itk::ExceptionObject& e)
  {
    cerr << "Exception caught while applying luminance filter" << endl;
    cerr << e.what();
    exit(EXIT_FAILURE);
  }
  
  ScalarImageType::Pointer blue = luminanceFilter->GetOutput();
  
  ScalarImageType::Pointer green = create_zero_image(red->GetLargestPossibleRegion(), red->GetSpacing());
  
  // create composite RGB image
  ComposerType::Pointer composer = ComposerType::New();
  composer->SetInput(0, red);
  composer->SetInput(1, green);
  composer->SetInput(2, blue);
  try{composer->Update();}
  catch(itk::ExceptionObject& e)
  {
    cerr << "Exception caught while composing images" << endl;
    cerr << e.what();
    exit(EXIT_FAILURE);
  }
  
  // save image
  RGBImageType::Pointer output = composer->GetOutput();
  writeImage< RGBImageType >(output, vm["outputImage"].as<string>());
  
  return EXIT_SUCCESS;
}

ScalarImageType::Pointer create_zero_image(ScalarImageType::RegionType region, ScalarImageType::SpacingType spacing)
{
  ScalarImageType::Pointer image = ScalarImageType::New();
  image->SetRegions(region);
  image->SetSpacing(spacing);
  image->Allocate();
  image->FillBuffer(0);
  return image;
}


po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("redInputImage", po::value<string>(), "an image of the red channel intensities")
      ("blueInputImage", po::value<string>(), "an image of the blue channel intensities")
      ("outputImage", po::value<string>(), "the composed rgb image")
  ;
  
  po::positional_options_description p;
  p.add("redInputImage", 1)
   .add("blueInputImage", 1)
   .add("outputImage", 1);
  
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
  if (vm.count("help") || !vm.count("redInputImage") || !vm.count("blueInputImage") || !vm.count("outputImage")) {
    cerr << "Usage: "
      << argv[0] << " [--redInputImage=]red.bmp [--blueInputImage=]blue.bmp [--outputImage=]output.bmp [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

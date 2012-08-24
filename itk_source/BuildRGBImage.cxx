// Build an RGB image with three scalar input images

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
  
  // set up composer
  ComposerType::Pointer composer = ComposerType::New();
  
  // three channels
  vector< string > channels;
  channels.push_back("red");
  channels.push_back("green");
  channels.push_back("blue");
  
  // collect region and spacing for zero images
  ScalarImageType::RegionType region;
  ScalarImageType::SpacingType spacing;
  
  for(unsigned int i=0; i<3; ++i)
  {
    // if channel is supplied at the command line, add it to RGB image
    if(vm.count(channels[i] + "InputImage"))
    {
      ScalarImageType::Pointer image = readImage< ScalarImageType >(vm[channels[i] + "InputImage"].as<string>());
      composer->SetInput(i, image);
      
      // collect region and spacing for zero images
      region  = image->GetLargestPossibleRegion();
      spacing = image->GetSpacing();
    }
  }
  
  // create zero images for missing channels
  for(unsigned int i=0; i<3; ++i)
  {
    // if channel isn't supplied at the command line, create zero image
    if(!vm.count(channels[i] + "InputImage"))
    {
      composer->SetInput(i, create_zero_image(region, spacing));
    }
  }
  
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
      ("greenInputImage", po::value<string>(), "an image of the green channel intensities")
      ("blueInputImage", po::value<string>(), "an image of the blue channel intensities")
      ("outputImage", po::value<string>(), "the composed rgb image")
  ;
  
  po::positional_options_description p;
  p.add("redInputImage", 1)
   .add("greenInputImage", 1)
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
  if (vm.count("help") ||
      (!vm.count("redInputImage") && !vm.count("blueInputImage") && !vm.count("outputImage")) )
  {
    cerr << "Usage: "
      << argv[0] << " [--redInputImage=]red.bmp [--blueInputImage=]blue.bmp [--outputImage=]output.bmp [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

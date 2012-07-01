// Crop an image by specifying starting index and size

#include "itkRGBPixel.h"
#include "itkExtractImageFilter.h"

// boost
#include "boost/program_options.hpp"

// my files
#include "IOHelpers.hpp"

namespace po = boost::program_options;

// typedefs
typedef itk::RGBPixel< unsigned char > RGBPixelType;
typedef itk::Image< RGBPixelType, 2 > RGBImageType;
typedef itk::ExtractImageFilter< RGBImageType, RGBImageType > CropperType;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // read input image
  RGBImageType::Pointer input = readImage< RGBImageType >(vm["inputImage"].as<string>());
  
  // construct image subregion
  RGBImageType::IndexType index;
  index[0] = vm["xIndex"].as<unsigned int>();
  index[1] = vm["yIndex"].as<unsigned int>();
  RGBImageType::SizeType size;
  size[0] = vm["xSize"].as<unsigned int>();
  size[1] = vm["ySize"].as<unsigned int>();
  RGBImageType::RegionType region;
  region.SetIndex(index);
  region.SetSize(size);
  
  // extract subimage
  CropperType::Pointer cropper = CropperType::New();
  cropper->SetInput(input);
  cropper->SetExtractionRegion(region);
  
  try{cropper->Update();}
  catch(itk::ExceptionObject& e)
  {
    cerr << "Exception caught while cropping image" << endl;
    cerr << e.what();
    exit(EXIT_FAILURE);
  }
  
  // save image
  RGBImageType::Pointer output = cropper->GetOutput();
  writeImage< RGBImageType >(output, vm["outputImage"].as<string>());
  
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
      ("xIndex", po::value<unsigned int>(), "starting index in the x dimension")
      ("yIndex", po::value<unsigned int>(), "starting index in the y dimension")
      ("xSize", po::value<unsigned int>(), "number of pixels in the x dimension")
      ("ySize", po::value<unsigned int>(), "number of pixels in the y dimension")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
   .add("xIndex", 1)
   .add("yIndex", 1)
   .add("xSize", 1)
   .add("ySize", 1);
  
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
    !vm.count("outputImage") ||
    !vm.count("xIndex") ||
    !vm.count("yIndex") ||
    !vm.count("xSize") ||
    !vm.count("ySize")) {
    cerr << "Usage: "
      << argv[0] << " [--inputImage=]big.bmp [--outputImage=]small.bmp "
      << "[--xIndex=]10 [--yIndex=]10 "
      << "[--xSize=]20 [--ySize=]20 "
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

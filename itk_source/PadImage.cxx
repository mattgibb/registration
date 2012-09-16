// Pad an image with a constant pixel value
// by specifying the padding dimensions

#include "itkRGBPixel.h"
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkImageRegionIterator.h"
 
// boost
#include "boost/program_options.hpp"

// my files
#include "IOHelpers.hpp"

namespace po = boost::program_options;

// typedefs
typedef itk::RGBPixel< unsigned char > RGBPixelType;
typedef itk::Image<RGBPixelType, 2>  ImageType;

po::variables_map parse_arguments(int argc, char *argv[]);

template<typename ImageType>
void doPadImage(po::variables_map vm, typename ImageType::SizeType lowerExtendSize, typename ImageType::SizeType upperExtendSize);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // run 2d padding
  // put here for extensibility to optional 3d
  // build padding regions
  ImageType::SizeType lowerExtendSize, upperExtendSize;
  lowerExtendSize[0] = vm["lower-x"].as<unsigned int>();
  lowerExtendSize[1] = vm["lower-y"].as<unsigned int>();
  upperExtendSize[0] = vm["upper-x"].as<unsigned int>();
  upperExtendSize[1] = vm["upper-y"].as<unsigned int>();
  
  doPadImage<ImageType>(vm, lowerExtendSize, upperExtendSize);
  
  return EXIT_SUCCESS;
}

template<typename ImageType>
void doPadImage(po::variables_map vm, typename ImageType::SizeType lowerExtendSize, typename ImageType::SizeType upperExtendSize)
{
  typedef itk::ConstantPadImageFilter <ImageType, ImageType> PadderType;
  
  // read input image
  typename ImageType::Pointer input = readImage< ImageType >(vm["inputImage"].as<string>());
  
  // set up padder
  typename PadderType::Pointer padder = PadderType::New();
  padder->SetInput(input);
  padder->SetPadLowerBound(lowerExtendSize);
  padder->SetPadUpperBound(upperExtendSize);
  padder->SetConstant(vm["pixelValue"].as<unsigned int>());
  
  // update pipeline
  try{padder->Update();}
  catch(itk::ExceptionObject& e)
  {
    cerr << "Exception caught while cropping image" << endl;
    cerr << e.what();
    exit(EXIT_FAILURE);
  }
  
  // save image
  typename ImageType::Pointer output = padder->GetOutput();
  writeImage< ImageType >(output, vm["outputImage"].as<string>());
  
}

void print_usage_and_exit(char *argv[], po::options_description& opts)
{
  cerr << "Usage: "
    << argv[0] << " [--inputImage=]small.bmp [--outputImage=]big.bmp "
    << "[--pixelValue=]255 "
    << "[--lower-x=]10 [--upper-x=]20 "
    << "[--lower-y=]10 [--upper-y=]20 "
    // << "[--lower-z=]10 [--upper-z=]20 "
    << endl << endl;
  cerr << opts << "\n";
  exit(EXIT_FAILURE);
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputImage", po::value<string>(), "an image of the red channel intensities")
      ("outputImage", po::value<string>(), "the composed rgb image")
      ("pixelValue", po::value<unsigned int>(), "value assigned to padding pixels outside the image")
      ("lower-x", po::value<unsigned int>(), "number of pixels to pad on the left")
      ("lower-y", po::value<unsigned int>(), "number of pixels to pad on the top")
      ("upper-x", po::value<unsigned int>(), "number of pixels to pad on the right")
      ("upper-y", po::value<unsigned int>(), "number of pixels to pad on the bottom")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
   .add("pixelValue", 1)
   .add("lower-x", 1)
   .add("upper-x", 1)
   .add("lower-y", 1)
   .add("upper-y", 1)
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
    !vm.count("outputImage") ||
    !vm.count("lower-x") ||
    !vm.count("upper-x") ||
    !vm.count("lower-y") ||
    !vm.count("upper-y"))
  {
    print_usage_and_exit(argv, opts);
  }
  
  // check dimensions
  // unsigned int dim = vm["dimension"].as<unsigned int>();
  // cerr << "dim: " << dim << endl;
  // if(dim == 2){}
  // else if(dim == 3)
  // {
  //   if( !vm.count("zIndex") || !vm.count("zSize") )
  //   {
  //     cerr << "zIndex or zSize are missing\n";
  //     print_usage_and_exit(argv, opts);
  //   }
  // }
  // else
  // {
  //   cerr << "dimension needs to be either 2 or 3\n";
  //   print_usage_and_exit(argv, opts);
  // }
  return vm;
}

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

po::variables_map parse_arguments(int argc, char *argv[]);

template<typename ImageType>
void doCropImage(po::variables_map vm, typename ImageType::IndexType index, typename ImageType::SizeType size);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // run 2d or 3d cropping
  unsigned int dim = vm["dimension"].as<unsigned int>();
  if(dim == 2)
  {
    typedef itk::Image< RGBPixelType, 2 > ImageType;
    ImageType::IndexType index;
    index[0] = vm["xIndex"].as<unsigned int>();
    index[1] = vm["yIndex"].as<unsigned int>();
    ImageType::SizeType size;
    size[0] = vm["xSize"].as<unsigned int>();
    size[1] = vm["ySize"].as<unsigned int>();
    doCropImage<ImageType>(vm, index, size);
  }
  if(dim == 3)
  {
    typedef itk::Image< RGBPixelType, 3 > ImageType;
    ImageType::IndexType index;
    index[0] = vm["xIndex"].as<unsigned int>();
    index[1] = vm["yIndex"].as<unsigned int>();
    index[2] = vm["zIndex"].as<unsigned int>();
    ImageType::SizeType size;
    size[0] = vm["xSize"].as<unsigned int>();
    size[1] = vm["ySize"].as<unsigned int>();
    size[2] = vm["zSize"].as<unsigned int>();
    doCropImage<ImageType>(vm, index, size);
  }
  
  return EXIT_SUCCESS;
}

template<typename ImageType>
void doCropImage(po::variables_map vm, typename ImageType::IndexType index, typename ImageType::SizeType size)
{
  typedef itk::ExtractImageFilter< ImageType, ImageType > CropperType;
  
  // read input image
  typename ImageType::Pointer input = readImage< ImageType >(vm["inputImage"].as<string>());
  
  // construct image subregion
  typename ImageType::RegionType region;
  region.SetIndex(index);
  region.SetSize(size);
  
  // extract subimage
  typename CropperType::Pointer cropper = CropperType::New();
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
  typename ImageType::Pointer output = cropper->GetOutput();
  writeImage< ImageType >(output, vm["outputImage"].as<string>());
  
}

void print_usage_and_exit(char *argv[], po::options_description& opts)
{
  cerr << "Usage: "
    << argv[0] << " [--inputImage=]big.bmp [--outputImage=]small.bmp "
    << "[--dimension=]2 "
    << "[--xIndex=]10 [--xSize=]20 "
    << "[--yIndex=]10 [--ySize=]20 "
    << "[--zIndex=]10 [--zSize=]20 "
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
      ("dimension", po::value<unsigned int>(), "dimensionality of the image")
      ("xIndex", po::value<unsigned int>(), "starting index in the x dimension")
      ("xSize", po::value<unsigned int>(), "number of pixels in the x dimension")
      ("yIndex", po::value<unsigned int>(), "starting index in the y dimension")
      ("ySize", po::value<unsigned int>(), "number of pixels in the y dimension")
      ("zIndex", po::value<unsigned int>(), "starting index in the y dimension")
      ("zSize", po::value<unsigned int>(), "number of pixels in the y dimension")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
   .add("dimension", 1)
   .add("xIndex", 1)
   .add("xSize", 1)
   .add("yIndex", 1)
   .add("ySize", 1)
   .add("zIndex", 1)
   .add("zSize", 1);
  
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
    !vm.count("xSize") ||
    !vm.count("yIndex") ||
    !vm.count("ySize"))
  {
    print_usage_and_exit(argv, opts);
  }
  
  // check dimensions
  unsigned int dim = vm["dimension"].as<unsigned int>();
  cerr << "dim: " << dim << endl;
  if(dim == 2){}
  else if(dim == 3)
  {
    if( !vm.count("zIndex") || !vm.count("zSize") )
    {
      cerr << "zIndex or zSize are missing\n";
      print_usage_and_exit(argv, opts);
    }
  }
  else
  {
    cerr << "dimension needs to be either 2 or 3\n";
    print_usage_and_exit(argv, opts);
  }
  return vm;
}

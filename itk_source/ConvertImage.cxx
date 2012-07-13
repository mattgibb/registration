// Load transforms and build RGB volumes from images

// boost
#include "boost/program_options.hpp"

//itk
#include "itkRGBPixel.h"

// my files
#include "IOHelpers.hpp"

namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

template<typename ImageType>
void convert_image(const string& input, const string& output)
{
  cout << "Reading image..." << flush;
  typename ImageType::Pointer image = readImage< ImageType >(input);
  cout << "done." << endl;
  
  cout << "Writing image..." << flush;
  writeImage< ImageType >(image, output);
  cout << "done." << endl;
}

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  string pixelType = vm["pixelType"].as<string>();
  unsigned int dim = vm["dimension"].as<unsigned int>();
  
  // RGB pixel types
  if(pixelType == "rgb")
  {
    typedef itk::RGBPixel< unsigned char > PixelType;
    
    if(dim == 2)
    {
      typedef itk::Image< PixelType, 2 > ImageType;
      convert_image< ImageType >(vm["inputImage"].as<string>(), vm["outputImage"].as<string>());
    }
    
    if(dim == 3)
    {
      typedef itk::Image< PixelType, 3 > ImageType;
      convert_image< ImageType >(vm["inputImage"].as<string>(), vm["outputImage"].as<string>());
    }
    
  }
  
  if(pixelType == "float")
  {
    typedef float PixelType;
    
    if(dim == 2)
    {
      typedef itk::Image< PixelType, 2 > ImageType;
      convert_image< ImageType >(vm["inputImage"].as<string>(), vm["outputImage"].as<string>());
    }
    
    if(dim == 3)
    {
      typedef itk::Image< PixelType, 3 > ImageType;
      convert_image< ImageType >(vm["inputImage"].as<string>(), vm["outputImage"].as<string>());
    }
    
  }
  
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
      ("pixelType", po::value<string>(), "either 'rgb' or 'float'")
      ("dimension", po::value<unsigned int>(), "number of dimensions in the image")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
   .add("pixelType", 1)
   .add("dimension", 1);
  
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
    !vm.count("pixelType") ||
    !vm.count("dimension")) {
    cerr << "Usage: "
      << argv[0] << " [--inputImage=]big.bmp [--outputImage=]small.bmp "
      << "[--pixelType=]rgb [--dimension=]2 "
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  // check dimension
  unsigned int dim = vm["dimension"].as<unsigned int>();
  if(dim < 2 || dim > 3)
  {
    cerr << "dimension must be 2 or 3" << endl;
    exit(EXIT_FAILURE);
  }
  
  // check pixel type
  string pixelType = vm["pixelType"].as<string>();
  if(pixelType != "rgb" && pixelType != "float")
  {
    cerr << "pixelType must be 'rgb' or 'float'" << endl;
    exit(EXIT_FAILURE);
  }
  return vm;
}

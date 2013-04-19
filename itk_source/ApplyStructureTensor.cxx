// Apply structure tensor to 3D volume

// boost
#include "boost/program_options.hpp"

//itk

// my files
#include "itkStructureTensorImageFilter.h"
#include "IOHelpers.hpp"

namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

template<typename ImageType>
void apply_structure_tensor(const string& input, const string& output, const double sigma)
{
  cout << "Reading image..." << flush;
  typename ImageType::Pointer image = readImage< ImageType >(input);
  cout << "done." << endl;
  
  typedef itk::StructureTensorImageFilter< ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetSigma(sigma);
  filter->SetInput(image);
  
  cout << "Calculating structure tensor image..." << flush;
  filter->Update();
  cout << "done." << endl;
  
  typedef typename FilterType::OutputImageType TensorImageType;
  typename TensorImageType::Pointer tensorImage = filter->GetOutput();
  
  cout << "Writing image..." << flush;
  writeImage< TensorImageType >(tensorImage, output);
  cout << "done." << endl;
}

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  unsigned int dim = vm["dimension"].as<unsigned int>();
  
  typedef float PixelType;
  
  if(dim == 2)
  {
    typedef itk::Image< PixelType, 2 > ImageType;
    apply_structure_tensor< ImageType >(vm["inputImage"].as<string>(), vm["outputImage"].as<string>(), vm["sigma"].as<double>() );
  }
  
  if(dim == 3)
  {
    typedef itk::Image< PixelType, 3 > ImageType;
    apply_structure_tensor< ImageType >(vm["inputImage"].as<string>(), vm["outputImage"].as<string>(), vm["sigma"].as<double>() );
  }
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputImage", po::value<string>(), "image to be converted")
      ("outputImage", po::value<string>(), "result")
      ("dimension", po::value<unsigned int>()->default_value(3), "number of dimensions in the image")
      ("sigma", po::value<double>()->default_value(1.0), "standard deviation of the Gaussian smoothing of the tensor image")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputImage", 1)
   .add("dimension", 1)
   .add("sigma", 1)
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
      << argv[0] << " [--inputImage=]big.bmp [--outputImage=]small.bmp "
      << "[[--dimension=]2] "
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
  
  return vm;
}

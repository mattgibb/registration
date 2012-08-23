// Apply structure tensor to 3D volume

// boost
#include "boost/program_options.hpp"

//itk
#include <itkSymmetricSecondRankTensor.h>

// my files
#include "IOHelpers.hpp"
#include "itkExtractLargestEigenvectorFilter.h"

namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	typedef itk::SymmetricSecondRankTensor< float, 3 > TensorType;
  typedef itk::Image< TensorType, 3 > TensorImageType;
  
  cout << "Reading image..." << flush;
  TensorImageType::Pointer image = readImage< TensorImageType >(vm["inputImage"].as<string>());
  cout << "done." << endl;
  
  typedef itk::ExtractLargestEigenvectorFilter< TensorImageType > FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(image);
  
  cout << "Extracting largest eigenvectors..." << flush;
  filter->Update();
  cout << "done." << endl;
  
  typedef FilterType::VectorImageType VectorImageType;
  VectorImageType::Pointer vectorImage = filter->GetOutput();
  
  cout << "Writing image..." << flush;
  writeImage< VectorImageType >(vectorImage, vm["outputImage"].as<string>());
  cout << "done." << endl;
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputImage", po::value<string>(), "input tensor image")
      ("outputImage", po::value<string>()->default_value(""), "output largest eigenvector image")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
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
  if(vm.count("help") ||
    !vm.count("inputImage") ||
    !vm.count("outputImage")) {
    cerr << "Usage: "
      << argv[0] << " [--inputImage=]tensor.vtk [--outputImage=]largesteigenvector.vtk "
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

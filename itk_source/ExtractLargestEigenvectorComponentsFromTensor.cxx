// Extract absolute magnitudes of components of largest eigenvector
// of a tensor image

// boost
#include "boost/program_options.hpp"

//itk
#include <itkSymmetricSecondRankTensor.h>
#include "itkVectorIndexSelectionCastImageFilter.h"

// my files
#include "IOHelpers.hpp"
#include "itkExtractLargestEigenvectorFilter.h"

namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	typedef itk::SymmetricSecondRankTensor< float, 3 > TensorType;
  typedef itk::Image< TensorType, 3 > TensorImageType;
  
  // read tensor image
  cout << "Reading image..." << flush;
  TensorImageType::Pointer image = readImage< TensorImageType >(vm["inputImage"].as<string>());
  cout << "done." << endl;
  
  // extract eigenvector
  typedef itk::ExtractLargestEigenvectorFilter< TensorImageType > EigenFilterType;
  EigenFilterType::Pointer eigenFilter = EigenFilterType::New();
  eigenFilter->SetInput(image);
  
  cout << "Extracting largest eigenvectors..." << flush;
  eigenFilter->Update();
  cout << "done." << endl;
  
  // vector to scalar extractor
  typedef itk::Image<float, 3> ScalarImageType;
  typedef itk::VectorIndexSelectionCastImageFilter< 
    EigenFilterType::VectorImageType,
    ScalarImageType>
      ComponentExtractorType;
  
  ComponentExtractorType::Pointer componentExtractor = ComponentExtractorType::New();
  componentExtractor->SetInput(eigenFilter->GetOutput());
  
  // write components
  for(unsigned int i=0; i<3; ++i)
  {
    // construct output file name
    stringstream output;
    output << vm["outputBase"].as<string>() << i << "." << vm["outputExtension"].as<string>();
    
    // write image
    cout << "Writing component " << i << "..." << flush;
    componentExtractor->SetIndex(i);
    ScalarImageType::Pointer scalarImage = componentExtractor->GetOutput();
    writeImage< ScalarImageType >(scalarImage, output.str());
    cout << "done." << endl;
  }
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputImage", po::value<string>(), "input tensor image")
      ("outputBase", po::value<string>()->default_value("eigencomponent"), "root name of each scalar component output image")
      ("outputExtension", po::value<string>()->default_value("mha"), "extension of scalar component output images")
  ;
  
  po::positional_options_description p;
  p.add("inputImage", 1)
   .add("outputBase", 1)
   .add("outputExtension", 1)
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
  !vm.count("inputImage")) {
    cerr << "Usage: "
      << argv[0] << " [--inputImage=]tensor.vtk [[--outputBase=]largest_eigenvector_component] "
      << " [[--outputExtension=]vtk]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

// Generate transforms to diffuse each slice toward its neighbours
// based on the final transforms of the HiRes pair registrations

// boost
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "itkCenteredAffineTransform.h"

#include "Dirs.hpp"
#include "IOHelpers.hpp"

namespace po = boost::program_options;
using namespace boost::filesystem;
using namespace boost;

typedef itk::CenteredAffineTransform< double, 2 > TransformType;

TransformType::Pointer squareRoot(TransformType::Pointer transform);

// function declarations
po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  const string transformsName = vm["transformsName"].as<string>();
  
  // set up pair transforms paths
  const string pairTransformsDir = Dirs::ResultsDir() + "HiResPairs/FinalTransforms/" + transformsName;
  vector< string > pairTransformBasenames = directoryContents(pairTransformsDir);
  vector< string > pairTransformPaths = constructPaths(pairTransformsDir, pairTransformBasenames);
  
  // read transforms
  vector< TransformType::Pointer > pairTransforms;
  
  for(unsigned int i=0; i<pairTransformPaths.size(); ++i)
  {
    TransformType::Pointer pairTransform
      = dynamic_cast< TransformType* >( readTransform(pairTransformPaths[i]).GetPointer() );
    assert(pairTransform);
    
    pairTransforms.push_back(pairTransform);
  }
  
  // write diffusion transforms
  const string diffusionTransformsDir = Dirs::ResultsDir() + "HiResPairs/DiffusionTransforms/" + transformsName + "/";
  remove_all(diffusionTransformsDir);
  create_directories(diffusionTransformsDir);
  
  for(unsigned int i=0; i<pairTransformPaths.size()-1; ++i)
  {
    // make sure two transforms share the same slice
    assert(pairTransformBasenames[i  ].substr(5,4) ==
           pairTransformBasenames[i+1].substr(0,4));
    
    // compose transform
    TransformType::Pointer diffusionTransform = TransformType::New();
    TransformType::Pointer belowTransform     = TransformType::New();
    pairTransforms[i]->GetInverse(belowTransform);
    TransformType::Pointer aboveTransform = pairTransforms[i+1];
    diffusionTransform->Compose( squareRoot(belowTransform) );
    diffusionTransform->Compose( squareRoot(aboveTransform) );
    
    // write transform
    string diffusionTransformPath = diffusionTransformsDir + pairTransformBasenames[i].substr(5,4);
    writeTransform(diffusionTransform, diffusionTransformPath);
  }
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("dataSet", po::value<string>(), "which rat to use")
      ("outputDir", po::value<string>(), "directory to place results")
      ("transformsName", po::value<string>(), "name of transform group")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("transformsName", 1)
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
    !vm.count("dataSet") ||
    !vm.count("outputDir") ||
    !vm.count("transformsName") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--transformsName=]CenteredAffineTransform_first_diffusion"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

// t = mv + o
// T = Mv + O
// t^2(v) = T(v)
// Mv + O = m(mv + o) + o
//        = m^2v + (m + I)o
// m = M^0.5
// o = (m + I)^(-1) O
TransformType::Pointer squareRoot(TransformType::Pointer transform)
{
  // extract matrix and offset
  const TransformType::MatrixType & M = transform->GetMatrix();
  const TransformType::OffsetType & O = transform->GetOffset();
  
  // compute square root matrix
  // http://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix
  double s = sqrt( M[0][0] * M[1][1] - M[1][0] * M[0][1] );
  double t = sqrt( M[0][0] + M[1][1] + 2 * s );
  
  TransformType::MatrixType m = M;
  m[0][0] += s;
  m[1][1] += s;
  m /= t;
  
  // compute square root offset
  TransformType::MatrixType offsetMatrix = m, inverseOffsetMatrix;
  offsetMatrix[0][0] += 1;
  offsetMatrix[1][1] += 1;
  inverseOffsetMatrix = offsetMatrix.GetInverse();
  TransformType::OffsetType o;
  for(unsigned int i=0; i<2; ++i)
  {
    for(unsigned int j=0; j<2; ++j)
    {
      o[i] += inverseOffsetMatrix[i][j] * O[j];
    }
  }
  
  // construct and return square root transform
  TransformType::Pointer root = TransformType::New();
  root->SetMatrix(m);
  root->SetOffset(o);
  return root;
}

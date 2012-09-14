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

// function declarations
po::variables_map parse_arguments(int argc, char *argv[]);
TransformType::Pointer squareRoot(TransformType::Pointer transform);
TransformType::Pointer interpolateTransforms(TransformType::Pointer a, TransformType::Pointer b, double alpha);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  const string transformsName = vm["transformsName"].as<string>();
  const double alpha = vm["alpha"].as<double>();
  
  // set up pair transforms paths
  const string pairTransformsDir = Dirs::ResultsDir() + "HiResPairs/FinalTransforms/" + transformsName;
  vector< string > pairTransformBasenames = directoryContents(pairTransformsDir);
  vector< string > pairTransformPaths = constructPaths(pairTransformsDir, pairTransformBasenames);
  
  // read transforms
  vector< TransformType::Pointer > pairTransforms;
  
  for(unsigned int i=0; i<pairTransformPaths.size(); ++i)
  {
    itk::TransformBase::Pointer bpPairTransform = readTransform(pairTransformPaths[i]);
    TransformType::Pointer pPairTransform
      = dynamic_cast< TransformType* >( bpPairTransform.GetPointer() );
    assert(pPairTransform);
    
    pairTransforms.push_back(pPairTransform);
  }
  
  // write diffusion transforms
  const string diffusionTransformsDir = Dirs::ResultsDir() + "HiResPairs/DiffusionTransforms/" + transformsName + "/";
  remove_all(diffusionTransformsDir);
  create_directories(diffusionTransformsDir);
  
  // first for middle transforms...
  for(unsigned int i=0; i<pairTransformPaths.size()-1; ++i)
  {
    // make sure two transforms share the same slice
    assert(pairTransformBasenames[i  ].substr(5,4) ==
           pairTransformBasenames[i+1].substr(0,4));
    
    // construct transform
    TransformType::Pointer belowTransform     = TransformType::New();
    pairTransforms[i]->GetInverse(belowTransform);
    TransformType::Pointer aboveTransform     = pairTransforms[i+1];
    TransformType::Pointer meanTransform      = interpolateTransforms(belowTransform, aboveTransform, 0.5);
    // interpolate by alpha from identity transform to meanTransform
    TransformType::Pointer diffusionTransform = TransformType::New();
    diffusionTransform = interpolateTransforms(diffusionTransform, meanTransform, 2 * alpha);
    
    // write transform
    string diffusionTransformPath = diffusionTransformsDir + pairTransformBasenames[i].substr(5,4);
    writeTransform(diffusionTransform, diffusionTransformPath);
  }
  
  // ...then for boundary transforms. Since the boundary slices
  // are just as likely to contain transformational noise, they are not fixed,
  // but instead diffuse toward their single neighbour, analagously to
  // zero-Neumann boundary conditions
  TransformType::Pointer bottomDiffusionTransform = TransformType::New();
  TransformType::Pointer topDiffusionTransform    = TransformType::New();
  TransformType::Pointer aboveBottomTransform     = pairTransforms[0];
  TransformType::Pointer belowTopTransform        = TransformType::New();
  pairTransforms[pairTransformPaths.size()-1]->GetInverse(belowTopTransform);
  
  bottomDiffusionTransform = interpolateTransforms(bottomDiffusionTransform, aboveBottomTransform, alpha);
  topDiffusionTransform    = interpolateTransforms(topDiffusionTransform,    belowTopTransform,    alpha);
  string bottomDiffusionTransformPath = diffusionTransformsDir + pairTransformBasenames[0].substr(0,4);
  string topDiffusionTransformPath    = diffusionTransformsDir + pairTransformBasenames[pairTransformPaths.size()-1].substr(5,4);
  writeTransform(bottomDiffusionTransform, bottomDiffusionTransformPath);
  writeTransform(topDiffusionTransform,    topDiffusionTransformPath   );
  
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
      ("alpha", po::value<double>(), "coefficient of diffusion")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("transformsName", 1)
   .add("alpha", 1)
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
    !vm.count("transformsName") ||
    !vm.count("alpha") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--transformsName=]CenteredAffineTransform_diffusion_1"
      << " [--alpha=]0.4"
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

TransformType::Pointer interpolateTransforms(TransformType::Pointer a, TransformType::Pointer b, double alpha)
{
  // extract matrices and offsets
  TransformType::MatrixType M_a = a->GetMatrix();
  TransformType::MatrixType M_b = b->GetMatrix();
  TransformType::OffsetType O_a = a->GetOffset();
  TransformType::OffsetType O_b = b->GetOffset();
  
  // construct new transform from linear interpolation of parameters
  TransformType::MatrixType M_c = M_a * (1 - alpha) + M_b * alpha;
  TransformType::OffsetType O_c = O_a * (1 - alpha) + O_b * alpha;
  
  TransformType::Pointer c = TransformType::New();
  c->SetMatrix(M_c);
  c->SetOffset(O_c);
  
  return c;
}

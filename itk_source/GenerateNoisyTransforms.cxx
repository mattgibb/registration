// generate transforms with normally distributed noise
// added to the parameters, in order to test the effectiveness
// of the diffusion transform algorithm

#define _USE_MATH_DEFINES
#include <cmath>

#include "boost/program_options.hpp"
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include "itkCenteredAffineTransform.h"

#include "Dirs.hpp"
#include "IOHelpers.hpp"
#include "Stack.hpp"
#include "HiResStackBuilder.hpp"
#include "StackTransforms.hpp"
#include "StackIOHelpers.hpp"

namespace po = boost::program_options;
using namespace boost::filesystem;
using namespace boost;

typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
typedef itk::CenteredAffineTransform< double, 2 > TransformType;

po::variables_map parse_arguments(int argc, char *argv[]);

template <typename T>
vector<T> sinspace(T a, T b, size_t N);

template <typename T>
vector<T> cosspace(T a, T b, size_t N);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	// set directories
  Dirs::SetDataSet( "dummy" );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  
  // initialise random number generator
  boost::mt19937 rng;
  rng.seed(static_cast<unsigned int>(std::time(0)));
  boost::normal_distribution<> nd(0.0, 1.0);
  boost::variate_generator<boost::mt19937&, 
                            boost::normal_distribution<> > varGen(rng, nd);
  
  // construct transform output paths
  string transformsDir = Dirs::HiResTransformsDir() + "CenteredAffineTransform/"; 
  create_directories( transformsDir );
  vector< string > paths = constructPaths(transformsDir, Dirs::ImageList());
  
  // initialise stack with correct spacings, sizes etc
  HiResStackBuilder<StackType> builder;
  shared_ptr<StackType> stack = builder.getStack();
  StackTransforms::InitializeToCommonCentre< StackType >( *stack );
  
  // convert to CenteredAffineTransform
  StackTransforms::InitializeFromCurrentTransforms< StackType, TransformType >( *stack );
  
  // initialise centre of rotation
  TransformType::CenterType center;
  center[0] = getSpacings<2>("LoRes")[0] * (double)getSize()[0] / 2.0;
  center[1] = getSpacings<2>("LoRes")[1] * (double)getSize()[1] / 2.0;
  
  vector<double> sine   = sinspace(0.0, 2 * M_PI, stack->GetSize());
  vector<double> cosine = cosspace(0.0, 2 * M_PI, stack->GetSize());
  
  // create identity transforms with added noise
  for(unsigned int slice_number=0; slice_number<stack->GetSize(); ++slice_number)
  {
    // extract transform
    StackType::TransformType::Pointer transform = stack->GetTransform(slice_number);
    
    // set centre of rotation
    StackTransforms::MoveCenter(static_cast< StackTransforms::LinearTransformType* >( transform.GetPointer() ), center);
    
    // scalings for different parameter types
    StackType::TransformType::ParametersType parameters = transform->GetParameters();
    StackType::TransformType::ParametersType scalings(8);
    scalings[0] = scalings[1] = scalings[2] = scalings[3] = 0.1; // matrix params
    scalings[4] = scalings[5] = 0;                                // centre of rotation params
    scalings[6] = scalings[7] = 1000;                              // translation params
    
    if(vm.count("noise"))
    {
      for(unsigned int i=0; i<8; ++i)
      {
        parameters[i] += varGen() * scalings[i];
      }
    }
    
    if(vm.count("rotation"))
    {
      double angle = cosine[slice_number];
      parameters[0] += cos(angle) - 1;
      parameters[1] -= sin(angle);
      parameters[2] += sin(angle);
      parameters[3] += cos(angle) - 1;
    }
    
    if(vm.count("translation"))
    {
      parameters[6] += 2000 * sine[slice_number];
      parameters[7] += 2000 * sine[slice_number];
    }
    
    transform->SetParameters(parameters);
    
    // write transform
    writeTransform(transform, paths[slice_number]);
  }
  
  Save(*stack, Dirs::HiResTransformsDir() + "CenteredAffineTransform/");
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("outputDir", po::value<string>(), "directory to source transforms and place results")
      ("translation,t", po::value<bool>()->zero_tokens(), "add translational sinusoidal signal")
      ("rotation,r",    po::value<bool>()->zero_tokens(), "add rotational sinusoidal signal")
      ("noise,n",       po::value<bool>()->zero_tokens(), "add noise")
  ;
  
  po::positional_options_description p;
  p.add("outputDir", 1);
  
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
  if (vm.count("help") || !vm.count("outputDir")) {
    cerr << "Usage: "
      << argv[0] << " [--outputDir=]my_dir [OPTIONS]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

template <typename T>
vector<T> linspace(T a, T b, size_t N) {
  T h = (b - a) / static_cast<T>(N-1);
  vector<T> xs(N);
  typename vector<T>::iterator x;
  T val;
  for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
    *x = val;
  return xs;
}

template <typename T>
vector<T> sinspace(T a, T b, size_t N) {
  vector<T> line = linspace(a, b, N);
  vector<T> sine(line.size());
  for(unsigned int i=0; i<sine.size(); ++i)
  {
    sine[i] = sin(line[i]);
  }
  return sine;
}

template <typename T>
vector<T> cosspace(T a, T b, size_t N) {
  vector<T> line = linspace(a, b, N);
  vector<T> cosine(line.size());
  for(unsigned int i=0; i<cosine.size(); ++i)
  {
    cosine[i] = cos(line[i]);
  }
  return cosine;
}

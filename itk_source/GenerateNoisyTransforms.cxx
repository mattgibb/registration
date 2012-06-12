// generate transforms with normally distributed noise
// added to the parameters, in order to test the effectiveness
// of the diffusion transform algorithm

#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include "itkCenteredAffineTransform.h"

#include "Dirs.hpp"
#include "IOHelpers.hpp"
#include "Stack.hpp"
#include "HiResStackBuilder.hpp"
#include "StackTransforms.hpp"
#include "StackIOHelpers.hpp"

using namespace boost::filesystem;
using namespace boost;

typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
typedef itk::CenteredAffineTransform< double, 2 > TransformType;

int main(int argc, char *argv[]) {
	// set directories
  Dirs::SetDataSet( "dummy" );
  Dirs::SetOutputDirName( "noisy_dummy" );
  
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
  
  // create identity transforms with added noise
  for(unsigned int slice_number=0; slice_number<stack->GetSize(); ++slice_number)
  {
    // extract transform
    StackType::TransformType::Pointer transform = stack->GetTransform(slice_number);
    
    // set centre of rotation
    StackTransforms::MoveCenter(static_cast< StackTransforms::LinearTransformType* >( transform.GetPointer() ), center);
    
    // add noise, scaled by parameter type
    StackType::TransformType::ParametersType parameters = transform->GetParameters();
    StackType::TransformType::ParametersType scalings(8);
    scalings[0] = scalings[1] = scalings[2] = scalings[3] = 0.02; // matrix params
    scalings[4] = scalings[5] = 0;                                // rotation params
    scalings[6] = scalings[7] = 100;                              // translation params
    
    for(unsigned int i=0; i<8; ++i)
    {
      parameters[i] += varGen() * scalings[i];
    }
    
    transform->SetParameters(parameters);
    
    // write transform
    writeTransform(transform, paths[slice_number]);
  }
  
  Save(*stack, Dirs::HiResTransformsDir() + "CenteredAffineTransform/");
  
  stack->updateVolumes();
  writeImage< StackType::VolumeType >(stack->GetVolume(), "/Users/Matt/Desktop/stack.mha");
  
  return EXIT_SUCCESS;
}

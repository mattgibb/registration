#include "boost/filesystem.hpp"

#include <assert.h>
#include "itkCenteredSimilarity2DTransform.h"

// my files
#include "Stack.hpp"
#include "NormalizeImages.hpp"
#include "StackInitializers.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "Profiling.hpp"

void checkUsage(int argc, char const *argv[]) {
  if( argc < 3 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet outputDir (slice)\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  string outputDir(Dirs::ResultsDir() + argv[2] + "/");
  vector< string > LoResFileNames, HiResFileNames;
  if( argc >= 4)
  {
    LoResFileNames.push_back(Dirs::BlockDir() + argv[3]);
    HiResFileNames.push_back(Dirs::SliceDir() + argv[3]);
  }
  else
  {
    LoResFileNames = getFilePaths(Dirs::BlockDir(), Dirs::SliceFile());
    HiResFileNames = getFilePaths(Dirs::SliceDir(), Dirs::SliceFile());
  }
	
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(LoResFileNames);
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFileNames);
  normalizeImages< StackType >(LoResImages);
  normalizeImages< StackType >(HiResImages);
  boost::shared_ptr< StackType > LoResStack = InitializeLoResStack<StackType>(LoResImages);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages);
  
  // Assert stacks have the same number of slices
  assert(LoResStack->GetSize() == HiResStack->GetSize());
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  StackTransforms::InitializeWithTranslation( *LoResStack, StackTransforms::GetLoResTranslation("whole_heart") );
  StackTransforms::InitializeToCommonCentre( *HiResStack );
  StackTransforms::SetMovingStackCORWithFixedStack( *LoResStack, *HiResStack );

  // Generate fixed images to register against
  LoResStack->updateVolumes();
  if( argc < 4)
  {
    writeImage< StackType::VolumeType >( LoResStack->GetVolume(), outputDir + "LoResStack.mha" );
  }
  
  // initialise registration framework
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder;
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner< StackType > stackAligner(*LoResStack, *HiResStack, registration);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredRigid2DTransform( registration->GetOptimizer() );
  
  // Add time and memory probes
  itkProbesCreate();
  
  // perform centered rigid 2D registration on each pair of slices
  itkProbesStart( "Aligning stacks" );
  stackAligner.Update();
  itkProbesStop( "Aligning stacks" );
  
  // Report the time and memory taken by the registration
  itkProbesReport( std::cout );
  
  // write rigid transforms
  if( argc < 4)
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), outputDir + "HiResRigidStack.mha" );
    // writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), outputDir + "HiResRigidMask.mha" );
  }
  StackTransforms::InitializeFromCurrentTransforms< StackType, itk::CenteredSimilarity2DTransform< double > >(*HiResStack);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredSimilarity2DTransform( registration->GetOptimizer() );
  
  // perform similarity rigid 2D registration
  stackAligner.Update();
  
  // write similarity transforms
  if(argc < 4)
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), outputDir + "HiResSimilarityStack.mha" );
  }
  
  // repeat registration with affine transform
  StackTransforms::InitializeFromCurrentTransforms< StackType, itk::CenteredAffineTransform< double, 2 > >(*HiResStack);
  StackTransforms::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );
  stackAligner.Update();
  
  if(argc < 4)
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), outputDir + "HiResAffineStack.mha" );
    // writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), outputDir + "HiResAffineMask.mha" );
  }
  
  // Update LoRes as the masks might have shrunk
  LoResStack->updateVolumes();
  
  // persist mask numberOfTimesTooBig
  saveNumberOfTimesTooBig(*HiResStack, outputDir + "numberOfTimesTooBig.txt");
  
  // Write final transforms to file
  using namespace boost::filesystem;
  string LoResTransformsDir = outputDir + "LoResTransforms";
  string HiResTransformsDir = outputDir + "HiResTransforms";
  create_directory(LoResTransformsDir);
  create_directory(HiResTransformsDir);
  Save(*LoResStack, LoResFileNames, LoResTransformsDir);
  Save(*HiResStack, HiResFileNames, HiResTransformsDir);
  
  return EXIT_SUCCESS;
}

#include <assert.h>
#include "itkCenteredSimilarity2DTransform.h"
#include "itkLBFGSBOptimizer.h"
// TEMP
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
// TEMP

// my files
#include "Stack.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "Profiling.hpp"


void checkUsage(int argc, char const *argv[]) {
  if( argc != 3 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet outputDir\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  string outputDir(Dirs::ResultsDir() + argv[2] + "/");
	
	// initialise stack objects
  Stack::VolumeType::SpacingType LoResSpacings, HiResSpacings;
	for(unsigned int i=0; i<3; i++) {
    imageDimensions()["LoResSpacings"][i] >> LoResSpacings[i];
    imageDimensions()["HiResSpacings"][i] >> HiResSpacings[i];
  }
  
  Stack::SliceType::SizeType LoResSize;
  itk::Vector< double, 2 > LoResTranslation;
  for(unsigned int i=0; i<2; i++) {
    imageDimensions()["LoResSize"][i] >> LoResSize[i];
    imageDimensions()["LoResTranslation"][i] >> LoResTranslation[i];
  }
  
  Stack LoResStack( getFileNames(Dirs::BlockDir(), Dirs::SliceFile()), LoResSpacings , LoResSize);
  
  Stack::SliceType::SpacingType HiResOriginalSpacings;
  for(unsigned int i=0; i<2; i++) HiResOriginalSpacings[i] = HiResSpacings[i];
  
  Stack HiResStack(getFileNames(Dirs::SliceDir(), Dirs::SliceFile()), HiResOriginalSpacings,
        LoResStack.GetSpacings(), LoResStack.GetResamplerSize());
  
  // Assert stacks have the same number of slices
  assert(LoResStack.GetSize() == HiResStack.GetSize());
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  StackTransforms::InitializeWithTranslation( LoResStack, LoResTranslation );
  StackTransforms::InitializeToCommonCentre( HiResStack );
  StackTransforms::SetMovingStackCORWithFixedStack( LoResStack, HiResStack );

  // Generate fixed images to register against
  LoResStack.updateVolumes();
  writeImage< Stack::VolumeType >( LoResStack.GetVolume(), outputDir + "LoResStack.mha" );
  
  // initialise registration framework
  RegistrationBuilder registrationBuilder;
  RegistrationBuilder::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner stackAligner(LoResStack, HiResStack, registration);
  
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
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResRigidStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResRigidMask.mha" );
  StackTransforms::InitializeFromCurrentTransforms< itk::CenteredSimilarity2DTransform< double > >(HiResStack);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredSimilarity2DTransform( registration->GetOptimizer() );
  
  // perform similarity rigid 2D registration
  stackAligner.Update();
  
  // write similarity transforms
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResSimilarityStack.mha" );
  
  // repeat registration with affine transform
  StackTransforms::InitializeFromCurrentTransforms< itk::CenteredAffineTransform< double, 2 > >(HiResStack);
  StackTransforms::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );
  stackAligner.Update();
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResAffineStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResAffineMask.mha" );
  
  // Update LoRes as the masks might have shrunk
  LoResStack.updateVolumes();
  
  // persist mask numberOfTimesTooBig
  saveNumberOfTimesTooBig(HiResStack, outputDir + "numberOfTimesTooBig.txt");
  
  // Write final transforms to file
  StackTransforms::Save(LoResStack, outputDir + "LoResTransforms.meta");
  StackTransforms::Save(HiResStack, outputDir + "HiResTransforms.meta");
  
  return EXIT_SUCCESS;
}

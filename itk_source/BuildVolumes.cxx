#include "itkCenteredSimilarity2DTransform.h"

// my files
#include "Stack.hpp"
#include "Framework2DRat.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "RegistrationParameters.hpp"

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
    registrationParameters()["LoResSpacings"][i] >> LoResSpacings[i];
    registrationParameters()["HiResSpacings"][i] >> HiResSpacings[i];
  }
  
  Stack::SliceType::SizeType LoResSize;
  itk::Vector< double, 2 > LoResTranslation;
  for(unsigned int i=0; i<2; i++) {
    registrationParameters()["LoResSize"][i] >> LoResSize[i];
    registrationParameters()["LoResTranslation"][i] >> LoResTranslation[i];
  }
  
  Stack LoResStack( getFileNames(Dirs::BlockDir(), Dirs::SliceFile()), LoResSpacings , LoResSize);
  
  Stack::SliceType::SpacingType HiResOriginalSpacings;
  for(unsigned int i=0; i<2; i++) HiResOriginalSpacings[i] = HiResSpacings[i];
  
  Stack HiResStack(getFileNames(Dirs::SliceDir(), Dirs::SliceFile()), HiResOriginalSpacings,
        LoResStack.GetSpacings(), LoResStack.GetResamplerSize());
  
  // Assert stacks have the same number of slices
  if (LoResStack.GetSize() != HiResStack.GetSize())
  {
    cerr << "LoRes and HiRes stacks are different sizes!" << endl;
    std::abort();
  }
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  StackTransforms::InitializeWithTranslation( LoResStack, LoResTranslation );
  StackTransforms::InitializeToCommonCentre( HiResStack );
  StackTransforms::SetMovingStackCORWithFixedStack( LoResStack, HiResStack );

  // Generate fixed images to register against
  LoResStack.updateVolumes();
  // writeImage< Stack::VolumeType >( LoResStack.GetVolume(), outputDir + "LoResStack.mha" );
  
  // initialise registration framework
  Framework2DRat framework2DRat(LoResStack, HiResStack);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredRigid2DTransform( framework2DRat.GetOptimizer() );
  
  // perform centered rigid 2D registration
  framework2DRat.StartRegistration();
  
  // write rigid transforms
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResRigidStack.mha" );
  // writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResRigidMask.mha" );
  
  StackTransforms::InitializeFromCurrentTransforms< itk::CenteredSimilarity2DTransform< double > >(HiResStack);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredSimilarity2DTransform( framework2DRat.GetOptimizer() );
  
  // perform similarity rigid 2D registration
  framework2DRat.StartRegistration();
  
  // write similarity transforms
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResSimilarityStack.mha" );
  
  // repeat registration with affine transform
  StackTransforms::InitializeFromCurrentTransforms< itk::CenteredAffineTransform< double, 2 > >(HiResStack);
  StackTransforms::SetOptimizerScalesForCenteredAffineTransform( framework2DRat.GetOptimizer() );
  framework2DRat.StartRegistration();
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResAffineStack.mha" );
  // writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResSimilarityMask.mha" );
  
  //   StackTransforms::InitializeFromCurrentTransforms< itk::Similarity2DTransform< double > >( HiResStack );
  //   
  //   // Set optimizer scales for Similarity2DTransform
  // Framework2DRat::OptimizerType::ScalesType similarityOptimizerScales( 4 );
  //   similarityOptimizerScales[0] = 1.0;
  //   similarityOptimizerScales[1] = 1.0;
  //   similarityOptimizerScales[2] = translationScale;
  //   similarityOptimizerScales[3] = translationScale;
  //   framework2DRat.GetOptimizer()->SetScales( similarityOptimizerScales );
  //   
  //   framework2DRat.StartRegistration( outputDir + "output2.txt" );
  //   
  //   // Update LoRes as the masks might have shrunk, HiRes as the transforms have changed
  //   LoResStack.updateVolumes();
  //   HiResStack.updateVolumes();
  
  // Write final transform to file
  // writeData< itk::TransformFileWriter, Framework3D::TransformType3D >( framework3D.transform3D, outputDir + "finalParameters3D.transform" );
  
	// perform non-rigid registration
	// check out itkMultiResolutionPDEDeformableRegistration
	// update HiRes slices
  // HiResStack.updateVolumes();
	// write volume and mask to disk
	
  return EXIT_SUCCESS;
}

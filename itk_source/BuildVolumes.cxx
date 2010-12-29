// YAML config reader
#include "yaml.h"

// ITK includes
#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageMaskSpatialObject.h"

// File IO
#include "itkImageFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkSimilarity2DTransform.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "Framework2DRat.hpp"
#include "helper_functions.hpp"
#include "TransformInitializers.hpp"
#include "Dirs.hpp"
#include "RegistrationParameters.hpp"

// TEMP
#include "itkTransformFileWriter.h"
// TEMP

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
  
  Stack::SliceType::SizeType LoResSize, LoResOffset;
  for(unsigned int i=0; i<2; i++) {
    registrationParameters()["LoResSize"][i] >> LoResSize[i];
    registrationParameters()["LoResOffset"][i] >> LoResOffset[i];
  }
  
  // Stack LoResStack( getFileNames(Dirs::BlockDir(), Dirs::SliceFile), LoResSpacings , LoResSize, LoResOffset);
  // Stack HiResStack( getFileNames(Dirs::SliceDir(), Dirs::SliceFile), HiResSpacings );
  // TEMP
  HiResSpacings[0] = 1;
  HiResSpacings[1] = 1;
  HiResSpacings[2] = 1;
  LoResSpacings[0] = 1;
  LoResSpacings[1] = 1;
  LoResSpacings[2] = 1;
  
  vector< string > brain, rotatedBrain;
  brain.push_back(Dirs::BlockDir() + "10000.png");
  rotatedBrain.push_back(Dirs::SliceDir() + "10000.png");
  Stack LoResStack( brain, LoResSpacings );
  Stack HiResStack( rotatedBrain, HiResSpacings );
  
  // TEMP
  
  // Assert stacks have the same number of slices
  if (LoResStack.GetSize() != HiResStack.GetSize())
  {
    cerr << "LoRes and HiRes stacks are different sizes!" << endl;
    std::abort();
  }
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  InitializeStackTransforms::ToCommonCentre( LoResStack );
  InitializeStackTransforms::ToCommonCentre( HiResStack );
  
  LoResStack.updateVolumes();
  HiResStack.updateVolumes();
  
  Framework2DRat framework2DRat(LoResStack, HiResStack);
  
  // Set optimizer scales for CenteredRigid2DTransform
  double translationScale;
  registrationParameters()["optimizer"]["translationScale"] >> translationScale;
	itk::Array< double > rigidOptimizerScales( 5 );
  rigidOptimizerScales[0] = 1.0;
  rigidOptimizerScales[1] = translationScale;
  rigidOptimizerScales[2] = translationScale;
  rigidOptimizerScales[3] = translationScale;
  rigidOptimizerScales[4] = translationScale;
  framework2DRat.GetOptimizer()->SetScales( rigidOptimizerScales );
  
	// perform centered rigid 2D registration
  framework2DRat.StartRegistration();
  
  HiResStack.updateVolumes();
  
  
  // TEMP
  // write transform
  itk::TransformFileWriter::Pointer writer;
  writer = itk::TransformFileWriter::New();
  writer->SetFileName( outputDir + "Transforms.meta" );
  writer->AddTransform( HiResStack.GetTransform(0) );
  
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    return 0;
    }
  
  // TEMP
  
  //   InitializeStackTransforms::FromCurrentTransforms< itk::Similarity2DTransform< double > >( HiResStack );
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
	writeImage< Stack::VolumeType >( LoResStack.GetVolume(), outputDir + "LoResStack.mha" );
  writeImage< Stack::MaskVolumeType >( LoResStack.Get3DMask()->GetImage(), outputDir + "LoResMask.mha" );
	writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResMask.mha" );
	
  return EXIT_SUCCESS;
}

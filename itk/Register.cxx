// YAML config reader
#include "yaml.h"

// ITK includes
#include "itkImage.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageMaskSpatialObject.h"

// File IO
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "Framework3D.hpp"
#include "Framework2DRabbit.hpp"
#include "helper_functions.hpp"

void checkUsage(int argc, char const *argv[]) {
  if( argc < 6 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " histoDir fileNames mriFile registrationFile outputDir ";
    exit(EXIT_FAILURE);
  }
  
}

int main (int argc, char const *argv[]) {
  string outputDir(argv[5]);
  
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// read registration parameters
  YAML::Node registrationParameters;
  readRegistrationParameters(registrationParameters, argv[4]);
	
	// initialise stack and MRI objects
	Stack stack( getFileNames(argv[1], argv[2]), registrationParameters );
		
  double MRIInitialResizeFactor;
  registrationParameters["MRIInitialResizeFactor"] >> MRIInitialResizeFactor;
  MRI mriVolume( argv[3],
	               stack.GetVolume()->GetSpacing(),
	               stack.GetVolume()->GetLargestPossibleRegion().GetSize(),
	               MRIInitialResizeFactor);
	
	// perform 3-D registration
  Framework3D framework3D(&stack, &mriVolume, registrationParameters);
  framework3D.StartRegistration( outputDir + "/output3D.txt" );
	
	// Write final transform to file
  writeData< itk::TransformFileWriter, Framework3D::TransformType3D >( framework3D.transform3D, (outputDir + "/finalParameters3D.transform").c_str() );
	
	writeImage< MRI::VolumeType >( mriVolume.GetResampledVolume(), (outputDir + "/registered_mri.mhd").c_str() );
    
	// perform 2-D registration
  Framework2DRabbit framework2DRabbit(&stack, &mriVolume, registrationParameters);
  framework2DRabbit.StartRegistration( outputDir + "/output2D.txt" );
	
	
	// perform non-rigid registration
	// check out itkMultiResolutionPDEDeformableRegistration
	
	// write volume and mask to disk
  stack.UpdateVolumes();
	writeImage< Stack::VolumeType >( stack.GetVolume(), (outputDir + "/stack.mhd").c_str() );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), (outputDir + "/mask.mhd").c_str() );
		
  return EXIT_SUCCESS;
}
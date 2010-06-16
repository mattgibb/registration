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
#include "Framework2D.hpp"
#include "helper_functions.hpp"

void checkUsage(int argc, char const *argv[]) {
  if( argc < 4 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " LoResDir HiResDir registrationFile outputDir\n\n";
    exit(EXIT_FAILURE);
  }
  
}

int main (int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
  
  string LoResDir(argv[1]), HiResDir(argv[2]), registrationFile(argv[3]), outputDir(argv[4]);
	
	// read registration parameters
  YAML::Node registrationParameters;
  readRegistrationParameters(registrationParameters, registrationFile);
	
	// initialise stack and MRI objects
  cout << "Check file separator before basename: " << outputDir + "/picked_files.txt" << endl;
	Stack LoResStack( getFileNames(LoResDir, outputDir + "/picked_files.txt"), registrationParameters );
		
  // Write final transform to file
    // writeData< itk::TransformFileWriter, Framework3D::TransformType3D >( framework3D.transform3D, (outputDir + "/finalParameters3D.transform").c_str() );
    
	// perform 2-D registration
  // Framework2D framework2D(&stack, &mriVolume, registrationParameters);
  // framework2D.StartRegistration( (outputDir + "/output2D.txt").c_str() );
  //  
	
	// perform non-rigid registration
	// check out itkMultiResolutionPDEDeformableRegistration
	
	// write volume and mask to disk
  LoResStack.UpdateVolumes();
	writeImage< Stack::VolumeType >( LoResStack.GetVolume(), (outputDir + "/LoResStack.mhd").c_str() );
	writeImage< Stack::MaskVolumeType >( LoResStack.GetMaskVolume(), (outputDir + "/LoResMask.mhd").c_str() );
		
  return EXIT_SUCCESS;
}
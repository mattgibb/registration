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


using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 10 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " histoDir regularExpression ";
		cerr << "sortingExpression mriFile transformFile3D ";
    cerr << "outputMRI outputStack outputMask outputInfo3D ";
		cerr << "outputInfo2D\n\n";
		cerr << "histoDir should have no trailing slash\n";
		cerr << "regularExpression should be enclosed in single quotes\n";
		cerr << "sortingExpression should be an integer ";
		cerr << "representing the group in the regexp\n\n";
    exit(EXIT_FAILURE);
  }
  
}

vector< string > getFileNames2(char const *argv[]) {
	string directory = argv[1];
	string regularExpression = argv[2];
	const unsigned int subMatch = atoi( argv[3]);
	
	typedef itk::RegularExpressionSeriesFileNames    NameGeneratorType;
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

  nameGenerator->SetRegularExpression( regularExpression );
  nameGenerator->SetSubMatch( subMatch );
  nameGenerator->SetDirectory( directory );
  
	return nameGenerator->GetFileNames();
}

vector< string > getFileNames(char const *argv[]) {
  string directory = argv[1];
  vector< string > fileNames;
  string fileName;
  fileNames.clear();
  ifstream infile("config/downsample_64x64x16_files.txt", ios_base::in);
  while (getline(infile, fileName)) {
    fileNames.push_back(directory + fileName);
  }
  
  return fileNames;
}

template<typename WriterType, typename DataType>
void writeData(typename DataType::Pointer data, char const *fileName) {
  // typedef itkTransformFileWriter WriterType;
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( data );
  // writer->AddTransform( anotherTransform ); // only applies to writing transforms
  
  writer->SetFileName( fileName );
	
  try {
  	writer->Update();
  }
	catch( itk::ExceptionObject & err ) {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
		exit(EXIT_FAILURE);
	}
}

template<typename ImageType>
void writeImage(typename ImageType::Pointer image, char const *fileName) {
  writeData< itk::ImageFileWriter< ImageType >, ImageType >( image, fileName );
}

void readRegistrationParameters(YAML::Node & parameters) {
  // TODO: extract filename into ARGV.
  ifstream config_filestream("config/registration_parameters_64.yml");
  YAML::Parser parser(config_filestream);
  parser.GetNextDocument(parameters);
}

int main (int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// read registration parameters
  YAML::Node registrationParameters;
  readRegistrationParameters(registrationParameters);
	
	// initialise stack and MRI objects
	Stack stack( getFileNames(argv) );
	
  double MRIInitialResizeFactor;
  registrationParameters["MRIInitialResizeFactor"] >> MRIInitialResizeFactor;
  MRI mriVolume( argv[4],
	               stack.GetVolume()->GetSpacing(),
	               stack.GetVolume()->GetLargestPossibleRegion().GetSize(),
	               MRIInitialResizeFactor);
	
	// perform 3-D registration
	Framework3D framework3D(&stack, &mriVolume, registrationParameters);
  cout << "Starting Registration..." << endl;
	framework3D.StartRegistration( argv[9] );
	
	// Write final transform to file
  writeData< itk::TransformFileWriter, Framework3D::TransformType3D >( framework3D.transform3D, argv[5] );
	
	writeImage< MRI::VolumeType >( mriVolume.GetResampledVolume(), argv[6] );
  
	// perform 2-D registration
  // Framework2D framework2D(&stack, &mriVolume, registrationParameters);
  // framework2D.StartRegistration( argv[10] );
	
	
	// perform non-rigid registration
	// check out itkMultiResolutionPDEDeformableRegistration
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[7] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), argv[8] );
		
  return EXIT_SUCCESS;
}
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

using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 10 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " histoDir regularExpression ";
		cerr << "sortingExpression mriFile transformFile3D ";
		cerr << "outputMRI outputStack outputMask outputParameters\n\n";
		cerr << "histoDir should have no trailing slash\n";
		cerr << "regularExpression should be enclosed in single quotes\n";
		cerr << "sortingExpression should be an integer ";
		cerr << "representing the group in the regexp\n\n";
    exit(EXIT_FAILURE);
  }
  
}

vector< string > getFileNames(char const *argv[]) {
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
  ifstream config_filestream("config/registration_parameters.yml");
  YAML::Parser parser(config_filestream);
  parser.GetNextDocument(parameters);
}

int main (int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
  YAML::Node registrationParameters;
	// read registration parameters
  readRegistrationParameters(registrationParameters);
	
	// initialise stack and MRI objects
	Stack stack( getFileNames(argv) );
  Stack::VolumeType::Pointer stackVolume = stack.GetVolume();
	MRI mriVolume( argv[4], stackVolume->GetSpacing(), stackVolume->GetLargestPossibleRegion().GetSize() );
  	
	// perform 3-D registration
	Framework3D framework3D(&stack, &mriVolume, registrationParameters);
	framework3D.beginRegistration( argv[9] );
	
	// Get final parameters
  Framework3D::OptimizerType3D::ParametersType finalParameters3D = framework3D.registration3D->GetLastTransformParameters();
  
	// Write final transform to file
  writeData< itk::TransformFileWriter, Framework3D::TransformType3D >( framework3D.transform3D, argv[5] );
	
  Framework3D::TransformType3D::MatrixType matrix3D = framework3D.transform3D->GetRotationMatrix();
  Framework3D::TransformType3D::OffsetType offset3D = framework3D.transform3D->GetOffset();
  
  std::cout << "Matrix = " << std::endl << matrix3D << std::endl;
  std::cout << "Offset = " << std::endl << offset3D << std::endl;  
	
  typedef itk::ResampleImageFilter< MRI::VolumeType, MRI::VolumeType > ResampleFilterType;
  Framework3D::TransformType3D::Pointer finalTransform = Framework3D::TransformType3D::New();
  
  // TODO Check to see if using transform3D directly instead of finalTransform makes a difference
  finalTransform->SetCenter( framework3D.transform3D->GetCenter() );
  finalTransform->SetParameters( finalParameters3D );
  finalTransform->SetFixedParameters( framework3D.transform3D->GetFixedParameters() );
  
  ResampleFilterType::Pointer resampler3D = ResampleFilterType::New();
  
  Stack::VolumeType::Pointer fixedImage3D = stack.GetVolume();
  
  resampler3D->SetSize( fixedImage3D->GetLargestPossibleRegion().GetSize() );
  resampler3D->SetOutputOrigin( fixedImage3D->GetOrigin() );
  resampler3D->SetOutputSpacing( fixedImage3D->GetSpacing() );
  resampler3D->SetOutputDirection( fixedImage3D->GetDirection() );
  resampler3D->SetDefaultPixelValue( 100 );
  
  // resampler3D->SetTransform( finalTransform );
  resampler3D->SetTransform( framework3D.transform3D );
	resampler3D->SetInterpolator( framework3D.interpolator3D );
  resampler3D->SetInput( mriVolume.GetVolume() );

	writeImage< MRI::VolumeType >(resampler3D->GetOutput(), argv[6] );
  
  // used for final resampling
	typedef itk::NearestNeighborInterpolateImageFunction< MRI::VolumeType, double > NearestNeighborInterpolatorType3D;
  
	// extract 2-D MRI slices
	// typedef itk::ExtractImageFilter< OutputImageType, OutputSliceType > ExtractFilterType;
  // ExtractFilterType::Pointer extractor = ExtractFilterType::New();
  
  // Framework2D framework2D(&stack, &mriVolume);
  // framework2D->setOptimizerTranslationScale( registrationParameters["optimizer_translation_scale_2D"] );
	
	
	
	// perform 2-D registration
	typedef StdOutIterationUpdate< itk::RegularStepGradientDescentOptimizer > ObserverType2D;
	ObserverType2D::Pointer observer2D = ObserverType2D::New();
	
	// perform non-rigid registration
	// check out itkMultiResolutionPDEDeformableRegistration
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[7] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), argv[8] );
		
  return EXIT_SUCCESS;
}
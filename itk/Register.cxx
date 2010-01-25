#include "itkImage.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageRegistrationMethod.h"
// #include "somesortofImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

// 3-D registration
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

// 2-D registration
#include "itkRegularStepGradientDescentOptimizer.h"

// File IO
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// my files
#include "CommandIterationUpdate.hpp"
#include "Stack.hpp"

using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 5 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " directory regularExpression ";
    cerr << "sortingExpression outputImageFile\n\n";
		cerr << "directory should have no trailing slash\n";
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

template<typename ImageType>
void writeImage(typename ImageType::Pointer image, char const *fileName) {
	typedef itk::ImageFileWriter< ImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( image );
  
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

int main (int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	
	// initialise stack object
	Stack stack( getFileNames(argv) );
	
	
	// perform 3-D registration
	typedef CommandIterationUpdate< itk::RegularStepGradientDescentOptimizer > ObserverType2D;
	ObserverType2D::Pointer test = ObserverType2D::New();
	
	// add observer to optimiser
	
	
	// extract 2-D MRI slices
	
	
	// perform 2-D registration
	
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[4] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), "testdir/testmask.mhd");
		
	
  return EXIT_SUCCESS;
}
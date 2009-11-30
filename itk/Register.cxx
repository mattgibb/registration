#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageRegistrationMethod.h"

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
	
  try{
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
	
  
  // set up input and output types
  typedef unsigned short PixelType;
  typedef itk::Image< PixelType, 2 > SliceType;
  typedef itk::Image< PixelType, 3 > VolumeType;
	
	
  // instantiate stack with list of file names
	Stack stack( getFileNames(argv) );
	
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[4] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), "testdir/testmask.mhd");
	
  // write image to file
  //TEMPORARY
	// typedef itk::ImageFileWriter< InputImageType > WriterType2;
	// WriterType2::Pointer writer2 = WriterType2::New();
	// 
	// for(unsigned int i=0; i<transformedImages.size(); i++)
	// {
	// 	writer2->SetFileName( string("testdir/") + fileNames[i] );
	// 	writer2->SetInput( transformedImages[i] );
	// 	writer2->Update();
	// }
  //TEMPORARY
	
	
  return EXIT_SUCCESS;
}
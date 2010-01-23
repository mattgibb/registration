#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPermuteAxesImageFilter.h"

using namespace std;

int main( int argc, char * argv[] )
{
  if( argc < 6 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << "  inputImageFile   outputImageFile axis1 axis2 axis3" << endl;
    return EXIT_FAILURE;
  }
	
  typedef  unsigned char PixelType;
  typedef itk::Image< PixelType,  3 >   ImageType;
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  typedef itk::ImageFileWriter< ImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( argv[1] );
  writer->SetFileName( argv[2] );

  typedef itk::PermuteAxesImageFilter< ImageType >  FilterType;
  FilterType::Pointer filter = FilterType::New();

  FilterType::PermuteOrderArrayType permuteArray;
	
  permuteArray[0] = atoi( argv[3] );
  permuteArray[1] = atoi( argv[4] );
  permuteArray[2] = atoi( argv[5] );

	filter->SetOrder( permuteArray );

  filter->SetInput( reader->GetOutput() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

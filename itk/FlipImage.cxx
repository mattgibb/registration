#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkFlipImageFilter.h"

using namespace std;

int main( int argc, char * argv[] )
{
  if( argc < 3 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << "  inputImageFile   outputImageFile" << endl;
    return EXIT_FAILURE;
  }

  typedef  itk::RGBPixel< unsigned char > PixelType;
  typedef itk::Image< PixelType,  2 >   ImageType;
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  typedef itk::ImageFileWriter< ImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( argv[1] );
  writer->SetFileName( argv[2] );

  typedef itk::FlipImageFilter< ImageType >  FilterType;
  FilterType::Pointer filter = FilterType::New();

  FilterType::FlipAxesArrayType flipArray;
	
  flipArray[0] = 1;
  flipArray[1] = 0;

  filter->SetFlipAxes( flipArray );

  filter->SetInput( reader->GetOutput() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

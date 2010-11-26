#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkFlipImageFilter.h"
#include "itkPermuteAxesImageFilter.h"

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

  // Permute the x and y axes, i.e. flip the images through x = y
  typedef itk::PermuteAxesImageFilter< ImageType >  PermuteAxesType;
  PermuteAxesType::Pointer permuter = PermuteAxesType::New();

  PermuteAxesType::PermuteOrderArrayType permuteArray;
	
  permuteArray[0] = 1;
  permuteArray[1] = 0;

  permuter->SetOrder( permuteArray );

  permuter->SetInput( reader->GetOutput() );
  
  // Flip the images through vertical axis
  typedef itk::FlipImageFilter< ImageType >  FlipImageType;
  FlipImageType::Pointer flipper = FlipImageType::New();

  FlipImageType::FlipAxesArrayType flipArray;
	
  flipArray[0] = 1;
  flipArray[1] = 0;

  flipper->SetFlipAxes( flipArray );

  flipper->SetInput( permuter->GetOutput() );

  writer->SetInput( flipper->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

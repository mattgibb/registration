// Rotate RGB image 90 degrees clockwise

#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkFlipImageFilter.h"
#include "itkPermuteAxesImageFilter.h"

#include "IOHelpers.hpp"

using namespace std;

int main( int argc, char * argv[] )
{
  if( argc < 3 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << "  inputImageFile outputImageFile" << endl;
    return EXIT_FAILURE;
  }
	
  typedef  itk::RGBPixel< unsigned char > PixelType;
  typedef itk::Image< PixelType,  2 >   ImageType;
  
  ImageType::Pointer input = readImage< ImageType >( argv[1] );

  // Permute the x and y axes, i.e. flip the images through x = y
  typedef itk::PermuteAxesImageFilter< ImageType >  PermuteAxesType;
  PermuteAxesType::Pointer permuter = PermuteAxesType::New();

  PermuteAxesType::PermuteOrderArrayType permuteArray;
	
  permuteArray[0] = 1;
  permuteArray[1] = 0;
  
  permuter->SetOrder( permuteArray );

  permuter->SetInput( input );
  
  // Flip the images through vertical axis
  typedef itk::FlipImageFilter< ImageType >  FlipImageType;
  FlipImageType::Pointer flipper = FlipImageType::New();

  FlipImageType::FlipAxesArrayType flipArray;
	
  flipArray[0] = 1;
  flipArray[1] = 0;

  flipper->SetFlipAxes( flipArray );

  flipper->SetInput( permuter->GetOutput() );
  
  ImageType::Pointer output = flipper->GetOutput();
  writeImage< ImageType >( output, argv[2] );

  return EXIT_SUCCESS;
}

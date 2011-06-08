// Deals with offset bug in ITK RGBA reader by permuting RGB channels

#include "itkRGBPixel.h"
#include "itkRGBAPixel.h"
#include "IOHelpers.hpp"


using namespace std;

int main( int argc, char * argv[] )
{
  if( argc != 3 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << "  inputImageFile outputImageFile" << endl;
    exit(EXIT_FAILURE);
  }
  
  string inputImageFile(argv[1]), outputImageFile(argv[2]);
  
  const     unsigned int   Dimension = 2;
  
  typedef itk::RGBAPixel< unsigned char > InputPixelType;
  typedef itk::RGBPixel< unsigned char >  OutputPixelType;
  
  typedef itk::Image< InputPixelType,  Dimension > InputImageType;
  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
  
  // Read RGBA image from file
  InputImageType::Pointer inputImage = readImage< InputImageType >( inputImageFile );
  
  // Build RGB image from RGBA image
  // Initialise output image
  OutputImageType::RegionType region;
  region.SetSize( inputImage->GetLargestPossibleRegion().GetSize() );
  
  OutputImageType::Pointer outputImage = OutputImageType::New();
  outputImage->SetRegions( region );
  outputImage->CopyInformation( inputImage );
  outputImage->Allocate();
  
  // copy values from input image
  itk::ImageRegionConstIterator< InputImageType > cit(inputImage, region);
  itk::ImageRegionIterator< OutputImageType >     it(outputImage, region);
  for (cit.GoToBegin(), it.GoToBegin(); !it.IsAtEnd(); ++cit, ++it ) {
    it.Value().SetRed( cit.Value().GetGreen() );
    it.Value().SetGreen( cit.Value().GetBlue() );
    it.Value().SetBlue( cit.Value().GetAlpha() );
    
    // check that actual Alpha channel (even though it's interpreted as Red)
    // contains only zeros
    if(cit.Value().GetRed())
    {
      cerr << "Non-zero alpha in input image!" << endl;
      abort();
    }
  }
  
  // write RGB image to file
  writeImage< OutputImageType >( outputImage, outputImageFile );
  
  return EXIT_SUCCESS;
}

#include "itkRGBPixel.h"
#include "itkRGBAPixel.h"
#include "IOHelpers.hpp"


using namespace std;

int main( int argc, char * argv[] )
{
  if( argc != 3 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << "  inputImageFile outputDir" << endl;
    exit(EXIT_FAILURE);
  }
  
  string inputImageFile(argv[1]), outputDir(argv[2]);
  
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
    it.Value().SetRed( cit.Value().GetRed() );
    it.Value().SetBlue( cit.Value().GetBlue() );
    it.Value().SetGreen( cit.Value().GetGreen() );
    
    // cout << cit.Value().GetAlpha() << endl;
    // if(cit.Value().GetAlpha())
    // {
    //   cerr << "Non-zero alpha in input image!" << endl;
    //   abort();
    // }
  }
  
  // write RGB image to file
  writeImage< OutputImageType >( outputImage, outputDir + "RGB.bmp" );
  
  // Write individual channels by name to file.
  #define writeChannelByName(Colour) \
  { \
   /* create image from named channel of input image */ \
    typedef itk::Image< unsigned char > ChannelType; \
    ChannelType::Pointer channel = ChannelType::New(); \
    channel->SetRegions( region ); \
    channel->CopyInformation( inputImage ); \
    channel->Allocate(); \
    \
    itk::ImageRegionConstIterator< InputImageType > cit(inputImage, region); \
    itk::ImageRegionIterator< ChannelType > it(channel, region); \
    \
    for (cit.GoToBegin(), it.GoToBegin(); !it.IsAtEnd(); ++cit, ++it ) { \
      it.Set( cit.Value().Get ## Colour() ); \
    } \
    \
   /* write channel */ \
    writeImage< ChannelType >(channel, outputDir + #Colour + ".bmp" ); \
  }
  
  writeChannelByName(Red);
  writeChannelByName(Green);
  writeChannelByName(Blue);
  writeChannelByName(Alpha);
  
  return EXIT_SUCCESS;
}

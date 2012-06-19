// Load transforms and build RGB volumes from images

//itk
#include "itkRGBPixel.h"

// my files
#include "IOHelpers.hpp"

int main(int argc, char *argv[]) {
  if( argc < 3 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << " inputImageFile outputImageFile" << endl;
    exit(EXIT_FAILURE);
  }
  
  typedef itk::RGBPixel< unsigned char > PixelType;
	typedef itk::Image< PixelType, 3 > VolumeType;
  
  cout << "Reading image..." << flush;
  VolumeType::Pointer volume = readImage< VolumeType >(argv[1]);
  cout << "done." << endl;
  
  cout << "Writing image..." << flush;
  writeImage< VolumeType >(volume, argv[2]);
  cout << "done." << endl;
  
  return EXIT_SUCCESS;
}

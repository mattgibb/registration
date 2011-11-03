#include "itkImageFileReader.h"

// my files
#include "ImageStats.hpp"

using namespace std;

int main( int argc, char ** argv )
{
  if( argc < 2 )
  {
    std::cerr << "Usage:" << std::endl;
    std::cerr << argv[0] << " inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }

  typedef unsigned char      PixelType;
  const   unsigned int        Dimension = 2;
  typedef itk::Image< PixelType, Dimension > ImageType;
  typedef itk::ImageFileReader< ImageType > ReaderType;

  ReaderType::Pointer reader = ReaderType::New();

  const char * inputFilename  = argv[1];

  reader->SetFileName( inputFilename );
  
  printImageStats< ImageType >( reader->GetOutput() );
  
  return EXIT_SUCCESS;
}

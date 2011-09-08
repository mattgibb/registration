#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkImage.h"


int main( int argc, char ** argv )
{
  if( argc < 3 )
    {
    std::cerr << "Usage:" << std::endl;
    std::cerr << argv[0] << " inputImageFile outputImageFile" << std::endl;
    return EXIT_FAILURE;
    }

  typedef float      PixelType;
  const   unsigned int        Dimension = 3;
  typedef itk::Image< PixelType, Dimension >    ImageType;
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  typedef itk::ImageFileWriter< ImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  const char * inputFilename  = argv[1];
  const char * outputFilename = argv[2];

  reader->SetFileName( inputFilename  );
  writer->SetFileName( outputFilename );

  writer->SetInput( reader->GetOutput() );

  try 
    { 
    writer->Update(); 
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return EXIT_FAILURE;
    } 

  return EXIT_SUCCESS;
}

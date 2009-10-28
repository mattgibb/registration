#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkRegularExpressionSeriesFileNames.h"
// #include "itkPNGImageIO.h"

int main (int argc, char const *argv[])
{
  // Verify the number of parameters in the command line
  if( argc < 5 )
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "directory regularExpression ";
    std::cerr << "sortingExpression outputImageFile " << std::endl;
    return EXIT_FAILURE;
  }


  typedef unsigned char                       PixelType;
  const unsigned int Dimension = 3;

  typedef itk::Image< PixelType, Dimension >  ImageType;

  typedef itk::ImageSeriesReader< ImageType >  ReaderType;
  typedef itk::ImageFileWriter<   ImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  std::string directory = argv[1];
  std::string regularExpression = argv[2];
  const unsigned int subMatch = atoi( argv[3] );
  std::string outputFilename = argv[4];

  typedef itk::RegularExpressionSeriesFileNames    NameGeneratorType;
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

  nameGenerator->SetRegularExpression( regularExpression );
  nameGenerator->SetSubMatch( subMatch );
  nameGenerator->SetDirectory( directory );
  
  //  The ImageIO object that actually performs the read process is now connected
  //  to the ImageSeriesReader. This is the safest way of making sure that we use
  //  an ImageIO object that is appropriate for the type of files that we want to
  //  read.
  // reader->SetImageIO( itk::PNGImageIO::New() );

  // GetFileNames() returns a vector of strings
  reader->SetFileNames( nameGenerator->GetFileNames()  );

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
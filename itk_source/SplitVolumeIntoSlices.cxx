#include "boost/filesystem.hpp"
#include <assert.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesWriter.h"
#include "itkNumericSeriesFileNames.h"
#include "itkRescaleIntensityImageFilter.h"

// my files
#include "Dirs.hpp"
#include "IOHelpers.hpp"


void checkUsage(int argc, char *argv[]) {
  if( argc < 4 )
  {
    cerr << "\nUsage: " << endl;
    std::cerr << argv[0] << " dataSet inputFile outputDir" << std::endl;
    exit(EXIT_FAILURE);
  }
}


int main( int argc, char *argv[] )
{
  using namespace boost::filesystem;

  // Verify the number of parameters in the command line
  checkUsage(argc, argv);
  
  // Process command line arguments
  Dirs::SetDataSet(argv[1]);
  const string inputFile(argv[2]), outputDir(argv[3]);
  create_directory(outputDir);
  
  // typedefs
  typedef itk::Image< unsigned char, 3 >      Mask3DType;
  typedef itk::Image< unsigned char, 2 >     Mask2DType;
  typedef itk::ImageFileReader< Mask3DType >   MaskReaderType;
  typedef itk::RescaleIntensityImageFilter< Mask3DType, Mask3DType > RescalerType;
  
  // reader
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName( inputFile );
  
  // rescaler
  RescalerType::Pointer rescaler = RescalerType::New();
  rescaler->SetOutputMinimum(  0  );
  rescaler->SetOutputMaximum( 255 );
  rescaler->SetInput( maskReader->GetOutput() );
  
  // writer
  typedef itk::ImageSeriesWriter< Mask3DType, Mask2DType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( rescaler->GetOutput() );
  
  // retrieve file names
  vector< string > outputFilePaths = constructPaths(outputDir, Dirs::ImageList(), ".bmp");
  
  // check image z-size is the same as number of file names
  try { maskReader->Update(); }
  catch( itk::ExceptionObject & e )
  {
    std::cerr << "Exception thrown while reading the image" << std::endl;
    std::cerr << e << std::endl;
  }
  
  const unsigned int zSize = maskReader->GetOutput()->GetLargestPossibleRegion().GetSize()[2];
  assert(outputFilePaths.size() == zSize);
  
  // write series
  writer->SetFileNames( outputFilePaths );

  try { writer->Update(); }
  catch( itk::ExceptionObject & excp )
  {
    std::cerr << "Exception thrown while reading the image" << std::endl;
    std::cerr << excp << std::endl;
  }
  
  return EXIT_SUCCESS;
}

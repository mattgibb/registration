#include "itkImageFileReader.h"
#include "itkStatisticsImageFilter.h"

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
  typedef itk::Image< PixelType, Dimension >    ImageType;
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  typedef itk::StatisticsImageFilter< ImageType > StatsType;

  ReaderType::Pointer reader = ReaderType::New();
  StatsType::Pointer  stats  = StatsType::New();

  const char * inputFilename  = argv[1];

  reader->SetFileName( inputFilename  );

  stats->SetInput( reader->GetOutput() );
  
  try 
    { 
    stats->Update(); 
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return EXIT_FAILURE;
    } 

  // Show statistics
  cerr << "Minimum:\n" << stats->GetMinimum() + 0 << endl << endl;
  cerr << "Maximum:\n" << stats->GetMaximum() + 0 << endl << endl;
  cerr << "Mean:\n" << stats->GetMean() << endl << endl;
  cerr << "Sigma:\n" << stats->GetSigma() << endl << endl;
  cerr << "Variance:\n" << stats->GetVariance() << endl << endl;
  cerr << "Sum:\n" << stats->GetSum() << endl << endl;
  cerr << "Image:\n" << stats->GetOutput() << endl << endl;

  return EXIT_SUCCESS;
}

#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkTileImageFilter.h"

using namespace std;

void checkUsage(int argc, char const *argv[])
{
  if( argc < 5 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " directory regularExpression ";
    cerr << "sortingExpression outputImageFile\n\n";
		cerr << "directory should have no trailing slash\n";
		cerr << "regularExpression should be enclosed in single quotes\n";
		cerr << "sortingExpression should be an integer ";
		cerr << "representing the group in the regexp\n\n";
    exit(EXIT_FAILURE);
  }
  
}

vector< string > getFileNames(char const *argv[])
{
	string directory = argv[1];
	string regularExpression = argv[2];
	const unsigned int subMatch = atoi( argv[3]);
	
	typedef itk::RegularExpressionSeriesFileNames    NameGeneratorType;
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

  nameGenerator->SetRegularExpression( regularExpression );
  nameGenerator->SetSubMatch( subMatch );
  nameGenerator->SetDirectory( directory );
  
	return nameGenerator->GetFileNames();
}

int main (int argc, char const *argv[])
{
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
  
  
  // get file names and build iterator
	vector< string > fileNames = getFileNames(argv);
	vector< string >::iterator fileIter;


  // set up input and output types
  typedef unsigned char PixelType;
  const unsigned int InputDimension  = 2;
  const unsigned int OutputDimension = 3;

  typedef itk::Image< PixelType, InputDimension  > InputImageType;
  typedef itk::Image< PixelType, OutputDimension > OutputImageType;


  // set up vector of input images
  typedef itk::ImageFileReader< InputImageType > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();

	typedef vector< InputImageType::Pointer > InputVectorType;
  InputVectorType slices;
  InputVectorType::iterator inputIter;
  
	for(fileIter=fileNames.begin(); fileIter!=fileNames.end(); fileIter++)
	{
		reader->SetFileName( *fileIter );
		reader->Update();
		cout << *fileIter << endl;
		slices.push_back( reader->GetOutput() );
		// slices.back()->DisconnectPipeline();
	}


  // set up tile image filter
	typedef itk::TileImageFilter< InputImageType, OutputImageType > TileFilterType;
	TileFilterType::Pointer tileFilter = TileFilterType::New();
	
	TileFilterType::LayoutArrayType layout;
	layout[0] = 1;
	layout[1] = 1;
	layout[2] = 0;
	tileFilter->SetLayout( layout );
	
	
  // write results to file
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();

	writer->SetInput( tileFilter->GetOutput() );
  
  std::string outputFilename = argv[4];
  writer->SetFileName( outputFilename );

  //TEMPORARY
	typedef itk::ImageFileWriter< InputImageType > WriterType2;
	WriterType2::Pointer writer2 = WriterType2::New();
	
  for(unsigned int i=0; i<slices.size(); i++)
	{
		writer2->SetFileName( string("testdir/") + fileNames[i] );
		writer2->SetInput( slices[i] );
		writer2->Update();
	}
	
  //TEMPORARY


  try 
    { 
    // writer->Update(); 
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return EXIT_FAILURE;
    } 
  return EXIT_SUCCESS;
}
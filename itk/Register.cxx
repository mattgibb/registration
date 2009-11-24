#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkChangeInformationImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkTileImageFilter.h"
#include "itkTranslationTransform.h"
#include "itkCenteredRigid2DTransform.h"
// #include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageRegistrationMethod.h"
#include "itkCommand.h"


using namespace std;

class CommandIterationUpdate : public itk::Command 
{
public:
  typedef CommandIterationUpdate   Self;
  typedef itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;

  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

public:
  typedef itk::RegularStepGradientDescentOptimizer     OptimizerType;
  typedef const OptimizerType *                        OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
	  // in this case, just calls the const version of Execute
    Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( object );

    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }

    cout << optimizer->GetCurrentIteration() << " = ";
    cout << optimizer->GetValue() << " : ";
    cout << optimizer->GetCurrentPosition() << endl;
    }   
};


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
  
  
  // get file names
	vector< string > fileNames = getFileNames(argv);


  // set up input and output types
  typedef unsigned short PixelType;
  const unsigned int InputDimension  = 2;
  const unsigned int OutputDimension = 3;

  typedef itk::Image< PixelType, InputDimension  > InputImageType;
  typedef itk::Image< PixelType, OutputDimension > OutputImageType;
		
	
  // set up vector of input images and find max width and height
  typedef itk::ImageFileReader< InputImageType > ReaderType;	

	typedef vector< InputImageType::Pointer > InputVectorType;
  InputVectorType originalImages;
	unsigned int i;
	
	ReaderType::Pointer reader;
	InputImageType::SizeType size, maxSize;
	maxSize[0] = 0;
	maxSize[1] = 0;
  
	for(i=0; i<fileNames.size(); i++)
	{
		reader = ReaderType::New();
		reader->SetFileName( fileNames[i] );
		reader->Update();
		originalImages.push_back( reader->GetOutput() );
		size = originalImages.back()->GetLargestPossibleRegion().GetSize();
		
		for(unsigned int j=0; j<InputDimension; j++)
		{
			if(maxSize[j] < size[j]) { maxSize[j] = size[j]; }
		}
		originalImages.back()->DisconnectPipeline();
	}
	
	// get spacing of input images
  InputImageType::SpacingType spacing = originalImages[0]->GetSpacing();
	

  // perform initial transform of images to a common centre
  typedef itk::TranslationTransform< double, InputDimension > Translation2DType;
	Translation2DType::Pointer translation2D;
	
	typedef itk::LinearInterpolateImageFunction< InputImageType, double > InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	
	typedef itk::ResampleImageFilter<InputImageType,InputImageType> ResamplerType;
	ResamplerType::Pointer resampler;
	
	InputVectorType transformedImages;
	
  InputImageType::SizeType originalSize;
  Translation2DType::ParametersType translationParams( InputDimension );

	for(i=0; i<originalImages.size(); i++)
	{
		translation2D = Translation2DType::New();
		resampler = ResamplerType::New();
		resampler->SetInterpolator( interpolator );
		resampler->SetSize( maxSize );
		resampler->SetOutputSpacing( spacing );
		resampler->SetInput( originalImages[i] );
		resampler->SetTransform( translation2D );
		
		// set translation parameters
		originalSize = originalImages[i]->GetLargestPossibleRegion().GetSize();	  
		translationParams[0] = - spacing[0] * ( maxSize[0] - originalSize[0] ) / 2.0;
		translationParams[1] = - spacing[1] * ( maxSize[1] - originalSize[1] ) / 2.0;
		translation2D->SetParameters( translationParams );
		
		resampler->Update();
		transformedImages.push_back( resampler->GetOutput() );
		transformedImages.back()->DisconnectPipeline();    
		
	}


  // 
  typedef itk::CenteredRigid2DTransform< double >  Rigid2DTransformType;
	Rigid2DTransformType::Pointer transform2D = Rigid2DTransformType::New();

	
  // set up tile image filter
  typedef itk::TileImageFilter< InputImageType, OutputImageType > TileFilterType;
  TileFilterType::Pointer tileFilter = TileFilterType::New();
  
  TileFilterType::LayoutArrayType layout;
  layout[0] = 1;
  layout[1] = 1;
  layout[2] = 0;
  tileFilter->SetLayout( layout );
	
	for(i=0; i<transformedImages.size(); i++)
	{
		tileFilter->PushBackInput( transformedImages[i] );
	}
	
	
  // Scale results in the z-direction
  typedef itk::ChangeInformationImageFilter<OutputImageType> ZScaleType;
	ZScaleType::Pointer zScaler = ZScaleType::New();
  
	zScaler->SetInput( tileFilter->GetOutput() );
	
	zScaler->ChangeSpacingOn();
	ZScaleType::SpacingType spacings;
	spacings[0] = 128;
	spacings[1] = 128;
	spacings[2] = 128;
	zScaler->SetOutputSpacing( spacings );

  
  // write image to file
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();

	writer->SetInput( zScaler->GetOutput() );
  
  std::string outputFilename = argv[4];
  writer->SetFileName( outputFilename );

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


  //TEMPORARY
	// typedef itk::ImageFileWriter< InputImageType > WriterType2;
	// WriterType2::Pointer writer2 = WriterType2::New();
	// 
	// for(unsigned int i=0; i<transformedImages.size(); i++)
	// {
	// 	writer2->SetFileName( string("testdir/") + fileNames[i] );
	// 	writer2->SetInput( transformedImages[i] );
	// 	writer2->Update();
	// }
  //TEMPORARY


  return EXIT_SUCCESS;
}
#include "itkImageFileReader.h"

#include "itkImage.h"
#include "itkFlipImageFilter.h"
#include "itkPermuteAxesImageFilter.h"
#include "itkImageMomentsCalculator.h"


using namespace std;

typedef float      PixelType;
const   unsigned int        Dimension = 2;
typedef itk::Image< PixelType, Dimension >    ImageType;

template<typename Pointer>
void Update(Pointer updatable)
{
  try 
    {
      updatable->Update();
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    exit(EXIT_FAILURE);
    }
}

ImageType::Pointer rotate(ImageType::Pointer image)
{
  
  // Permute the x and y axes, i.e. flip the images through x = y
  typedef itk::PermuteAxesImageFilter< ImageType >  PermuteAxesType;
  PermuteAxesType::Pointer permuter = PermuteAxesType::New();

  PermuteAxesType::PermuteOrderArrayType permuteArray;
	
  permuteArray[0] = 1;
  permuteArray[1] = 0;

  permuter->SetOrder( permuteArray );
  permuter->SetInput( image );
  
  // Flip the images through vertical axis
  typedef itk::FlipImageFilter< ImageType >  FlipImageType;
  FlipImageType::Pointer flipper = FlipImageType::New();

  FlipImageType::FlipAxesArrayType flipArray;
	
  flipArray[0] = 1;
  flipArray[1] = 0;
  
  flipper->SetFlipAxes( flipArray );
  flipper->SetInput( permuter->GetOutput() );
  Update(flipper);
  
  ImageType::Pointer rotatedImage = flipper->GetOutput();
  
  rotatedImage->DisconnectPipeline();
  return rotatedImage;
}

int main( int argc, char ** argv )
{
  if( argc < 2 )
  {
    std::cerr << "Usage:" << std::endl;
    std::cerr << argv[0] << " inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }
  
  // reader
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );
  Update(reader);
  
  typedef vector< ImageType::Pointer > VectorType;
  VectorType images;
  images.push_back( reader->GetOutput()   );
  images.push_back( rotate(images.back()) );
  images.push_back( rotate(images.back()) );
  images.push_back( rotate(images.back()) );
  
  typedef itk::ImageMomentsCalculator< ImageType > CalculatorType;
  CalculatorType::Pointer calculator  = CalculatorType::New();
  for(unsigned int i = 0; i < images.size(); ++i)
  {
    cerr << "image: " << i << endl;
    calculator->SetImage(images[i]);
    calculator->Compute();
    
    cerr << "calculator->GetTotalMass():\n" << calculator->GetTotalMass() << endl;
    cerr << "calculator->GetFirstMoments():\n" << calculator->GetFirstMoments() << endl;
    cerr << "calculator->GetSecondMoments():\n" << calculator->GetSecondMoments() << endl;
    cerr << "calculator->GetCenterOfGravity():\n" << calculator->GetCenterOfGravity() << endl;
    cerr << "calculator->GetCentralMoments():\n" << calculator->GetCentralMoments() << endl;
    cerr << "calculator->GetPrincipalMoments():\n" << calculator->GetPrincipalMoments() << endl;
    cerr << "calculator->GetPrincipalAxes():\n" << calculator->GetPrincipalAxes() << endl;
    cerr << endl << endl;
  }
  
  return EXIT_SUCCESS;
}

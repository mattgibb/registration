#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"


int main( int argc, char * argv[] )
{
  if( argc < 3 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputImageFile  outputImageFile" << std::endl; 
    return EXIT_FAILURE;
    }
  
  const     unsigned int   Dimension = 2;
  
  typedef   unsigned char  InputPixelType;
  typedef   float          InternalPixelType;
  typedef   unsigned short OutputPixelType;
  
  typedef itk::Image< InputPixelType,    Dimension > InputImageType;
  typedef itk::Image< InternalPixelType, Dimension > InternalImageType;
  typedef itk::Image< OutputPixelType,   Dimension > OutputImageType;
  
  typedef itk::ImageFileReader< InputImageType  > ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();
  
  reader->SetFileName( argv[1] );
  writer->SetFileName( argv[2] );
  
  const double factor = 128.0;
  
  try 
    {
    reader->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << "Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    }
  
  InputImageType::ConstPointer inputImage = reader->GetOutput();
  
  typedef itk::CastImageFilter< InputImageType, InternalImageType > CastFilterType;

  CastFilterType::Pointer caster = CastFilterType::New();

  caster->SetInput( inputImage );

  typedef itk::RecursiveGaussianImageFilter< InternalImageType,
                                             InternalImageType > GaussianFilterType;

  GaussianFilterType::Pointer smootherX = GaussianFilterType::New();
  GaussianFilterType::Pointer smootherY = GaussianFilterType::New();

  smootherX->SetInput( caster->GetOutput() );
  smootherY->SetInput( smootherX->GetOutput() );

  // The Sigma values to use in the smoothing filters is computed based on the
  // pixel spacings of the input image and the factors provided as arguments. 
  
  const InputImageType::SpacingType& inputSpacing = inputImage->GetSpacing();

  const double sigmaX = inputSpacing[0] * factor;
  const double sigmaY = inputSpacing[1] * factor;

  smootherX->SetSigma( sigmaX );
  smootherY->SetSigma( sigmaY );

  smootherX->SetDirection( 0 );
  smootherY->SetDirection( 1 );
  
  smootherX->SetNormalizeAcrossScale( false );
  smootherY->SetNormalizeAcrossScale( false );
  
  typedef itk::ResampleImageFilter< InternalImageType, OutputImageType >  ResampleFilterType;

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();

  typedef itk::IdentityTransform< double, Dimension >  TransformType;

  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  resampler->SetTransform( transform );

  typedef itk::LinearInterpolateImageFunction< InternalImageType, double >  InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  resampler->SetInterpolator( interpolator );
  
  resampler->SetDefaultPixelValue( 0 ); 
  
  OutputImageType::SpacingType spacing;

  spacing[0] = inputSpacing[0] * factor;
  spacing[1] = inputSpacing[1] * factor;

  resampler->SetOutputSpacing( spacing );
  resampler->SetOutputOrigin( inputImage->GetOrigin() );
  resampler->SetOutputDirection( inputImage->GetDirection() );

  InputImageType::SizeType   inputSize = inputImage->GetLargestPossibleRegion().GetSize();

  typedef InputImageType::SizeType::SizeValueType SizeValueType;

  InputImageType::SizeType size;

  size[0] = static_cast< SizeValueType >( inputSize[0] / factor );
  size[1] = static_cast< SizeValueType >( inputSize[1] / factor );

  resampler->SetSize( size );

  resampler->SetInput( smootherY->GetOutput() );

  writer->SetInput( resampler->GetOutput() );


  try 
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << "Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    }


  return EXIT_SUCCESS;
}


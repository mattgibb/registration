// Smoothes RGB image
#include "boost/program_options.hpp"

#include "itkRGBPixel.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"

#include "IOHelpers.hpp"
#include "ImageStats.hpp"

using namespace std;

namespace po = boost::program_options;
po::variables_map parse_arguments(int argc, char *argv[]);

int main( int argc, char * argv[] )
{
  if( argc < 4 ) {
    cerr << "Usage: " << endl;
    cerr << argv[0] << "  inputImageFile  outputImageFile downsampleRatio" << endl;
    exit(EXIT_FAILURE);
  }
  
  const     unsigned int   Dimension = 2;
  
  // typedef   unsigned char  InputPixelType;
  // typedef   float          InternalPixelType;
  // typedef   unsigned short OutputPixelType;
	typedef itk::RGBPixel< unsigned char >	PixelType;
  typedef PixelType InputPixelType;
  typedef PixelType InternalPixelType;
  typedef PixelType OutputPixelType;
  
  typedef itk::Image< InputPixelType,    Dimension > InputImageType;
  typedef itk::Image< InternalPixelType, Dimension > InternalImageType;
  typedef itk::Image< OutputPixelType,   Dimension > OutputImageType;
  
  typedef itk::ImageFileReader< InputImageType  > ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();
  
  reader->SetFileName( argv[1] );
  writer->SetFileName( argv[2] );
  
  const double factor = atoi(argv[3]);
  
  try {
    reader->Update();
  }
  catch( itk::ExceptionObject & excep ) {
    cerr << "Exception caught!" << endl;
    cerr << excep << endl;
    exit(EXIT_FAILURE);
  }
  
  InputImageType::ConstPointer inputImage = reader->GetOutput();
  
  typedef itk::CastImageFilter< InputImageType, InternalImageType > CastFilterType;
  CastFilterType::Pointer caster = CastFilterType::New();
  caster->SetInput( inputImage );
	
  typedef itk::RecursiveGaussianImageFilter< InternalImageType, InternalImageType > GaussianFilterType;
	
  GaussianFilterType::Pointer smootherX = GaussianFilterType::New();
  GaussianFilterType::Pointer smootherY = GaussianFilterType::New();
	
  // smootherX->SetInput( caster->GetOutput() );
  smootherX->SetInput( reader->GetOutput() ); // skip caster
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
  
  typedef itk::VectorResampleImageFilter< InternalImageType, OutputImageType > ResampleFilterType;
  ResampleFilterType::Pointer resampler = ResampleFilterType::New();

  typedef itk::IdentityTransform< double, Dimension > TransformType;

  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  resampler->SetTransform( transform );
  
  typedef itk::VectorLinearInterpolateImageFunction< InternalImageType, double >  InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  resampler->SetInterpolator( interpolator );
  
  // resampler->SetDefaultPixelValue( 0 );
  
  OutputImageType::SpacingType spacing;

  spacing[0] = inputSpacing[0] * factor;
  spacing[1] = inputSpacing[1] * factor;

  resampler->SetOutputSpacing( spacing );
  resampler->SetOutputOrigin( inputImage->GetOrigin() );
  resampler->SetOutputDirection( inputImage->GetDirection() );

  InputImageType::SizeType inputSize = inputImage->GetLargestPossibleRegion().GetSize();

  typedef InputImageType::SizeType::SizeValueType SizeValueType;
  InputImageType::SizeType size;

  size[0] = static_cast< SizeValueType >( inputSize[0] / factor );
  size[1] = static_cast< SizeValueType >( inputSize[1] / factor );

  resampler->SetSize( size );

  resampler->SetInput( smootherY->GetOutput() );

  writer->SetInput( resampler->GetOutput() );
  
  try {
    writer->Update();
  }
  catch( itk::ExceptionObject & excep ) {
    cerr << "Exception caught!" << endl;
    cerr << excep << endl;
    exit(EXIT_FAILURE);
  }
	
  return EXIT_SUCCESS;
}




int main( int argc, char * argv[] )
{
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
  typedef unsigned char PixelType;
  typedef itk::Image< PixelType, 2 > ImageType;
  
  ImageType::Pointer input = readImage< ImageType >( vm["inputFile"].as<string>() );
  
  // invert intensity
  if(!vm["no-invert"].as<bool>())
  {
    typedef itk::InvertIntensityImageFilter <ImageType> InverterType;
    
    InverterType::Pointer inverter = InverterType::New();
    // inverter->SetMaximum(50); // default 255
    inverter->SetInput(input);
    inverter->Update();
    input = inverter->GetOutput();
  }
  
  // rescale intensity
  typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > IntensifierType;
  IntensifierType::Pointer intensifier = IntensifierType::New();
  intensifier->SetInput( input );
  intensifier->SetOutputMinimum(vm["min"].as<unsigned int>());
  intensifier->SetOutputMaximum(vm["max"].as<unsigned int>());
  
  ImageType::Pointer output = intensifier->GetOutput();
  writeImage< ImageType >( output, vm["outputFile"].as<string>() );
  
  printImageStats<ImageType>(output);
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputFile", po::value<string>(), "input image")
      ("outputFile", po::value<string>(), "output image")
      ("no-invert", po::bool_switch(), "do not invert intensities")
      ("min", po::value<unsigned int>()->default_value(0), "minimum output intensity")
      ("max", po::value<unsigned int>()->default_value(255), "maximum output intensity")
  ;
  
  po::positional_options_description p;
  p.add("inputFile", 1)
   .add("outputFile", 1);
  
  // parse command line
  po::variables_map vm;
	try
	{
  po::store(po::command_line_parser(argc, argv)
            .options(opts)
            .positional(p)
            .run(),
            vm);
	}
	catch (std::exception& e)
	{
	  cerr << "caught command-line parsing error" << endl;
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  po::notify(vm);
  
  // if help is specified, or positional args aren't present,
  // or more than one loadX flag
  if(    vm.count("help")
     || !vm.count("inputFile")
     || !vm.count("outputFile")
    )
  {
    cerr << "Usage: "
      << argv[0] << " [--inputFile=]dark.bmp [--outputFile=]bright.bmp [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

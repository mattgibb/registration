#include "itkImage.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageMaskSpatialObject.h"

// 3-D registration
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

// 2-D registration
#include "itkRegularStepGradientDescentOptimizer.h"

// File IO
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"


// my files
#include "CommandIterationUpdate.hpp"
#include "Stack.hpp"

using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 9 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " histoDir regularExpression ";
		cerr << "sortingExpression mriFile transformFile3D";
		cerr << "outputMRI outputStack outputMask\n\n";
		cerr << "histoDir should have no trailing slash\n";
		cerr << "regularExpression should be enclosed in single quotes\n";
		cerr << "sortingExpression should be an integer ";
		cerr << "representing the group in the regexp\n\n";
    exit(EXIT_FAILURE);
  }
  
}

vector< string > getFileNames(char const *argv[]) {
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

template<typename ImageType>
void writeImage(typename ImageType::Pointer image, char const *fileName) {
	typedef itk::ImageFileWriter< ImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( image );
  
  writer->SetFileName( fileName );
	
  try {
  	writer->Update();
  }
	catch( itk::ExceptionObject & err ) {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
		exit(EXIT_FAILURE);
	}
}

int main (int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	
	// initialise stack object
	Stack stack( getFileNames(argv) );
	
	
	// perform 3-D registration
  typedef unsigned char MRIPixelType;
  typedef itk::Image< MRIPixelType, 3 > MRIVolumeType;
	typedef itk::VersorRigid3DTransform< double > TransformType3D;
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType3D;
	typedef itk::MattesMutualInformationImageToImageMetric< Stack::VolumeType, MRIVolumeType > MetricType3D;
  // used for registration
  typedef itk::LinearInterpolateImageFunction< MRIVolumeType, double > LinearInterpolatorType3D;
  // used for final resampling
	typedef itk::NearestNeighborInterpolateImageFunction< MRIVolumeType, double > NearestNeighborInterpolatorType3D;
  typedef itk::ImageRegistrationMethod< Stack::VolumeType, MRIVolumeType > RegistrationType3D;
	
	
	MetricType3D::Pointer metric3D = MetricType3D::New();
  OptimizerType3D::Pointer optimizer3D = OptimizerType3D::New();
  LinearInterpolatorType3D::Pointer interpolator3D = LinearInterpolatorType3D::New();
  RegistrationType3D::Pointer registration3D = RegistrationType3D::New();
  TransformType3D::Pointer  transform3D = TransformType3D::New();

	// Number of spatial samples should be ~20% of pixels for detailed images, see ITK Software Guide p341
	// Total pixels in MRI: 128329344
	// metric3D.UseAllPixelsOn() // Uses all the pixels in the fixed image, rather than just a sample
	metric3D->SetNumberOfSpatialSamples( 12800000 );
	// Number of bins recommended to be about 50, see ITK Software Guide p341
	metric3D->SetNumberOfHistogramBins( 50 );
	typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
	MaskType3D::Pointer stackMask = MaskType3D::New();
	stackMask->SetImage( stack.GetMaskVolume() );
	metric3D->SetFixedImageMask( stackMask );
	
  registration3D->SetMetric( metric3D );
  registration3D->SetOptimizer( optimizer3D );
  registration3D->SetInterpolator( interpolator3D );
  registration3D->SetTransform( transform3D );
	
	
  typedef itk::ImageFileReader< MRIVolumeType > MRIVolumeReaderType;
  MRIVolumeReaderType::Pointer mriVolumeReader = MRIVolumeReaderType::New();
	mriVolumeReader->SetFileName( argv[4] );
	
	typedef itk::RescaleIntensityImageFilter< MRIVolumeType, MRIVolumeType > RescaleFilterType;
	RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
	rescaleFilter->SetInput( mriVolumeReader->GetOutput() );
	rescaleFilter->SetOutputMinimum( 0 );
	rescaleFilter->SetOutputMaximum( 255 );
	MRIVolumeType::Pointer mriVolume = rescaleFilter->GetOutput();
	
  registration3D->SetFixedImage( stack.GetVolume() );
  registration3D->SetMovingImage( mriVolume );
	
  registration3D->SetFixedImageRegion( stack.GetVolume()->GetBufferedRegion() );


	// Set up transform initializer
  typedef itk::CenteredTransformInitializer< TransformType3D,
																						 Stack::VolumeType,
																						 MRIVolumeType > TransformInitializerType;
  TransformInitializerType::Pointer initializer = TransformInitializerType::New();

  initializer->SetTransform( transform3D );
  initializer->SetFixedImage(  stack.GetVolume() );
  initializer->SetMovingImage( mriVolume );

  //  The use of the geometrical centers is selected by calling
  //  GeometryOn() while the use of center of mass is selected by
  //  calling MomentsOn(). Below we select the center of mass mode.
  // initializer->GeometryOn();
  initializer->MomentsOn();
  initializer->InitializeTransform();

  //  The rotation part of the transform is initialized using a
  //  Versor which is simply a unit quaternion. The
  //  VersorType can be obtained from the transform traits. The versor
  //  itself defines the type of the vector used to indicate the rotation axis.
  //  This trait can be extracted as VectorType. The following lines
  //  create a versor object and initialize its parameters by passing a
  //  rotation axis and an angle.
  typedef TransformType3D::VersorType VersorType;
  typedef VersorType::VectorType VectorType;
  
  VersorType rotation;
  VectorType axis;
  
  axis[0] = 1.0;
  axis[1] = 0.0;
  axis[2] = -1.0;
  
  const double angle = M_PI;
  
  rotation.Set(  axis, angle );
  
  transform3D->SetRotation( rotation );
	
  //  We now pass the parameters of the current transform as the initial
  //  parameters to be used when the registration process starts.
  registration3D->SetInitialTransformParameters( transform3D->GetParameters() );
  

	// Construct and configure the optimiser
  typedef OptimizerType3D::ScalesType OptimizerScalesType3D;
  OptimizerScalesType3D optimizerScales3D( transform3D->GetNumberOfParameters() );
  const double translationScale = 1.0 / 40000.0;
  
  optimizerScales3D[0] = 1.0;
  optimizerScales3D[1] = 1.0;
  optimizerScales3D[2] = 1.0;
  optimizerScales3D[3] = translationScale;
  optimizerScales3D[4] = translationScale;
  optimizerScales3D[5] = translationScale;
  
  optimizer3D->SetScales( optimizerScales3D );
  
  optimizer3D->SetMaximumStepLength( 0.10  ); 
  optimizer3D->SetMinimumStepLength( 0.00001 );
  
  optimizer3D->SetNumberOfIterations( 5 );
  
  
  // Create the Command observer and register it with the optimizer.
	typedef CommandIterationUpdate< itk::VersorRigid3DTransformOptimizer > ObserverType3D;
	ObserverType3D::Pointer observer3D = ObserverType3D::New();
  optimizer3D->AddObserver( itk::IterationEvent(), observer3D );
  
  // Begin registration
  try
    {
    registration3D->StartRegistration(); 
    std::cout << "Optimizer stop condition: "
              << registration3D->GetOptimizer()->GetStopConditionDescription()
              << std::endl;
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return EXIT_FAILURE;
    } 
  
	// Get final parameters
  OptimizerType3D::ParametersType finalParameters3D = registration3D->GetLastTransformParameters();
  
  const double versorX              = finalParameters3D[0];
  const double versorY              = finalParameters3D[1];
  const double versorZ              = finalParameters3D[2];
  const double finalTranslationX    = finalParameters3D[3];
  const double finalTranslationY    = finalParameters3D[4];
  const double finalTranslationZ    = finalParameters3D[5];
  
  const unsigned int numberOfIterations = optimizer3D->GetCurrentIteration();
  
  const double bestValue = optimizer3D->GetValue();
  
	// Write final transform to file
	itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();

  writer->SetInput( transform3D );
  // writer->AddTransform( anotherTransform );

  writer->SetFileName( argv[5] );

  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    return 0;
    }
	
  
  // Print out results
  std::cout << std::endl << std::endl;
  std::cout << "Result = " << std::endl;
  std::cout << " versor X      = " << versorX  << std::endl;
  std::cout << " versor Y      = " << versorY  << std::endl;
  std::cout << " versor Z      = " << versorZ  << std::endl;
  std::cout << " Translation X = " << finalTranslationX  << std::endl;
  std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;
  
	// TESTING
	cout << "transform3D->GetParameters(): " << transform3D->GetParameters() << endl << endl;
	cout << "finalParameters3D: " << finalParameters3D << endl << endl;
  // END TESTING
  transform3D->SetParameters( finalParameters3D );
	
  TransformType3D::MatrixType matrix3D = transform3D->GetRotationMatrix();
  TransformType3D::OffsetType offset3D = transform3D->GetOffset();
  
  std::cout << "Matrix = " << std::endl << matrix3D << std::endl;
  std::cout << "Offset = " << std::endl << offset3D << std::endl;  


  typedef itk::ResampleImageFilter< MRIVolumeType, MRIVolumeType > ResampleFilterType;
  TransformType3D::Pointer finalTransform = TransformType3D::New();
  
  finalTransform->SetCenter( transform3D->GetCenter() );
  finalTransform->SetParameters( finalParameters3D );
  finalTransform->SetFixedParameters( transform3D->GetFixedParameters() );
  
  ResampleFilterType::Pointer resampler3D = ResampleFilterType::New();
  
  Stack::VolumeType::Pointer fixedImage3D = stack.GetVolume();
  
  resampler3D->SetSize( fixedImage3D->GetLargestPossibleRegion().GetSize() );
  resampler3D->SetOutputOrigin( fixedImage3D->GetOrigin() );
  resampler3D->SetOutputSpacing( fixedImage3D->GetSpacing() );
  resampler3D->SetOutputDirection( fixedImage3D->GetDirection() );
  resampler3D->SetDefaultPixelValue( 100 );
  
  // resampler3D->SetTransform( finalTransform );
  resampler3D->SetTransform( transform3D );
	resampler3D->SetInterpolator( interpolator3D );
  resampler3D->SetInput( mriVolume );

	writeImage< MRIVolumeType >(resampler3D->GetOutput(), argv[6] );
    
  
	// extract 2-D MRI slices
	// typedef itk::ExtractImageFilter< OutputImageType, OutputSliceType > ExtractFilterType;
  // ExtractFilterType::Pointer extractor = ExtractFilterType::New();
  
	
	
	// perform 2-D registration
	typedef CommandIterationUpdate< itk::RegularStepGradientDescentOptimizer > ObserverType2D;
	ObserverType2D::Pointer observer2D = ObserverType2D::New();
	
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[7] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), argv[8] );
		
	
  return EXIT_SUCCESS;
}
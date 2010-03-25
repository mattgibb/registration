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
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiResolutionPyramidImageFilter.h"
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
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "Framework3D.hpp"

using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 10 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " histoDir regularExpression ";
		cerr << "sortingExpression mriFile transformFile3D ";
		cerr << "outputMRI outputStack outputMask outputParameters\n\n";
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
	cout << "About to begin stack construction..." << endl;
	Stack stack( getFileNames(argv) );
	cout << "Just finished stack construction..." << endl;
	
	// TODO Make Framework::MRIVolumeType reference new MRI class instead, e.g. MRI::VolumeType
	typedef Framework3D::MRIVolumeType MRIVolumeType;
	
	// perform 3-D registration
	MRI mriVolume( argv[4] );
	Framework3D framework3D(stack, mriVolume);

	// Set up transform initializer
  typedef itk::CenteredTransformInitializer< Framework3D::TransformType3D,
																						 Stack::VolumeType,
																						 MRI::MRIVolumeType > TransformInitializerType;
  TransformInitializerType::Pointer initializer = TransformInitializerType::New();
	
  initializer->SetTransform( framework3D.transform3D );
  initializer->SetFixedImage(  stack.GetVolume() );
  initializer->SetMovingImage( mriVolume.GetVolume() );
	
  //  The use of the geometrical centers is selected by calling
  //  GeometryOn() while the use of center of mass is selected by
  //  calling MomentsOn(). Below we select the center of mass mode.
  initializer->GeometryOn();
  // initializer->MomentsOn();
  initializer->InitializeTransform();
	cout << "Finished initialising transform..." << endl;

  //  The rotation part of the transform is initialized using a
  //  Versor which is simply a unit quaternion. The
  //  VersorType can be obtained from the transform traits. The versor
  //  itself defines the type of the vector used to indicate the rotation axis.
  //  This trait can be extracted as VectorType. The following lines
  //  create a versor object and initialize its parameters by passing a
  //  rotation axis and an angle.
  typedef Framework3D::TransformType3D::VersorType VersorType;
  typedef VersorType::VectorType VectorType;
  
  VersorType rotation;
  VectorType axis;
  
  axis[0] = 1.0;
  axis[1] = 0.0;
  axis[2] = -1.0;
  
  const double angle = M_PI;
  
  rotation.Set(  axis, angle );
  
  framework3D.transform3D->SetRotation( rotation );
	
  //  We now pass the parameters of the current transform as the initial
  //  parameters to be used when the registration process starts.
  framework3D.registration3D->SetInitialTransformParameters( framework3D.transform3D->GetParameters() );
  
	
	// Construct and configure the optimiser
  typedef Framework3D::OptimizerType3D::ScalesType OptimizerScalesType3D;
  OptimizerScalesType3D optimizerScales3D( framework3D.transform3D->GetNumberOfParameters() );
  const double translationScale = 1.0 / 15000.0;
  // const double translationScale = 1.0 / 5000.0;
  
  optimizerScales3D[0] = 1.0;
  optimizerScales3D[1] = 1.0;
  optimizerScales3D[2] = 1.0;
  optimizerScales3D[3] = translationScale;
  optimizerScales3D[4] = translationScale;
  optimizerScales3D[5] = translationScale;
  
  framework3D.optimizer3D->SetScales( optimizerScales3D );
    
  // Create the command observers and register them with the optimiser.
	typedef StdOutIterationUpdate< Framework3D::OptimizerType3D > StdOutObserverType3D;
	typedef FileIterationUpdate< Framework3D::OptimizerType3D > FileObserverType3D;
	typedef MultiResRegistrationCommand< Framework3D::RegistrationType3D,
	                                     Framework3D::OptimizerType3D,
	                                     Framework3D::MetricType3D > MultiResCommandType;
	StdOutObserverType3D::Pointer stdOutObserver3D = StdOutObserverType3D::New();
	FileObserverType3D::Pointer   fileObserver3D   = FileObserverType3D::New();
	MultiResCommandType::Pointer  multiResCommand  = MultiResCommandType::New();
  framework3D.optimizer3D->AddObserver( itk::IterationEvent(), stdOutObserver3D );
  framework3D.optimizer3D->AddObserver( itk::IterationEvent(), fileObserver3D   );
  framework3D.registration3D->AddObserver( itk::IterationEvent(), multiResCommand  );

	ofstream output;
	output.open( argv[9] );
	fileObserver3D->SetOfstream( &output );
	
	framework3D.registration3D->SetNumberOfLevels( 4 );
	
	cout << "About to begin registration..." << endl;
	
  // Begin registration
  try
    {
    framework3D.registration3D->StartRegistration();
    cout << "Optimizer stop condition: "
         << framework3D.registration3D->GetOptimizer()->GetStopConditionDescription() << endl << endl;

    }
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }
  
  output.close();
	
	
	
	// Get final parameters
  Framework3D::OptimizerType3D::ParametersType finalParameters3D = framework3D.registration3D->GetLastTransformParameters();
  
	// Write final transform to file
	itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();

  writer->SetInput( framework3D.transform3D );
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
	
  Framework3D::TransformType3D::MatrixType matrix3D = framework3D.transform3D->GetRotationMatrix();
  Framework3D::TransformType3D::OffsetType offset3D = framework3D.transform3D->GetOffset();
  
  std::cout << "Matrix = " << std::endl << matrix3D << std::endl;
  std::cout << "Offset = " << std::endl << offset3D << std::endl;  
	
  typedef itk::ResampleImageFilter< MRI::MRIVolumeType, MRI::MRIVolumeType > ResampleFilterType;
  Framework3D::TransformType3D::Pointer finalTransform = Framework3D::TransformType3D::New();
  
  // TODO Check to see if using transform3D directly instead of finalTransform makes a difference
  finalTransform->SetCenter( framework3D.transform3D->GetCenter() );
  finalTransform->SetParameters( finalParameters3D );
  finalTransform->SetFixedParameters( framework3D.transform3D->GetFixedParameters() );
  
  ResampleFilterType::Pointer resampler3D = ResampleFilterType::New();
  
  Stack::VolumeType::Pointer fixedImage3D = stack.GetVolume();
  
  resampler3D->SetSize( fixedImage3D->GetLargestPossibleRegion().GetSize() );
  resampler3D->SetOutputOrigin( fixedImage3D->GetOrigin() );
  resampler3D->SetOutputSpacing( fixedImage3D->GetSpacing() );
  resampler3D->SetOutputDirection( fixedImage3D->GetDirection() );
  resampler3D->SetDefaultPixelValue( 100 );
  
  // resampler3D->SetTransform( finalTransform );
  resampler3D->SetTransform( framework3D.transform3D );
	resampler3D->SetInterpolator( framework3D.interpolator3D );
  resampler3D->SetInput( mriVolume.GetVolume() );

	writeImage< MRI::MRIVolumeType >(resampler3D->GetOutput(), argv[6] );
    
  // used for final resampling
	typedef itk::NearestNeighborInterpolateImageFunction< MRI::MRIVolumeType, double > NearestNeighborInterpolatorType3D;
  
	// extract 2-D MRI slices
	// typedef itk::ExtractImageFilter< OutputImageType, OutputSliceType > ExtractFilterType;
  // ExtractFilterType::Pointer extractor = ExtractFilterType::New();
  
	
	
	// perform 2-D registration
	typedef StdOutIterationUpdate< itk::RegularStepGradientDescentOptimizer > ObserverType2D;
	ObserverType2D::Pointer observer2D = ObserverType2D::New();
	
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[7] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), argv[8] );
		
	
  return EXIT_SUCCESS;
}
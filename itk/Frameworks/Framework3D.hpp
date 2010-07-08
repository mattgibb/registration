// This object constructs and encapsulates the 3D registration framework.

#ifndef FRAMEWORK3D_HPP_
#define FRAMEWORK3D_HPP_
// YAML config reader
#include "yaml.h"

// 3-D registration
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImageMaskSpatialObject.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "MRI.hpp"

class Framework3D {
public:
	typedef itk::VersorRigid3DTransform< double > TransformType;
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
	typedef itk::MattesMutualInformationImageToImageMetric< Stack::VolumeType, MRI::VolumeType > MetricType;
  typedef itk::LinearInterpolateImageFunction< MRI::VolumeType, double > LinearInterpolatorType;
	typedef itk::MultiResolutionImageRegistrationMethod< Stack::VolumeType, MRI::VolumeType > RegistrationType;
	typedef itk::MultiResolutionPyramidImageFilter< Stack::VolumeType, Stack::VolumeType > FixedImagePyramidType;
  typedef itk::MultiResolutionPyramidImageFilter< MRI::VolumeType, MRI::VolumeType > MovingImagePyramidType;
	typedef itk::ImageMaskSpatialObject< 3 > MaskType;
	typedef StdOutIterationUpdate< OptimizerType > StdOutObserverType;
	typedef FileIterationUpdate< OptimizerType > FileObserverType;
	typedef MultiResRegistrationCommand< RegistrationType, OptimizerType, MetricType > MultiResCommandType;
  
	
	Stack *stack;
	MRI *mriVolume;
	MetricType::Pointer metric;
	OptimizerType::Pointer optimizer;
	LinearInterpolatorType::Pointer interpolator;
	RegistrationType::Pointer registration;
  TransformType::Pointer transform;
	FixedImagePyramidType::Pointer fixedImagePyramid;
	MovingImagePyramidType::Pointer movingImagePyramid;
	StdOutObserverType::Pointer stdOutObserver;
	FileObserverType::Pointer fileObserver;
	MultiResCommandType::Pointer multiResCommand;
	ofstream observerOutput;
  YAML::Node& registrationParameters;
	
	
	Framework3D(Stack *inputStack, MRI *inputMriVolume, YAML::Node& parameters):
	stack(inputStack),
	mriVolume(inputMriVolume),
	registrationParameters(parameters) {
	    
    initializeRegistrationComponents();
    wireUpRegistrationComponents();
    
		registration->SetFixedImage( stack->GetVolume() );
	  registration->SetMovingImage( mriVolume->GetVolume() );
	  
	  // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
	  registration->SetFixedImageRegion( stack->GetVolume()->GetLargestPossibleRegion() );
	  // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
	  
		metric->SetFixedImageMask( stack->Get3DMask() );
    metric->SetMovingImageMask( mriVolume->Get3DMask() );
		
    unsigned int levels; parameters["levels"] >> levels;
	  registration->SetNumberOfLevels( levels );
		
    initializeTransformParameters();
		
    setUpObservers();
		
    setOptimizerTranslationScale();
	}
	
	void initializeRegistrationComponents() {
		registration = RegistrationType::New();
		metric = MetricType::New();
	  optimizer = OptimizerType::New();
	  interpolator = LinearInterpolatorType::New();
	  transform = TransformType::New();
	  fixedImagePyramid = FixedImagePyramidType::New();
	  movingImagePyramid = MovingImagePyramidType::New();
	}
	
	void wireUpRegistrationComponents() {
		registration->SetMetric( metric );
	  registration->SetOptimizer( optimizer );
	  registration->SetInterpolator( interpolator );
	  registration->SetTransform( transform );
	  registration->SetFixedImagePyramid( fixedImagePyramid );
	  registration->SetMovingImagePyramid( movingImagePyramid );
	}
	
	void initializeTransformParameters() {
		initializeTranslationParameters();
		initializeRotationParameters();
	}
	
	void initializeTranslationParameters() {
		typedef itk::CenteredTransformInitializer< TransformType,
																							 Stack::VolumeType,
																							 MRI::VolumeType > TransformInitializerType;
	  TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    
	  initializer->SetTransform( transform );
	  initializer->SetFixedImage( stack->GetVolume() );
	  initializer->SetMovingImage( mriVolume->GetVolume() );
    
	  // initializer->MomentsOn();
	  initializer->GeometryOn();
	  
	  initializer->InitializeTransform();
	}
	
	void initializeRotationParameters() {
	  typedef TransformType::VersorType VersorType;
	  typedef VersorType::VectorType VectorType;
    
	  VersorType rotation;
	  VectorType axis;
    
	  axis[0] = 1.0;
	  axis[1] = 0.0;
	  axis[2] = -1.0;
    
	  const double angle = M_PI;
    
	  rotation.Set( axis, angle );
    
	  transform->SetRotation( rotation );
	}
	
	void setUpObservers() {
		// Create the command observers
		stdOutObserver = StdOutObserverType::New();
		fileObserver   = FileObserverType::New();
		multiResCommand  = MultiResCommandType::New();
		
	  // register the observers
	  optimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
	  optimizer->AddObserver( itk::IterationEvent(), fileObserver );
	  registration->AddObserver( itk::IterationEvent(), multiResCommand );
	
	  // add output to fileObserver
		fileObserver->SetOfstream( &observerOutput );
		
	  // pass parameters to multiResCommand so it can configure itself
    multiResCommand->configure( registrationParameters );
	}
	
	void setOptimizerTranslationScale() {
    double translationScale;
    registrationParameters["optimizerTranslationScale3D"] >> translationScale;
    
		OptimizerType::ScalesType optimizerScales( transform->GetNumberOfParameters() );
    
	  optimizerScales[0] = 1.0;
	  optimizerScales[1] = 1.0;
	  optimizerScales[2] = 1.0;
	  optimizerScales[3] = translationScale;
	  optimizerScales[4] = translationScale;
	  optimizerScales[5] = translationScale;
    
	  optimizer->SetScales( optimizerScales );
	}
	
	void StartRegistration(string outputFileName) {
		observerOutput.open( outputFileName.c_str() );
    
    registration->SetInitialTransformParameters( transform->GetParameters() );
    
	  try
	    {
	    registration->StartRegistration();
	    cout << "Optimizer stop condition: "
	         << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
	    }
	  catch( itk::ExceptionObject & err ) 
	    { 
	    std::cerr << "ExceptionObject caught !" << std::endl;
	    std::cerr << err << std::endl;
	    exit(EXIT_FAILURE);
	    }
    
	  observerOutput.close();
	  
    mriVolume->SetTransformParameters( transform );
	}
	
protected:
};
#endif

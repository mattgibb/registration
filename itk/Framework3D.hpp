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
	typedef itk::VersorRigid3DTransform< double > TransformType3D;
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType3D;
	typedef itk::MattesMutualInformationImageToImageMetric< Stack::VolumeType, MRI::VolumeType > MetricType3D;
  typedef itk::LinearInterpolateImageFunction< MRI::VolumeType, double > LinearInterpolatorType3D;
	typedef itk::MultiResolutionImageRegistrationMethod< Stack::VolumeType, MRI::VolumeType > RegistrationType3D;
	typedef itk::MultiResolutionPyramidImageFilter< Stack::VolumeType, Stack::VolumeType > FixedImagePyramidType;
  typedef itk::MultiResolutionPyramidImageFilter< MRI::VolumeType, MRI::VolumeType > MovingImagePyramidType;
	typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
	typedef StdOutIterationUpdate< OptimizerType3D > StdOutObserverType3D;
	typedef FileIterationUpdate< OptimizerType3D > FileObserverType3D;
	typedef MultiResRegistrationCommand< RegistrationType3D, OptimizerType3D, MetricType3D > MultiResCommandType;
  
	
	Stack *stack;
	MRI *mriVolume;
	MetricType3D::Pointer metric3D;
	OptimizerType3D::Pointer optimizer3D;
	LinearInterpolatorType3D::Pointer interpolator3D;
	RegistrationType3D::Pointer registration3D;
  TransformType3D::Pointer transform3D;
	FixedImagePyramidType::Pointer fixedImagePyramid;
	MovingImagePyramidType::Pointer movingImagePyramid;
	StdOutObserverType3D::Pointer stdOutObserver3D;
	FileObserverType3D::Pointer fileObserver3D;
	MultiResCommandType::Pointer multiResCommand;
	ofstream observerOutput;
  YAML::Node& registrationParameters;
	
	
	Framework3D(Stack *inputStack, MRI *inputMriVolume, YAML::Node& parameters):
	  stack(inputStack),
	  mriVolume(inputMriVolume),
	  registrationParameters(parameters)
	  {	
		initializeRegistrationComponents();
		
		wireUpRegistrationComponents();
		    
		registration3D->SetFixedImage( stack->GetVolume() );
	  registration3D->SetMovingImage( mriVolume->GetVolume() );
	  
	  registration3D->SetFixedImageRegion( stack->GetVolume()->GetLargestPossibleRegion() );
	  
		metric3D->SetFixedImageMask( stack->GetMask3D() );
    metric3D->SetMovingImageMask( mriVolume->GetMask3D() );
		
	  registration3D->SetNumberOfLevels( 4 );
		
		initializeTransformParameters();
		
		setUpObservers();
		
    setOptimizerTranslationScale();
	}
	
	void initializeRegistrationComponents() {
		registration3D = RegistrationType3D::New();
		metric3D = MetricType3D::New();
	  optimizer3D = OptimizerType3D::New();
	  interpolator3D = LinearInterpolatorType3D::New();
	  transform3D = TransformType3D::New();
	  fixedImagePyramid = FixedImagePyramidType::New();
	  movingImagePyramid = MovingImagePyramidType::New();
	}
	
	void wireUpRegistrationComponents() {
		registration3D->SetMetric( metric3D );
	  registration3D->SetOptimizer( optimizer3D );
	  registration3D->SetInterpolator( interpolator3D );
	  registration3D->SetTransform( transform3D );
	  registration3D->SetFixedImagePyramid( fixedImagePyramid );
	  registration3D->SetMovingImagePyramid( movingImagePyramid );
	}
	
	void initializeTransformParameters() {
		initializeTranslationParameters();
		initializeRotationParameters();
		
	  registration3D->SetInitialTransformParameters( transform3D->GetParameters() );
	}
	
	void initializeTranslationParameters() {
		typedef itk::CenteredTransformInitializer< TransformType3D,
																							 Stack::VolumeType,
																							 MRI::VolumeType > TransformInitializerType;
	  TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    
	  initializer->SetTransform( transform3D );
	  initializer->SetFixedImage( stack->GetVolume() );
	  initializer->SetMovingImage( mriVolume->GetVolume() );
    
	  // initializer->MomentsOn();
	  initializer->GeometryOn();
	  
	  initializer->InitializeTransform();
	}
	
	void initializeRotationParameters() {
	  typedef TransformType3D::VersorType VersorType;
	  typedef VersorType::VectorType VectorType;
    
	  VersorType rotation;
	  VectorType axis;
    
	  axis[0] = 1.0;
	  axis[1] = 0.0;
	  axis[2] = -1.0;
    
	  const double angle = M_PI;
    
	  rotation.Set( axis, angle );
    
	  transform3D->SetRotation( rotation );
	}
	
	void setUpObservers() {
		// Create the command observers
		stdOutObserver3D = StdOutObserverType3D::New();
		fileObserver3D   = FileObserverType3D::New();
		multiResCommand  = MultiResCommandType::New();
		
		// register the observers
	  optimizer3D->AddObserver( itk::IterationEvent(), stdOutObserver3D );
	  optimizer3D->AddObserver( itk::IterationEvent(), fileObserver3D );
	  registration3D->AddObserver( itk::IterationEvent(), multiResCommand );
	
	  // add output to fileObserver3D
		fileObserver3D->SetOfstream( &observerOutput );
		
		// set maximum number of iterations at each level
    itk::Array<unsigned int> maxIterations(4);
    for(int i=0; i<4; i++)  { registrationParameters["maxIterations"][i] >> maxIterations[i]; }
    multiResCommand->setMaxIterations(maxIterations);
	}
	
	void setOptimizerTranslationScale() {
    string translationScaleStr;
    registrationParameters["optimizerTranslationScale3D"] >> translationScaleStr;
    double translationScale = atof(translationScaleStr.c_str());
    
		OptimizerType3D::ScalesType optimizerScales3D( transform3D->GetNumberOfParameters() );
    
	  optimizerScales3D[0] = 1.0;
	  optimizerScales3D[1] = 1.0;
	  optimizerScales3D[2] = 1.0;
	  optimizerScales3D[3] = translationScale;
	  optimizerScales3D[4] = translationScale;
	  optimizerScales3D[5] = translationScale;
    
	  optimizer3D->SetScales( optimizerScales3D );
	}
	
	void beginRegistration(char const *outputFileName) {
		observerOutput.open( outputFileName );
    
	  try
	    {
	    registration3D->StartRegistration();
	    cout << "Optimizer stop condition: "
	         << registration3D->GetOptimizer()->GetStopConditionDescription() << endl << endl;
	    }
	  catch( itk::ExceptionObject & err ) 
	    { 
	    std::cerr << "ExceptionObject caught !" << std::endl;
	    std::cerr << err << std::endl;
	    exit(EXIT_FAILURE);
	    }
    
	  observerOutput.close();
	}
	
protected:
};
#endif

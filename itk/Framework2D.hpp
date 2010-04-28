// This object constructs and encapsulates the 2D registration framework.

#ifndef FRAMEWORK2D_HPP_
#define FRAMEWORK2D_HPP_

// YAML config reader
#include "yaml.h"

// ITK includes
#include "itkImageRegistrationMethod.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImageMaskSpatialObject.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "Stack.hpp"
#include "MRI.hpp"

class Framework2D {
public:
	typedef itk::CenteredRigid2DTransform< double > TransformType2D;
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType2D;
	typedef itk::MattesMutualInformationImageToImageMetric< MRI::SliceType, Stack::SliceType > MetricType2D;
  typedef itk::LinearInterpolateImageFunction< Stack::SliceType, double > LinearInterpolatorType2D;
	typedef itk::ImageRegistrationMethod< MRI::SliceType, Stack::SliceType > RegistrationType2D;
	typedef itk::ImageMaskSpatialObject< 2 > MaskType2D;
	
	
	Stack *stack;
	MRI *mriVolume;
	RegistrationType2D::Pointer registration2D;
	MetricType2D::Pointer metric2D;
	OptimizerType2D::Pointer optimizer2D;
	LinearInterpolatorType2D::Pointer interpolator2D;
  TransformType2D::Pointer transform2D;
	ofstream observerOutput;
	
	
	Framework2D(Stack *inputStack, MRI *inputMriVolume) {
		this->stack = inputStack;
		this->mriVolume = inputMriVolume;
		
		initializeRegistrationComponents();
		
		wireUpRegistrationComponents();
		    
    unsigned int number_of_slices = stack->originalImages.size();
    
    for(unsigned int slice_number = 0; slice_number < number_of_slices; slice++)
    {
  		registration2D->SetFixedImage( stack->originalImages[slice_number] );
  	  registration2D->SetMovingImage( mriVolume->GetVolume() );
	
  	  registration3D->SetFixedImageRegion( stack->GetVolume()->GetLargestPossibleRegion() );
	    
  		metric3D->SetFixedImageMask( stack->GetMask3D() );
  		metric3D->SetMovingImageMask( mriVolume->GetMask3D() );
				
  	  registration3D->SetNumberOfLevels( 4 );
		
    }
    
		initializeTransformParameters();
		
		setUpObservers();
	}
	
	void initializeRegistrationComponents() {
		registration2D = RegistrationType2D::New();
		metric2D = MetricType2D::New();
	  optimizer2D = OptimizerType2D::New();
	  interpolator2D = LinearInterpolatorType2D::New();
	  transform2D = TransformType2D::New();
	}
	
	void wireUpRegistrationComponents() {
		registration2D->SetMetric( metric2D );
	  registration2D->SetOptimizer( optimizer2D );
	  registration2D->SetInterpolator( interpolator2D );
	  registration2D->SetTransform( transform2D );
	}
		
	void setOptimizerTranslationScale(const double translationScale) {
		OptimizerType2D::ScalesType optimizerScales2D( transform2D->GetNumberOfParameters() );
    
	  optimizerScales2D[0] = 1.0;
	  optimizerScales2D[1] = 1.0;
	  optimizerScales2D[2] = 1.0;
	  optimizerScales2D[3] = translationScale;
	  optimizerScales2D[4] = translationScale;
	  optimizerScales2D[5] = translationScale;
    
	  optimizer2D->SetScales( optimizerScales2D );
	}
	
	void initializeTransformParameters() {
		initializeTranslationParameters();
		initializeRotationParameters();
		
	  registration3D->SetInitialTransformParameters( transform3D->GetParameters() );
	}
	
	void initializeTranslationParameters() {
		typedef itk::CenteredTransformInitializer< TransformType2D,
																							 Stack::SliceType,
																							 MRI::SliceType > TransformInitializerType;
	  TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    
	  initializer->SetTransform( transform2D );
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

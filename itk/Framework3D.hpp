// This object constructs and encapsulates the 3D registration framework.

#ifndef FRAMEWORK3D_HPP_
#define FRAMEWORK3D_HPP_

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
 
	
	MetricType3D::Pointer metric3D;
	OptimizerType3D::Pointer optimizer3D;
	LinearInterpolatorType3D::Pointer interpolator3D;
	RegistrationType3D::Pointer registration3D;
  TransformType3D::Pointer transform3D;
	FixedImagePyramidType::Pointer fixedImagePyramid;
	MovingImagePyramidType::Pointer movingImagePyramid;
	MaskType3D::Pointer stackMask;
	StdOutObserverType3D::Pointer stdOutObserver3D;
	FileObserverType3D::Pointer fileObserver3D;
	MultiResCommandType::Pointer multiResCommand;
	ofstream observerOutput;
	
	
	Framework3D(Stack stack, MRI mriVolume) {
		initialiseRegistrationComponents();
		wireUpRegistrationComponents();
		setOptimizerTranslationScale(1.0 / 15000.0);

		metric3D->SetFixedImageMask( stack.GetMask3D() );
		
		registration3D->SetFixedImage( stack.GetVolume() );
	  registration3D->SetMovingImage( mriVolume.GetVolume() );
    
	  registration3D->SetFixedImageRegion( stack.GetVolume()->GetBufferedRegion() );
	  
	  registration3D->SetNumberOfLevels( 4 );
		
	  
		// Set up transform initializer
	  typedef itk::CenteredTransformInitializer< TransformType3D,
																							 Stack::VolumeType,
																							 MRI::VolumeType > TransformInitializerType;
	  TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    
	  initializer->SetTransform( transform3D );
	  initializer->SetFixedImage(  stack.GetVolume() );
	  initializer->SetMovingImage( mriVolume.GetVolume() );
    
	  //  The use of the geometrical centers is selected by calling
	  //  GeometryOn() while the use of center of mass is selected by
	  //  calling MomentsOn(). Below we select the center of mass mode.
	  initializer->GeometryOn();
	  // initializer->MomentsOn();
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
	
		setUpObservers();
		
	}
	
	void initialiseRegistrationComponents() {
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
		
	void setOptimizerTranslationScale(const double scale) {
		OptimizerType3D::ScalesType optimizerScales3D( transform3D->GetNumberOfParameters() );
	  const double translationScale = scale;
    
	  optimizerScales3D[0] = 1.0;
	  optimizerScales3D[1] = 1.0;
	  optimizerScales3D[2] = 1.0;
	  optimizerScales3D[3] = translationScale;
	  optimizerScales3D[4] = translationScale;
	  optimizerScales3D[5] = translationScale;
    
	  optimizer3D->SetScales( optimizerScales3D );
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

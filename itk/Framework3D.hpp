// This object constructs and encapsulates the 3D registration framework.

#ifndef FRAMEWORK3D_HPP_
#define FRAMEWORK3D_HPP_

#include "Stack.hpp"
#include "MRI.hpp"

class Framework3D {
public:
  typedef MRI::MRIVolumeType MRIVolumeType;
	typedef itk::VersorRigid3DTransform< double > TransformType3D;
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType3D;
	typedef itk::MattesMutualInformationImageToImageMetric< Stack::VolumeType, MRIVolumeType > MetricType3D;
  typedef itk::LinearInterpolateImageFunction< MRIVolumeType, double > LinearInterpolatorType3D;
	typedef itk::MultiResolutionImageRegistrationMethod< Stack::VolumeType, MRIVolumeType > RegistrationType3D;
	typedef itk::MultiResolutionPyramidImageFilter< Stack::VolumeType, Stack::VolumeType > FixedImagePyramidType;
  typedef itk::MultiResolutionPyramidImageFilter< MRIVolumeType, MRIVolumeType > MovingImagePyramidType;
	typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
 
	
	MetricType3D::Pointer metric3D;
	OptimizerType3D::Pointer optimizer3D;
	LinearInterpolatorType3D::Pointer interpolator3D;
	RegistrationType3D::Pointer registration3D;
  TransformType3D::Pointer transform3D;
	FixedImagePyramidType::Pointer fixedImagePyramid;
	MovingImagePyramidType::Pointer movingImagePyramid;
	MaskType3D::Pointer stackMask;
	
	
	Framework3D(Stack stack, MRI mriVolume) {
		// create registration and components
	  registration3D = RegistrationType3D::New();
		metric3D = MetricType3D::New();
	  optimizer3D = OptimizerType3D::New();
	  interpolator3D = LinearInterpolatorType3D::New();
	  transform3D = TransformType3D::New();
	  fixedImagePyramid = FixedImagePyramidType::New();
	  movingImagePyramid = MovingImagePyramidType::New();
	  
	  // wire up registration
	  registration3D->SetMetric( metric3D );
	  registration3D->SetOptimizer( optimizer3D );
	  registration3D->SetInterpolator( interpolator3D );
	  registration3D->SetTransform( transform3D );
	  registration3D->SetFixedImagePyramid( fixedImagePyramid );
	  registration3D->SetMovingImagePyramid( movingImagePyramid );  
    
	  // Configure the metric
		// Number of spatial samples should be ~20% of pixels for detailed images, see ITK Software Guide p341
		// Total pixels in MRI: 128329344
		// metric3D.UseAllPixelsOn() // Uses all the pixels in the fixed image, rather than just a sample
		// metric3D->SetNumberOfSpatialSamples( 12800000 );
		// Number of bins recommended to be about 50, see ITK Software Guide p341
		metric3D->SetNumberOfHistogramBins( 50 );
		
	  // Configure the optimiser
	  typedef Framework3D::OptimizerType3D::ScalesType OptimizerScalesType3D;
	  OptimizerScalesType3D optimizerScales3D( transform3D->GetNumberOfParameters() );
	  const double translationScale = 1.0 / 15000.0;
	  // const double translationScale = 1.0 / 5000.0;
    
	  optimizerScales3D[0] = 1.0;
	  optimizerScales3D[1] = 1.0;
	  optimizerScales3D[2] = 1.0;
	  optimizerScales3D[3] = translationScale;
	  optimizerScales3D[4] = translationScale;
	  optimizerScales3D[5] = translationScale;
    
	  optimizer3D->SetScales( optimizerScales3D );
		
		stackMask = MaskType3D::New();
		stackMask->SetImage( stack.GetMaskVolume() );
		metric3D->SetFixedImageMask( stackMask );
		
		registration3D->SetFixedImage( stack.GetVolume() );
	  registration3D->SetMovingImage( mriVolume.GetVolume() );
    
	  registration3D->SetFixedImageRegion( stack.GetVolume()->GetBufferedRegion() );
	  
	  
	  
		// Set up transform initializer
	  typedef itk::CenteredTransformInitializer< TransformType3D,
																							 Stack::VolumeType,
																							 MRI::MRIVolumeType > TransformInitializerType;
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
		
	}
	
protected:
};
#endif

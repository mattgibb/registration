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
		metric3D = MetricType3D::New();
		// Number of spatial samples should be ~20% of pixels for detailed images, see ITK Software Guide p341
		// Total pixels in MRI: 128329344
		// metric3D.UseAllPixelsOn() // Uses all the pixels in the fixed image, rather than just a sample
		// metric3D->SetNumberOfSpatialSamples( 12800000 );
		// Number of bins recommended to be about 50, see ITK Software Guide p341
		metric3D->SetNumberOfHistogramBins( 50 );
	  optimizer3D = OptimizerType3D::New();
	  interpolator3D = LinearInterpolatorType3D::New();
	  registration3D = RegistrationType3D::New();
	  transform3D = TransformType3D::New();
	  fixedImagePyramid = FixedImagePyramidType::New();
	  movingImagePyramid = MovingImagePyramidType::New();
	
	  registration3D->SetMetric( metric3D );
	  registration3D->SetOptimizer( optimizer3D );
	  registration3D->SetInterpolator( interpolator3D );
	  registration3D->SetTransform( transform3D );
	  registration3D->SetFixedImagePyramid( fixedImagePyramid );
	  registration3D->SetMovingImagePyramid( movingImagePyramid );  

		stackMask = MaskType3D::New();
		stackMask->SetImage( stack.GetMaskVolume() );
		metric3D->SetFixedImageMask( stackMask );
		
		registration3D->SetFixedImage( stack.GetVolume() );
	  registration3D->SetMovingImage( mriVolume.GetVolume() );

	  registration3D->SetFixedImageRegion( stack.GetVolume()->GetBufferedRegion() );
	  
	
	}
	
protected:
};
#endif

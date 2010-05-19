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
  // typedef itk::CenteredRigid2DTransform< double > TransformType2D;
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType2D;
	typedef itk::MattesMutualInformationImageToImageMetric< MRI::SliceType, Stack::SliceType > MetricType2D;
  typedef itk::LinearInterpolateImageFunction< Stack::SliceType, double > LinearInterpolatorType2D;
	typedef itk::ImageRegistrationMethod< MRI::SliceType, Stack::SliceType > RegistrationType2D;
	typedef itk::ImageMaskSpatialObject< 2 > MaskType2D;
	typedef StdOutIterationUpdate< OptimizerType2D > StdOutObserverType2D;
	typedef FileIterationUpdate< OptimizerType2D > FileObserverType2D;
	
	
	Stack *stack;
	MRI *mriVolume;
	RegistrationType2D::Pointer registration2D;
	MetricType2D::Pointer metric2D;
	OptimizerType2D::Pointer optimizer2D;
	LinearInterpolatorType2D::Pointer interpolator2D;
	StdOutObserverType2D::Pointer stdOutObserver2D;
	FileObserverType2D::Pointer fileObserver2D;
	ofstream observerOutput;
	YAML::Node& registrationParameters;
  
	
	Framework2D(Stack *inputStack, MRI *inputMriVolume, YAML::Node& parameters):
	stack(inputStack),
	mriVolume(inputMriVolume),
	registrationParameters(parameters) {
	  
		initializeRegistrationComponents();
		
		wireUpRegistrationComponents();
		
		setUpObservers();
		
		setOptimizerTranslationScale();
	}
	
	void initializeRegistrationComponents() {
		registration2D = RegistrationType2D::New();
		metric2D = MetricType2D::New();
	  optimizer2D = OptimizerType2D::New();
	  interpolator2D = LinearInterpolatorType2D::New();
	}
	
	void wireUpRegistrationComponents() {
		registration2D->SetMetric( metric2D );
	  registration2D->SetOptimizer( optimizer2D );
	  registration2D->SetInterpolator( interpolator2D );
	}
		
	void setUpObservers() {
    // Create the command observers
		stdOutObserver2D = StdOutObserverType2D::New();
		fileObserver2D   = FileObserverType2D::New();
		
		// register the observers
	  optimizer2D->AddObserver( itk::IterationEvent(), stdOutObserver2D );
	  optimizer2D->AddObserver( itk::IterationEvent(), fileObserver2D );
	
	  // add output to fileObserver2D
		fileObserver2D->SetOfstream( &observerOutput );
		    
    // set parameters from config file
    double maxStepLength, minStepLength, maxIterations;
    registrationParameters["maxStepLength2D"] >> maxStepLength;
    registrationParameters["minStepLength2D"] >> minStepLength;
    registrationParameters["maxIterations2D"] >> maxIterations;
    optimizer2D->SetMaximumStepLength( maxStepLength );
    optimizer2D->SetMinimumStepLength( minStepLength );
    optimizer2D->SetNumberOfIterations( maxIterations );
	}
	
	void setOptimizerTranslationScale() {
	  double translationScale;
    registrationParameters["optimizerTranslationScale2D"] >> translationScale;

		OptimizerType2D::ScalesType optimizerScales2D( stack->GetTransform(0)->GetNumberOfParameters() );
    
	  optimizerScales2D[0] = 1.0;
	  optimizerScales2D[1] = translationScale;
	  optimizerScales2D[2] = translationScale;
	  optimizerScales2D[3] = translationScale;
	  optimizerScales2D[4] = translationScale;
    
	  optimizer2D->SetScales( optimizerScales2D );
	}
	
	void StartRegistration(char const *outputFileName) {
		observerOutput.open( outputFileName );
    
    unsigned int number_of_slices = stack->originalImages.size();
    
    for(unsigned int slice_number=0; slice_number < number_of_slices; slice_number++) {
  	  registration2D->SetFixedImage( mriVolume->GetResampledSlice(slice_number) );
  		registration2D->SetMovingImage( stack->originalImages[slice_number] );
	    
  	  // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
  	  registration2D->SetFixedImageRegion( mriVolume->GetResampledSlice(slice_number)->GetLargestPossibleRegion() );
  	  // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
	    
  		metric2D->SetFixedImageMask( mriVolume->GetMask2D(slice_number) );
  		metric2D->SetMovingImageMask( stack->GetMask2D(slice_number) );
  		
  		registration2D->SetTransform( stack->GetTransform(slice_number) );
  		registration2D->SetInitialTransformParameters( stack->GetTransform(slice_number)->GetParameters() );
  		
  	  try
  	    {
  	    registration2D->StartRegistration();
  	    cout << "Optimizer stop condition: "
  	         << registration2D->GetOptimizer()->GetStopConditionDescription() << endl << endl;
  	    }
  	  catch( itk::ExceptionObject & err )
  	    {
  	    std::cerr << "ExceptionObject caught !" << std::endl;
  	    std::cerr << err << std::endl;
  	    exit(EXIT_FAILURE);
  	    }
    }
    
	  observerOutput.close();
	}
	
protected:
};
#endif

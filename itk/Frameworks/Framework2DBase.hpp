#ifndef FRAMEWORK2DBASE_HPP_
#define FRAMEWORK2DBASE_HPP_

// YAML config reader
#include "yaml.h"

// ITK includes
#include "itkImageRegistrationMethod.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
// #include "itkImageMaskSpatialObject.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"


class Framework2DBase {
public:
  // typedef itk::CenteredRigid2DTransform< double > TransformType2D;
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType2D;
	typedef itk::MattesMutualInformationImageToImageMetric< MRI::SliceType, Stack::SliceType > MetricType2D;
  typedef itk::LinearInterpolateImageFunction< Stack::SliceType, double > LinearInterpolatorType2D;
	typedef itk::ImageRegistrationMethod< MRI::SliceType, Stack::SliceType > RegistrationType2D;
	typedef itk::ImageMaskSpatialObject< 2 > MaskType2D;
	typedef StdOutIterationUpdate< OptimizerType2D > StdOutObserverType2D;
	typedef FileIterationUpdate< OptimizerType2D > FileObserverType2D;
	
	RegistrationType2D::Pointer registration2D;
	MetricType2D::Pointer metric2D;
	OptimizerType2D::Pointer optimizer2D;
	LinearInterpolatorType2D::Pointer interpolator2D;
	StdOutObserverType2D::Pointer stdOutObserver2D;
	FileObserverType2D::Pointer fileObserver2D;
	ofstream observerOutput;
	YAML::Node& registrationParameters;
  
  Framework2DBase(YAML::Node& parameters):
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

		OptimizerType2D::ScalesType optimizerScales2D( 5 );
    
	  optimizerScales2D[0] = 1.0;
	  optimizerScales2D[1] = translationScale;
	  optimizerScales2D[2] = translationScale;
	  optimizerScales2D[3] = translationScale;
	  optimizerScales2D[4] = translationScale;
    
	  optimizer2D->SetScales( optimizerScales2D );
	}
	
	// explicitly declare virtual destructor,
  // so that base pointers to derived classes will be destroyed fully
  virtual ~Framework2DBase()=0;
  
private:
  // Copy constructor and copy assignment operator deliberately not implemented
  // Made private so that nobody can use them
  Framework2DBase(const Framework2DBase&);
  Framework2DBase& operator=(const Framework2DBase&);
	
};

Framework2DBase::~Framework2DBase() {}

#endif
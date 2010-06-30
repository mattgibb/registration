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
  // typedef itk::CenteredRigid2DTransform< double > TransformType;
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
	typedef itk::MattesMutualInformationImageToImageMetric< MRI::SliceType, Stack::SliceType > MetricType;
  typedef itk::LinearInterpolateImageFunction< Stack::SliceType, double > LinearInterpolatorType;
	typedef itk::ImageRegistrationMethod< MRI::SliceType, Stack::SliceType > RegistrationType;
	typedef itk::ImageMaskSpatialObject< 2 > MaskType;
	typedef StdOutIterationUpdate< OptimizerType > StdOutObserverType;
	typedef FileIterationUpdate< OptimizerType > FileObserverType;
	
	RegistrationType::Pointer registration;
	MetricType::Pointer metric;
	OptimizerType::Pointer optimizer;
	LinearInterpolatorType::Pointer interpolator;
	StdOutObserverType::Pointer stdOutObserver;
	FileObserverType::Pointer fileObserver;
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
		registration = RegistrationType::New();
		metric = MetricType::New();
	  optimizer = OptimizerType::New();
	  interpolator = LinearInterpolatorType::New();
	}
  
  void wireUpRegistrationComponents() {
		registration->SetMetric( metric );
	  registration->SetOptimizer( optimizer );
	  registration->SetInterpolator( interpolator );
	}
  
  void setUpObservers() {
    // Create the command observers
		stdOutObserver = StdOutObserverType::New();
		fileObserver   = FileObserverType::New();
		
		// register the observers
	  optimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
	  optimizer->AddObserver( itk::IterationEvent(), fileObserver );
	
	  // add output to fileObserver
		fileObserver->SetOfstream( &observerOutput );
		    
    // set parameters from config file
    double maxStepLength, minStepLength, maxIterations;
    registrationParameters["maxStepLength2D"] >> maxStepLength;
    registrationParameters["minStepLength2D"] >> minStepLength;
    registrationParameters["maxIterations2D"] >> maxIterations;
    optimizer->SetMaximumStepLength( maxStepLength );
    optimizer->SetMinimumStepLength( minStepLength );
    optimizer->SetNumberOfIterations( maxIterations );
	}
	
	void setOptimizerTranslationScale() {
	  double translationScale;
    registrationParameters["optimizerTranslationScale2D"] >> translationScale;

		OptimizerType::ScalesType optimizerScales( 5 );
    
	  optimizerScales[0] = 1.0;
	  optimizerScales[1] = translationScale;
	  optimizerScales[2] = translationScale;
	  optimizerScales[3] = translationScale;
	  optimizerScales[4] = translationScale;
    
	  optimizer->SetScales( optimizerScales );
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
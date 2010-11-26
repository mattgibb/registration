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
  
  explicit Framework2DBase(YAML::Node& parameters):
  registrationParameters(parameters) {
    initializeRegistrationComponents();
    wireUpRegistrationComponents();
    configureRegistrationComponents();
		setUpObservers();
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
  
  void configureRegistrationComponents() {
    // declare local variables
    unsigned int maxIterations, numberOfSpatialSamples, numberOfHistogramBins;
    double maxStepLength, minStepLength, relaxationFactor;
    bool maximizeOn;
    
    // extract parameter values
    registrationParameters["maxIterations"]  >> maxIterations;
    registrationParameters["maxStepLength"]  >> maxStepLength;
    registrationParameters["minStepLength"]  >> minStepLength;
    registrationParameters["relaxationFactor"]  >> relaxationFactor;
    registrationParameters["numberOfSpatialSamples"]  >> numberOfSpatialSamples;
    registrationParameters["numberOfHistogramBins"]  >> numberOfHistogramBins;
    registrationParameters["maximizeOn"]  >> maximizeOn;
    
    // set parameters
    optimizer->SetNumberOfIterations( maxIterations );
    optimizer->SetMaximumStepLength( maxStepLength );
    optimizer->SetMinimumStepLength( minStepLength );
    optimizer->SetRelaxationFactor( relaxationFactor );
    cout << "optimizer->GetRelaxationFactor(): " << optimizer->GetRelaxationFactor() << endl;
    if(maximizeOn)
    {
      cout << "Optimizer set to maximize." << endl;
      optimizer->MaximizeOn();
    }
    else
    {
      cout << "Optimizer set to minimize." << endl;
    }
		metric->SetNumberOfSpatialSamples( numberOfSpatialSamples );
		metric->SetNumberOfHistogramBins( numberOfHistogramBins );
		
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
		
	}
	
	OptimizerType::Pointer GetOptimizer() {
    return optimizer;
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
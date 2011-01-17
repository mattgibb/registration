#ifndef FRAMEWORK2DBASE_CXX_
#define FRAMEWORK2DBASE_CXX_

#include "Framework2DBase.hpp"
#include "RegistrationParameters.hpp"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkGradientDescentOptimizer.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "NormalizedDifferenceIterationUpdate.hpp"


Framework2DBase::Framework2DBase()
{
  buildRegistrationComponents();
  wireUpRegistrationComponents();
	setUpObservers();
}

void Framework2DBase::buildRegistrationComponents() {
	registration = RegistrationType::New();
  buildMetric();
  buildOptimizer();
  interpolator = LinearInterpolatorType::New();
}

void Framework2DBase::buildMetric() {
  const YAML::Node& metricParameters = registrationParameters()["metric"];
  
  // ensure metric will be built
  if(
    !metricParameters.FindValue("meanSquares") &&
    !metricParameters.FindValue("mattesMutualInformation")
    )
  {
    cerr << "No metric specified in params file!" << endl;
    exit(EXIT_FAILURE);
  }
  
  // pick metric
  if(metricParameters.FindValue("meanSquares")) {
    cout << "Using mean squares image metric.\n";
    typedef itk::MeanSquaresImageToImageMetric< SliceType, SliceType > MetricType;
    MetricType::Pointer specificMetric = MetricType::New();
    
    metric = specificMetric;
  }
  
  if(metricParameters.FindValue("mattesMutualInformation")) {
    cout << "Using Mattes mutual information image metric.\n";
    typedef itk::MattesMutualInformationImageToImageMetric< SliceType, SliceType > MetricType;
    MetricType::Pointer specificMetric = MetricType::New();
    
    // specific settings
    unsigned int numberOfSpatialSamples, numberOfHistogramBins;
    
    metricParameters["mattesMutualInformation"]["numberOfSpatialSamples"]  >> numberOfSpatialSamples;
    metricParameters["mattesMutualInformation"]["numberOfHistogramBins"]  >> numberOfHistogramBins;
    
		specificMetric->SetNumberOfSpatialSamples( numberOfSpatialSamples );
		specificMetric->SetNumberOfHistogramBins( numberOfHistogramBins );
    
    metric = specificMetric;
  }
}

void Framework2DBase::buildOptimizer() {
  const YAML::Node& optimizerParameters = registrationParameters()["optimizer"];
  
  // ensure optimizer will be built
  if(
    !optimizerParameters.FindValue("gradientDescent") &&
    !optimizerParameters.FindValue("regularStepGradientDescent")
    )
  {
    cerr << "No optimizer specified in params file!";
    exit(EXIT_FAILURE);
  }
  
  // extract options
  unsigned int maxIterations;
  
  optimizerParameters["maxIterations"]  >> maxIterations;
  
  // pick optimizer and set specific options
  if(optimizerParameters.FindValue("gradientDescent")) {
    cout << "Using gradient descent optimizer.\n";
    typedef itk::GradientDescentOptimizer OptimizerType;
    OptimizerType::Pointer specificOptimizer = OptimizerType::New();
    
    double learningRate;
    optimizerParameters["gradientDescent"] >> learningRate;
    specificOptimizer->SetLearningRate( learningRate );
    specificOptimizer->SetNumberOfIterations( maxIterations );

    optimizer = specificOptimizer;
  }
  
  if(optimizerParameters.FindValue("regularStepGradientDescent")) {
    cout << "Using regular step gradient descent optimizer.\n";
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    OptimizerType::Pointer specificOptimizer = OptimizerType::New();
    
    double relaxationFactor, maxStepLength, minStepLength;
    optimizerParameters["regularStepGradientDescent"]["relaxationFactor"]  >> relaxationFactor;
    optimizerParameters["regularStepGradientDescent"]["maxStepLength"]  >> maxStepLength;
    optimizerParameters["regularStepGradientDescent"]["minStepLength"]  >> minStepLength;
    specificOptimizer->SetRelaxationFactor( relaxationFactor );
    specificOptimizer->SetNumberOfIterations( maxIterations );
    specificOptimizer->SetMaximumStepLength( maxStepLength );
    specificOptimizer->SetMinimumStepLength( minStepLength );
    
    optimizer = specificOptimizer;
  }
}

void Framework2DBase::wireUpRegistrationComponents() {
	registration->SetMetric( metric );
  registration->SetOptimizer( optimizer );
  registration->SetInterpolator( interpolator );
}

// implementation of Framework2DBase::SetUpObservers()
template< typename OptimizerType >
void doSetUpObservers(itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
{
  typedef StdOutIterationUpdate< OptimizerType > StdOutObserverType;
  typedef NormalizedDifferenceIterationUpdate< OptimizerType > NormalizedDifferenceObserverType;
	typedef FileIterationUpdate< OptimizerType > FileObserverType;
  
  // instantiate observers
  typename StdOutObserverType::Pointer stdOutObserver = StdOutObserverType::New();
  typename NormalizedDifferenceObserverType::Pointer normalizedDifferenceObserver = NormalizedDifferenceObserverType::New();
	typename FileObserverType::Pointer fileObserver = FileObserverType::New();
  
	// register the observers
  optimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
  // optimizer->AddObserver( itk::IterationEvent(), normalizedDifferenceObserver );
  optimizer->AddObserver( itk::IterationEvent(), fileObserver );
  
}

void Framework2DBase::setUpObservers() {
  const YAML::Node& optimizerParameters = registrationParameters()["optimizer"];
  
  // declare observer types
  if(optimizerParameters.FindValue("gradientDescent"))
  {
    doSetUpObservers< itk::GradientDescentOptimizer >(optimizer);
  }
  if(optimizerParameters.FindValue("regularStepGradientDescent"))
  {
    doSetUpObservers< itk::RegularStepGradientDescentOptimizer >(optimizer);
  }
}

Framework2DBase::~Framework2DBase() {}

#endif
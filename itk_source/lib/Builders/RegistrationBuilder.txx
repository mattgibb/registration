#ifndef REGISTRATIONBUILDER_CXX_
#define REGISTRATIONBUILDER_CXX_

// metrics
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

#include "itkNormalizedCorrelationImageToImageMetric.h"
// optimisers
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkGradientDescentOptimizer.h"
// interpolators
#include "itkLinearInterpolateImageFunction.h"

// my files
#include "RegistrationBuilder.hpp"
#include "Parameters.hpp"
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "NormalizedDifferenceIterationUpdate.hpp"


template <typename StackType>
RegistrationBuilder< StackType >::RegistrationBuilder():
  m_registrationParameters( registrationParameters() )
{
  buildRegistrationComponents();
	setUpObservers();
}

template <typename StackType>
RegistrationBuilder< StackType >::RegistrationBuilder(YAML::Node& parameters):
  m_registrationParameters( parameters )
{
  buildRegistrationComponents();
	setUpObservers();
}

template <typename StackType>
void RegistrationBuilder< StackType >::buildRegistrationComponents() {
  buildRegistration();
  buildMetric();
  buildOptimizer();
  buildInterpolator();
}

template <typename StackType>
void RegistrationBuilder< StackType >::buildRegistration() {
  m_registration = RegistrationType::New();
}

template <typename StackType>
void RegistrationBuilder< StackType >::buildMetric() {
  const YAML::Node& metricParameters = m_registrationParameters["metric"];
  
  // ensure metric will be built
  if(
    !metricParameters.FindValue("meanSquares") &&
    !metricParameters.FindValue("mattesMutualInformation") &&
    !metricParameters.FindValue("normalizedCorrelation")
    )
  {
    cerr << "No metric specified in params file!" << endl;
    exit(EXIT_FAILURE);
  }
  
  // pick metric
  if(metricParameters.FindValue("meanSquares")) {
    cout << "Using mean squares image metric.\n";
    typedef itk::MeanSquaresImageToImageMetric< typename StackType::SliceType, typename StackType::SliceType > MetricType;
    typename MetricType::Pointer metric = MetricType::New();
    
    m_registration->SetMetric( metric );
  }
  
  if(metricParameters.FindValue("normalizedCorrelation")) {
    cout << "Using normalized correlation image metric.\n";
    typedef itk::NormalizedCorrelationImageToImageMetric< typename StackType::SliceType, typename StackType::SliceType > MetricType;
    typename MetricType::Pointer metric = MetricType::New();
    
    m_registration->SetMetric( metric );
  }
  
  if(metricParameters.FindValue("mattesMutualInformation")) {
    cout << "Using Mattes mutual information image metric.\n";
    typedef itk::MattesMutualInformationImageToImageMetric< typename StackType::SliceType, typename StackType::SliceType > MetricType;
    typename MetricType::Pointer metric = MetricType::New();
    
    // specific settings
    unsigned int numberOfSpatialSamples, numberOfHistogramBins;
    
    metricParameters["mattesMutualInformation"]["numberOfSpatialSamples"]  >> numberOfSpatialSamples;
    metricParameters["mattesMutualInformation"]["numberOfHistogramBins"]  >> numberOfHistogramBins;
    
		metric->SetNumberOfSpatialSamples( numberOfSpatialSamples );
		metric->SetNumberOfHistogramBins( numberOfHistogramBins );
    
    m_registration->SetMetric( metric );
  }
}

template <typename StackType>
void RegistrationBuilder< StackType >::buildOptimizer() {
  const YAML::Node& optimizerParameters = m_registrationParameters["optimizer"];
  
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
  bool maximize;
  
  optimizerParameters["maxIterations"]  >> maxIterations;
  optimizerParameters["maximize"] >> maximize;
  
  // pick optimizer and set specific options
  if(optimizerParameters.FindValue("gradientDescent")) {
    cout << "Using gradient descent optimizer.\n";
    typedef itk::GradientDescentOptimizer OptimizerType;
    OptimizerType::Pointer specificOptimizer = OptimizerType::New();
    
    double learningRate;
    optimizerParameters["gradientDescent"] >> learningRate;
    specificOptimizer->SetLearningRate( learningRate );
    specificOptimizer->SetNumberOfIterations( maxIterations );
    
    specificOptimizer->SetMaximize(maximize);
    
    m_registration->SetOptimizer( specificOptimizer );
  }
  
  if(optimizerParameters.FindValue("regularStepGradientDescent")) {
    cout << "Using regular step gradient descent optimizer.\n";
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    OptimizerType::Pointer specificOptimizer = OptimizerType::New();
    
    double relaxationFactor, maxStepLength, minStepLength, gradientMagnitudeTolerance;
    optimizerParameters["regularStepGradientDescent"]["relaxationFactor"]  >> relaxationFactor;
    optimizerParameters["regularStepGradientDescent"]["maxStepLength"]  >> maxStepLength;
    optimizerParameters["regularStepGradientDescent"]["minStepLength"]  >> minStepLength;
    optimizerParameters["regularStepGradientDescent"]["gradientMagnitudeTolerance"]  >> gradientMagnitudeTolerance;
    specificOptimizer->SetRelaxationFactor( relaxationFactor );
    specificOptimizer->SetNumberOfIterations( maxIterations );
    specificOptimizer->SetMaximumStepLength( maxStepLength );
    specificOptimizer->SetMinimumStepLength( minStepLength );
    specificOptimizer->SetGradientMagnitudeTolerance( gradientMagnitudeTolerance );
    
    specificOptimizer->SetMaximize(maximize);
    
    m_registration->SetOptimizer( specificOptimizer );
  }
    
}

template <typename StackType>
void RegistrationBuilder< StackType >::buildInterpolator() {
  typedef itk::LinearInterpolateImageFunction< typename StackType::SliceType, double > LinearInterpolatorType;
  m_registration->SetInterpolator( LinearInterpolatorType::New() );
}

// implementation of RegistrationBuilder< StackType >::SetUpObservers()
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

template <typename StackType>
void RegistrationBuilder< StackType >::setUpObservers() {
  const YAML::Node& optimizerParameters = m_registrationParameters["optimizer"];
  
  // declare observer types
  if(optimizerParameters.FindValue("gradientDescent"))
  {
    doSetUpObservers< itk::GradientDescentOptimizer >( m_registration->GetOptimizer() );
  }
  if(optimizerParameters.FindValue("regularStepGradientDescent"))
  {
    doSetUpObservers< itk::RegularStepGradientDescentOptimizer >( m_registration->GetOptimizer() );
  }
}

template <typename StackType>
typename RegistrationBuilder< StackType >::RegistrationType::Pointer RegistrationBuilder< StackType >::GetRegistration() {
  // Sanity checks
  if( ! this->m_registration )
  {
    itkExceptionMacro( "Registration has not been initialised, check params file." );
  }

  if( ! this->m_registration->GetMetric() )
  {
    itkExceptionMacro( "Metric has not been initialised, check params file" );
  }

  if( ! this->m_registration->GetOptimizer() )
  {
    itkExceptionMacro( "Optimiser has not been initialised, check params file" );
  }

  if( ! this->m_registration->GetInterpolator() )
  {
    itkExceptionMacro( "Interpolator has not been initialised, check params file" );
  }
  
  return m_registration;
}

// RegistrationBuilder< StackType >::~RegistrationBuilder() {}

#endif

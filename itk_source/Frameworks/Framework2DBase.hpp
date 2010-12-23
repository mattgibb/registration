#ifndef FRAMEWORK2DBASE_HPP_
#define FRAMEWORK2DBASE_HPP_

// YAML config reader
#include "yaml.h"

// ITK includes
#include "itkImageRegistrationMethod.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkGradientDescentOptimizer.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
// #include "itkImageMaskSpatialObject.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "Stack.hpp"

// implementation of Framework2DBase::SetUpObservers()
template< typename OptimizerType >
void doSetUpObservers(itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
{
  typedef StdOutIterationUpdate< OptimizerType > StdOutObserverType;
	typedef FileIterationUpdate< OptimizerType > FileObserverType;
  
  // instantiate observers
  typename StdOutObserverType::Pointer stdOutObserver = StdOutObserverType::New();
	typename FileObserverType::Pointer fileObserver = FileObserverType::New();
  
	// register the observers
  optimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
  optimizer->AddObserver( itk::IterationEvent(), fileObserver );
  
}

class Framework2DBase {
public:
  typedef short PixelType;
  typedef itk::Image< PixelType, 2 > SliceType;
  typedef itk::LinearInterpolateImageFunction< SliceType, double > LinearInterpolatorType;
	typedef itk::ImageRegistrationMethod< SliceType, SliceType > RegistrationType;
	typedef itk::ImageMaskSpatialObject< 2 > MaskType;
	
	RegistrationType::Pointer registration;
	itk::ImageToImageMetric< SliceType, SliceType >::Pointer metric;
	itk::SingleValuedNonLinearOptimizer::Pointer optimizer;
	LinearInterpolatorType::Pointer interpolator;
	YAML::Node& registrationParameters;
  
  explicit Framework2DBase(YAML::Node& parameters);
  
  void buildRegistrationComponents();
  
  void buildMetric();
  
  void buildOptimizer();
  
  void wireUpRegistrationComponents();
  
  void setUpObservers() {
    const YAML::Node& optimizerParameters = registrationParameters["optimizer"];
    
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
	
	itk::SingleValuedNonLinearOptimizer::Pointer GetOptimizer() {
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

#endif
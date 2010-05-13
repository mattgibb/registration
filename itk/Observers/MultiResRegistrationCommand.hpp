#ifndef MULTIRESREGISTRATIONCOMMAND_HPP_
#define MULTIRESREGISTRATIONCOMMAND_HPP_

#include "itkCommand.h"

using namespace std;

template <typename RegistrationType, typename OptimizerType, typename MetricType>
class MultiResRegistrationCommand : public itk::Command
{
protected:
  MultiResRegistrationCommand() {};
  itk::Array<unsigned int> maxIterations, spatialSamples;
  itk::Array<double> maxStepLengths, minStepLengths;
  unsigned int histogramBins;

public:
  typedef  MultiResRegistrationCommand      Self;
  typedef  itk::Command                     Superclass;
  typedef  itk::SmartPointer<Self>          Pointer;
  typedef  RegistrationType *               RegistrationPointer;
  typedef  OptimizerType *                  OptimizerPointer;
	typedef  MetricType *                     MetricPointer;

  itkNewMacro( Self );

  // Two arguments are passed to the Execute() method: the first
  // is the pointer to the object which invoked the event and the
  // second is the event that was invoked.
  void Execute(itk::Object * object, const itk::EventObject & event) {
    if( !(itk::IterationEvent().CheckEvent( &event )) )
      { return; }
    
    RegistrationPointer registration = dynamic_cast<RegistrationPointer>( object );
    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( registration->GetOptimizer() );
		MetricPointer metric = dynamic_cast< MetricPointer >( registration->GetMetric() );

    unsigned long level = registration->GetCurrentLevel();
    optimizer->SetNumberOfIterations( maxIterations[level] );
    optimizer->SetMaximumStepLength( maxStepLengths[level] );
    optimizer->SetMinimumStepLength( minStepLengths[level] );
		metric->SetNumberOfSpatialSamples( spatialSamples[level] );
    
    if ( level == 0 ) {
			metric->SetNumberOfHistogramBins( histogramBins );
    }
    else {
	    cout << "Optimizer stop condition: "
			   	 << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
    }
    
    cout << "TESTING This should be 50: " << metric->GetNumberOfHistogramBins() << endl;
    cout << "-------------------------------------" << endl;
    cout << "MultiResolution Level : " << registration->GetCurrentLevel()  << endl;
		cout << "Max step length : " << optimizer->GetMaximumStepLength() << endl;
		cout << "Min step length : " << optimizer->GetMinimumStepLength() << endl;
		cout << "Spatial samples : " << metric->GetNumberOfSpatialSamples() << endl << endl;
  }

  // Another version of the Execute() method accepting a const
  // input object is also required since this method is defined as pure virtual
  // in the base class.  This version simply returns without taking any action.
  void Execute(const itk::Object * , const itk::EventObject & ) {
		return;
	}
	  
  void configure(YAML::Node& parameters) {
    // initialise arrays
    maxIterations = itk::Array< unsigned int >(4);
    spatialSamples = itk::Array< unsigned int >(4);
    maxStepLengths = itk::Array< double >(4);
    minStepLengths = itk::Array< double >(4);
    
    // assign values
    parameters["histogramBins3D"] >> histogramBins;
    
    
    for(int i=0; i<4; i++) {
      parameters["maxIterations3D"][i]  >> maxIterations[i];
      parameters["spatialSamples3D"][i] >> spatialSamples[i];
      parameters["maxStepLengths3D"][i] >> maxStepLengths[i];
      parameters["minStepLengths3D"][i] >> minStepLengths[i];
    }
  }
  
};
#endif

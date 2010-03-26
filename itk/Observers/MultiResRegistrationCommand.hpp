#ifndef MULTIRESREGISTRATIONCOMMAND_HPP_
#define MULTIRESREGISTRATIONCOMMAND_HPP_

#include "itkCommand.h"

using namespace std;

template <typename RegistrationType, typename OptimizerType, typename MetricType>
class MultiResRegistrationCommand : public itk::Command
{
protected:
  MultiResRegistrationCommand() {};

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
  void Execute(itk::Object * object, const itk::EventObject & event)
    {
    if( !(itk::IterationEvent().CheckEvent( &event )) )
      { return; }

    RegistrationPointer registration = dynamic_cast<RegistrationPointer>( object );
    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( registration->GetOptimizer() );
		MetricPointer metric = dynamic_cast< MetricPointer >( registration->GetMetric() );


    if ( registration->GetCurrentLevel() == 0 )
      {
	      optimizer->SetMaximumStepLength( 0.25 );
	      optimizer->SetMinimumStepLength( 0.00125 );
			  optimizer->SetNumberOfIterations( 100 );
				metric->SetNumberOfSpatialSamples( 24000 );
				// Number of bins recommended to be about 50, see ITK Software Guide p341
				metric->SetNumberOfHistogramBins( 50 );
      }
    else
      {
		    cout << "Optimizer stop condition: "
				   	 << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
	      optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() / 2.0 );
				// optimizer->SetMaximumStepLength( optimizer->GetCurrentStepLength() );
	      optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() / 2.0 );
				// optimizer->SetNumberOfIterations( 1 );
				// metric->SetNumberOfSpatialSamples( metric->GetNumberOfSpatialSamples() * 4 );
				// metric->SetNumberOfSpatialSamples( metric->GetNumberOfSpatialSamples() * 8 );
      }

    cout << "-------------------------------------" << endl;
    cout << "MultiResolution Level : " << registration->GetCurrentLevel()  << endl;
		cout << "Max step length : " << optimizer->GetMaximumStepLength() << endl;
		cout << "Min step length : " << optimizer->GetMinimumStepLength() << endl;
		cout << "Spatial samples : " << metric->GetNumberOfSpatialSamples() << endl << endl;
  }

  // Another version of the Execute() method accepting a const
  // input object is also required since this method is defined as pure virtual
  // in the base class.  This version simply returns without taking any action.
  void Execute(const itk::Object * , const itk::EventObject & )
    {
			return;
		}
};
#endif

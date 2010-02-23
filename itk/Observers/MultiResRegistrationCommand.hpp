#ifndef MULTIRESREGISTRATIONCOMMAND_HPP_
#define MULTIRESREGISTRATIONCOMMAND_HPP_

#include "itkCommand.h"

using namespace std;

template <typename RegistrationType, typename OptimizerType>
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

    std::cout << "-------------------------------------" << std::endl;
    std::cout << "MultiResolution Level : "
              << registration->GetCurrentLevel()  << std::endl;
    std::cout << std::endl;

    if ( registration->GetCurrentLevel() == 0 )
      {
      optimizer->SetMaximumStepLength( 16.00 );
      optimizer->SetMinimumStepLength( 0.01 ); // from example in source code
      // optimizer->SetMinimumStepLength( 2.5 ); // from ItkSoftwareGuide
      }
    else
      {
      // optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() / 4.0 ); // from example in source code
			optimizer->SetMaximumStepLength( optimizer->GetCurrentStepLength() ); // from ItkSoftwareGuide
      optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() / 10.0 );
      }
  }

  // Another version of the \code{Execute()} method accepting a \code{const}
  // input object is also required since this method is defined as pure virtual
  // in the base class.  This version simply returns without taking any action.
  void Execute(const itk::Object * , const itk::EventObject & )
    {
			return;
		}
};
#endif

#ifndef STDOUTITERATIONUPDATE_HPP_
#define STDOUTITERATIONUPDATE_HPP_

#include "CommandObserverBase.hpp"

using namespace std;

template<typename OptimizerType>
class StdOutIterationUpdate : public CommandObserverBase
{
public:
  typedef StdOutIterationUpdate                Self;
  typedef CommandObserverBase                  Superclass;
  typedef itk::SmartPointer<Self>              Pointer;
  typedef const OptimizerType*                 OptimizerPointer;

  itkNewMacro( Self );

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( object );
		
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
			
    cout << optimizer->GetCurrentIteration() << " = ";
    cout << optimizer->GetValue() << " : ";
    cout << optimizer->GetCurrentPosition() << endl;
  }
};
#endif

#ifndef COMMANDITERATIONUPDATE_CPP_
#define COMMANDITERATIONUPDATE_CPP_

#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkCommand.h"

// Class to handle blah blah blah

using namespace std;

class CommandIterationUpdate : public itk::Command
{
public:
  typedef CommandIterationUpdate   Self;
  typedef itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;

  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {}

public:
  typedef itk::RegularStepGradientDescentOptimizer     OptimizerType;
  typedef const OptimizerType *                        OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
	  // in this case, just calls the const version of Execute
    Execute( (const itk::Object *)caller, event);
    }

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
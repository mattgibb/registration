#ifndef COMMANDOBSERVERBASE_HPP_
#define COMMANDOBSERVERBASE_HPP_

#include "itkCommand.h"

using namespace std;

class CommandObserverBase : public itk::Command
{
protected:
  CommandObserverBase() {}

public:
  typedef CommandObserverBase      Self;
  typedef itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
	
  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
	  // in this case, just calls the const version of Execute
    this->Execute( (const itk::Object *)caller, event);
  }

	virtual void Execute(const itk::Object *caller, const itk::EventObject & event) = 0;

};
#endif

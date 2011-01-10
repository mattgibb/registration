#ifndef COMMANDOBSERVERBASE_HPP_
#define COMMANDOBSERVERBASE_HPP_

#include "itkCommand.h"

using namespace std;

class CommandObserverBase : public itk::Command
{
public:
  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
	  // in this case, just calls the const version of Execute
    this->Execute( (const itk::Object *)caller, event);
  }
  
	virtual void Execute(const itk::Object *caller, const itk::EventObject & event) = 0;
  
  CommandObserverBase() {}
  
	// explicitly declare virtual destructor,
  // so that base pointers to derived classes will be destroyed fully
  // Made pure virtual to make class abstract
  virtual ~CommandObserverBase()=0;
  
private:
  // Copy constructor and copy assignment operator Made private
  // so that no subclasses or clients can use them,
  // deliberately not implemented so not even class methods can use them
  CommandObserverBase(const CommandObserverBase&);
  CommandObserverBase& operator=(const CommandObserverBase&);
  
};

inline CommandObserverBase::~CommandObserverBase() {}
#endif


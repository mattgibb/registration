#ifndef COMMANDOBSERVERBASE_HPP_
#define COMMANDOBSERVERBASE_HPP_

#include "itkCommand.h"
#include "itkArray.h"
#include "itkGradientDescentOptimizer.h"
#include "itkRegularStepGradientDescentOptimizer.h"

using namespace std;

class CommandObserverBase : public itk::Command
{
public:
  typedef itk::Array< double > ParamsType;
  
  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
	  // in this case, just calls the const version of Execute
    this->Execute( (const itk::Object *)caller, event);
  }
  
  // extracts info from optimiser into instance variables
  // and calls pure virtual implementation for subclass
	void Execute(const itk::Object *caller, const itk::EventObject & event)
	{
	  // Check if it is the right type of event
	  if( ! itk::IterationEvent().CheckEvent( &event ) )
    { return; }
    
    // extract values from optimizer
    typedef itk::GradientDescentOptimizer GD;
    typedef itk::RegularStepGradientDescentOptimizer RSGD;
    
    // optimizer base class does not have GetCurrentIteration method,
    // so runtime check to extract current iteration
    if(const GD* gd = dynamic_cast< const GD* >( caller ))
      extractVariablesFromOptimizer< GD >( gd );
    else if(const RSGD * rsgd = dynamic_cast< const RSGD* >( caller ))
      extractVariablesFromOptimizer< RSGD >( rsgd );
    else
    {
      cerr << "No optimizer found." << endl;
      exit(EXIT_FAILURE);
    }
    
    // run the derived class virtual method run()
    run();
	}
  
  // Virtual method to override in base classes
  // Specifics of what the derived class does
  virtual void run() = 0;
  
	// explicitly declare virtual destructor,
  // so that base pointers to derived classes will be destroyed fully
  // Made pure virtual to make class abstract
  virtual ~CommandObserverBase()=0;
  
protected:
  // template method is necessary because there is no interface
  // to extract values in a common Optimizer base class in ITKv3
  // therefore must use implicit template-based interfaces
  template<typename OptimizerType>
  void extractVariablesFromOptimizer(const OptimizerType* optimizer)
  {
    m_iteration = optimizer->GetCurrentIteration();
    m_value     = optimizer->GetValue();
    m_position  = optimizer->GetCurrentPosition();
    m_scales    = optimizer->GetScales();
  }
  
  // extracted optimiser variables
  unsigned long m_iteration;
  double m_value;
  ParamsType m_position;
  ParamsType m_scales;
  
  // constructor
  CommandObserverBase():m_iteration(0), m_value(0.0) {}
  
private:
  // Copy constructor and copy assignment operator Made private
  // so that no subclasses or clients can use them,
  // deliberately not implemented so not even class methods can use them
  CommandObserverBase(const CommandObserverBase&);
  CommandObserverBase& operator=(const CommandObserverBase&);
  
};

inline CommandObserverBase::~CommandObserverBase() {}
#endif


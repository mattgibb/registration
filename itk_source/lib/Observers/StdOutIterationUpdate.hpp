#ifndef STDOUTITERATIONUPDATE_HPP_
#define STDOUTITERATIONUPDATE_HPP_

#include "CommandObserverBase.hpp"

using namespace std;

class StdOutIterationUpdate : public CommandObserverBase
{
public:
  typedef StdOutIterationUpdate                Self;
  typedef CommandObserverBase                  Superclass;
  typedef itk::SmartPointer<Self>              Pointer;

  itkNewMacro( Self );
  
  virtual void run()
  {
    cout << m_iteration << " = ";
    cout << m_value << " : ";
    cout << m_position << endl;
  }
};

#endif

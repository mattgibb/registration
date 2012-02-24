#ifndef NORMALIZEDDIFFERENCEITERATIONUPDATE_HPP_
#define NORMALIZEDDIFFERENCEITERATIONUPDATE_HPP_

#include <math.h>
#include "CommandObserverBase.hpp"

using namespace std;

class NormalizedDifferenceIterationUpdate : public CommandObserverBase
{
public:
  typedef NormalizedDifferenceIterationUpdate  Self;
  typedef CommandObserverBase                  Superclass;
  typedef itk::SmartPointer<Self>              Pointer;

  itkNewMacro( Self );

  virtual void run()
  {
    cout << m_iteration << " = ";
    cout << m_value << " : ";
    
    // unless this is the first event (and thus m_oldPosition hasn't been set)
    // print out the differences between m_oldPosition and position,
    // normalised by the optimiser scales and by the total length of
    // the step
    if(m_oldPosition.Size())
    {
      ParamsType differences( m_position.Size() );
      double length = 0.0;
      
      // scale step lengths and calculate total step length
      for(unsigned int i=0; i<m_position.Size(); ++i)
      {
        differences[i] = ( m_position[i] - m_oldPosition[i] ) * m_scales[i];
        length += differences[i] * differences[i];
      }
      length = sqrt( length );
      
      // normalise step lengths
      for(unsigned int i=0; i<m_position.Size(); ++i)
      {
        differences[i] = differences[i] / length;
      }
      
      cout << differences;
    }
    
    cout << endl;
    
    // save params
    m_oldPosition = m_position;
  }
  
private:
  ParamsType m_oldPosition;
};
#endif

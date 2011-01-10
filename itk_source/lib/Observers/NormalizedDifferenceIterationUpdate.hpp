#ifndef NORMALIZEDDIFFERENCEITERATIONUPDATE_HPP_
#define NORMALIZEDDIFFERENCEITERATIONUPDATE_HPP_

#include <math.h>
#include "CommandObserverBase.hpp"

using namespace std;

template<typename OptimizerType>
class NormalizedDifferenceIterationUpdate : public CommandObserverBase
{
public:
  typedef NormalizedDifferenceIterationUpdate  Self;
  typedef CommandObserverBase                  Superclass;
  typedef itk::SmartPointer<Self>              Pointer;

  itkNewMacro( Self );

  virtual void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    const OptimizerType* optimizer = dynamic_cast< const OptimizerType* >( object );
		
    if( ! itk::IterationEvent().CheckEvent( &event ) )
    {
      return;
    }
		
    cout << optimizer->GetCurrentIteration() << " = ";
    cout << optimizer->GetValue() << " : ";
    
    typename OptimizerType::ParametersType params = optimizer->GetCurrentPosition();
    
    // unless this is the first event, and oldParams hasn't been set,
    // print out the differences between oldParams and params,
    // normalised by the optimiser scales and by the total length of
    // the step
    if(oldParams.Size())
    {
      typename OptimizerType::ParametersType differences( params.Size() );
      typename OptimizerType::ScalesType scales = optimizer->GetScales();
      double length = 0.0;
      
      // scale step lengths and calculate total step length
      for(unsigned int i=0; i<params.GetNumberOfElements(); i++)
      {
        differences[i] = ( params[i] - oldParams[i] ) * scales[i];
        length += differences[i] * differences[i];
      }
      length = sqrt( length );
      
      // normalise step lengths
      for(unsigned int i=0; i<params.GetNumberOfElements(); i++)
      {
        differences[i] = differences[i] / length;
      }
      
      cout << differences;
    
    }
    
    cout << endl;
    
    // save params
    oldParams = params;
  }
  
private:
  typename OptimizerType::ParametersType oldParams;
};
#endif

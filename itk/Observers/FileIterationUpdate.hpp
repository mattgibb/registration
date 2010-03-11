#ifndef FILEITERATIONUPDATE_HPP_
#define FILEITERATIONUPDATE_HPP_

#include "CommandObserverBase.hpp"

using namespace std;

template<typename OptimizerType>
class FileIterationUpdate : public CommandObserverBase< OptimizerType >
{
public:
  typedef FileIterationUpdate                  Self;
  typedef CommandObserverBase< OptimizerType > Superclass;
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
		
		typename OptimizerType::ParametersType params = optimizer->GetCurrentPosition();
		
		// (*output) << registration->GetCurrentLevel() << " ";
    (*output) << optimizer->GetCurrentIteration() << " ";
    (*output) << optimizer->GetValue();
		for(unsigned int i=0; i<params.GetNumberOfElements(); i++)
		{
			(*output) << " " << params[i];
		}
    (*output) << endl;
  }

	void SetOfstream(ofstream *stream)
	{
		output = stream;
	}
	
	ofstream * GetOfstream()
	{
		return output;
	}
	
protected:
	ofstream *output;

};
#endif

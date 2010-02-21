#ifndef FILEITERATIONUPDATE_HPP_
#define FILEITERATIONUPDATE_HPP_

#include "itkCommand.h"

using namespace std;

template<typename OptimizerType>
class FileIterationUpdate : public itk::Command
{
public:
  typedef FileIterationUpdate      Self;
  typedef itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;

  itkNewMacro( Self );

protected:
  FileIterationUpdate() {}
	ofstream *output;

public:
  typedef const OptimizerType* OptimizerPointer;

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
		
		typename OptimizerType::ParametersType params = optimizer->GetCurrentPosition();
		
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
	
};
#endif

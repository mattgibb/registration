#ifndef FILEITERATIONUPDATE_HPP_
#define FILEITERATIONUPDATE_HPP_

#include <fstream>

#include "CommandObserverBase.hpp"

using namespace std;

template<typename OptimizerType>
class FileIterationUpdate : public CommandObserverBase
{
public:
  typedef FileIterationUpdate                  Self;
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
		
		typename OptimizerType::ParametersType params = optimizer->GetCurrentPosition();
		
		// output << registration->GetCurrentLevel() << " ";
    output << optimizer->GetCurrentIteration() << " ";
    output << optimizer->GetValue();
		for(unsigned int i=0; i<params.GetNumberOfElements(); i++)
		{
			output << " " << params[i];
		}
    output << endl;
  }

	void SetFilename(const string& fileName)
	{
    output.close();
		output.open(fileName.c_str());
	}
	
protected:
	ofstream output;
  
};
#endif

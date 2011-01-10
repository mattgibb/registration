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
  
  itkNewMacro( Self );
  
  virtual void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    if( ! itk::IterationEvent().CheckEvent( &event ) )
    {
      return;
    }
		
    const OptimizerType* optimizer = dynamic_cast< const OptimizerType* >( object );
		
		// output << registration->GetCurrentLevel() << " ";
    output << optimizer->GetCurrentIteration() << " ";
    output << optimizer->GetValue();
		
		typename OptimizerType::ParametersType params = optimizer->GetCurrentPosition();
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

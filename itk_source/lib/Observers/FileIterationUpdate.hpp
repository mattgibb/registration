#ifndef FILEITERATIONUPDATE_HPP_
#define FILEITERATIONUPDATE_HPP_

#include <fstream>

#include "CommandObserverBase.hpp"

using namespace std;

class FileIterationUpdate : public CommandObserverBase
{
public:
  typedef FileIterationUpdate     Self;
  typedef CommandObserverBase     Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  
  itkNewMacro( Self );
  
  virtual void run()
  {
    m_output << m_iteration << " ";
    m_output << m_value;
		
		for(unsigned int i=0; i<m_position.GetNumberOfElements(); ++i)
			m_output << " " << m_position[i];
		
    m_output << endl;
  }
  
	void SetFilename(const string& fileName)
	{
    m_output.close();
		m_output.open(fileName.c_str());
	}

private:
	ofstream m_output;
  
};
#endif

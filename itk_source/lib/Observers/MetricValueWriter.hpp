#ifndef METRICVALUEWRITER_HPP_
#define METRICVALUEWRITER_HPP_

#include "WriterCommand.hpp"

#include "StackBase.hpp"

class MetricValueWriter : public WriterCommand
{
public:
  typedef MetricValueWriter        Self;
  typedef WriterCommand            Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  
  itkNewMacro( Self );
  
  virtual void run() { m_output << m_value << endl; }
  
  // sets slice number and resets io
  virtual void setSliceNumber(unsigned int sliceNumber)
  {
    // construct paths
    string dirPath = m_outputRootDir + transformType() + "/";
    create_directories(dirPath);
    string filePath = dirPath + m_stack->GetBasename(sliceNumber);
    
    // open file IO
    m_output.close();
		m_output.open(fileName);
  }
  
private:
  ofstream m_output;
  
};
#endif

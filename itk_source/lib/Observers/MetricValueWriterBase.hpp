#ifndef METRICVALUEWRITERBASE_HPP_
#define METRICVALUEWRITERBASE_HPP_

#include "WriterCommand.hpp"

#include "StackBase.hpp"

class MetricValueWriterBase : public WriterCommand
{
public:
  virtual void run() { m_output << m_value << endl; }
  
  // sets slice number and resets io
  virtual void setSliceNumber(unsigned int sliceNumber)
  {
    m_sliceNumber = sliceNumber;
    
    // construct paths
    create_directories(dirPath());
    string filePath = dirPath() + m_stack->GetBasename(sliceNumber);
    
    // open file IO
    m_output.close();
		m_output.open(filePath.c_str());
  }
  
  virtual string dirPath()=0;
  
private:
  ofstream m_output;
  
};

#endif

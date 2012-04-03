#ifndef TRANSFORMWRITERBASE_HPP_
#define TRANSFORMWRITERBASE_HPP_

#include "WriterCommand.hpp"
#include "IOHelpers.hpp"

class TransformWriterBase : public WriterCommand
{
public:
  virtual void run()
  {
    create_directories(dirPath());
    stringstream filePath;
    filePath << dirPath()
      // leading zeros
      << setfill('0')
      // 4 digits wide
      << setw(4)
      << m_iteration;
    
    // save transform
    writeTransform(m_stack->GetTransform(m_sliceNumber), filePath.str());
  }
  
  virtual void setSliceNumber(unsigned int sliceNumber) { m_sliceNumber = sliceNumber; }
  
  virtual string dirPath()=0;
  
};

#endif

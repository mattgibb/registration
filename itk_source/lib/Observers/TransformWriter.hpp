#ifndef TRANSFORMWRITER_HPP_
#define TRANSFORMWRITER_HPP_

#include "WriterCommand.hpp"
#include "IOHelpers.hpp"

class TransformWriter : public WriterCommand
{
public:
  typedef TransformWriter            Self;
  typedef WriterCommand              Superclass;
  typedef itk::SmartPointer<Self>    Pointer;
  
  itkNewMacro( Self );
  
  virtual void run()
  {
    // construct paths
    string dirPath = m_outputRootDir +
                     transformType() + "/" +
                     m_stack->GetBasename(m_sliceNumber) + "/";
                     
    create_directories(dirPath);
    stringstream filePath;
    filePath << dirPath
      // leading zeros
      << setfill('0')
      // 4 digits wide
      << setw(4)
      << m_iteration;
    
    // save transform
    writeTransform(m_stack->GetTransform(m_sliceNumber), filePath.str());
  }
  
  virtual void setSliceNumber(unsigned int sliceNumber) { m_sliceNumber = sliceNumber; }
  
};

#endif

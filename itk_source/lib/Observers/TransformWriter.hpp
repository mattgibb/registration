#ifndef TRANSFORMWRITER_HPP_
#define TRANSFORMWRITER_HPP_

// This transform writer saves transforms in a directory
// based on the transform type.

#include "TransformWriterBase.hpp"

class TransformWriter : public TransformWriterBase
{
public:
  typedef TransformWriter            Self;
  typedef TransformWriterBase        Superclass;
  typedef itk::SmartPointer<Self>    Pointer;
  
  itkNewMacro( Self );
  
  virtual string dirPath()
  {
    return m_outputRootDir +
           transformType() + "/" +
           m_stack->GetBasename(m_sliceNumber) + "/";
  }
};

#endif

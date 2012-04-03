#ifndef SIMPLETRANSFORMWRITER_HPP_
#define SIMPLETRANSFORMWRITER_HPP_

// This transform writer doesn't namespace the saved transforms
// in a directory based on the transform type.
// It just saves the transforms in outputRootDir/m_SliceNumber/m_iteration

#include "TransformWriterBase.hpp"

class SimpleTransformWriter : public TransformWriterBase
{
public:
  typedef SimpleTransformWriter      Self;
  typedef TransformWriterBase        Superclass;
  typedef itk::SmartPointer<Self>    Pointer;
  
  itkNewMacro( Self );
  
  virtual string dirPath()
  {
    return m_outputRootDir + m_stack->GetBasename(m_sliceNumber) + "/";
  }
  
};

#endif

#ifndef SIMPLEMETRICVALUEWRITER_HPP_
#define SIMPLEMETRICVALUEWRITER_HPP_

#include "MetricValueWriterBase.hpp"

class SimpleMetricValueWriter : public MetricValueWriterBase
{
public:
  typedef SimpleMetricValueWriter  Self;
  typedef MetricValueWriterBase    Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  
  itkNewMacro( Self );
  
  virtual string dirPath()
  {
    return m_outputRootDir;
  }
  
};

#endif

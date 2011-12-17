#ifndef STACKBASE_HPP_
#define STACKBASE_HPP_

#include "itkTransform.h"

using namespace std;

class StackBase {
public:
  typedef itk::Transform< double, 2, 2 > TransformType;
	typedef vector< TransformType::Pointer > TransformVectorType;
	
  StackBase();
  // abstract polymorphic base
  // see Effective C++ item 7
  virtual ~StackBase()=0;
  
private:
  // Copy constructor and copy assignment operator deliberately not implemented
  // Made private so that nobody can use them
  StackBase(const StackBase&);
  StackBase& operator=(const StackBase&);
  
public:
  virtual TransformType::Pointer GetTransform(unsigned int slice_number)=0;
  virtual const TransformVectorType& GetTransforms()=0;
  virtual const vector< string >& GetBasenames()=0;
  virtual const string& GetBasename(unsigned int slice_number)=0;
  virtual void SetBasenames(const vector< string >& basenames)=0;
  virtual void SetTransforms(const TransformVectorType& inputTransforms)=0;
  
};

StackBase::StackBase() {}
StackBase::~StackBase() {}

#endif
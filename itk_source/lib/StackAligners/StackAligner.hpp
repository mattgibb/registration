// This object constructs and encapsulates the registration of 
// HiRes to LoRes images in a stack.

// The stack aligner is responsible for:
// 1) Checking that both images exist
// 2) Trying registration up to 5 times, whilst shrinking the mask
// 3) Observers to write intermediate transforms and metric values


#ifndef STACKALIGNER_HPP_
#define STACKALIGNER_HPP_

// my files
#include "Stack.hpp"
#include "itkImageRegistrationMethod.h"


template <typename StackType>
class StackAligner {
public:
	typedef itk::ImageRegistrationMethod< typename StackType::SliceType, typename StackType::SliceType > RegistrationType;
	
  StackAligner(StackType &LoResStack,
               StackType &HiResStack,
               typename RegistrationType::Pointer registration);

  void Update();
  
protected:
  bool bothImagesExist(unsigned int slice_number);
  
  bool tryRegistration();
  
private:
  // Copy constructor and copy assignment operator Made private
  // so that no subclasses or clients can use them,
  // deliberately not implemented so not even class methods can use them
  StackAligner(const StackAligner&);
  StackAligner& operator=(const StackAligner&);
  
  StackType &m_LoResStack, &m_HiResStack;
  typename RegistrationType::Pointer m_registration;
};

#include "StackAligner.txx"
#endif

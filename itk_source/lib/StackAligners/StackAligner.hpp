// This object constructs and encapsulates the registration of 
// HiRes to LoRes images in a stack.

#ifndef STACKALIGNER_HPP_
#define STACKALIGNER_HPP_

// my files
#include "Stack.hpp"
#include "itkImageRegistrationMethod.h"


class StackAligner {
public:
	typedef itk::ImageRegistrationMethod< Stack::SliceType, Stack::SliceType > RegistrationType;
	
  StackAligner(Stack &LoResStack,
               Stack &HiResStack,
               RegistrationType::Pointer registration);

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
  
  Stack &m_LoResStack, &m_HiResStack;
  RegistrationType::Pointer m_registration;
};

#endif

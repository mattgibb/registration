#ifndef REGISTRATIONBUILDER_HPP_
#define REGISTRATIONBUILDER_HPP_

// ITK includes
#include "itkImageRegistrationMethod.h"

// my files
#include "Stack.hpp"


class RegistrationBuilder : public itk::Object {
public:
	typedef itk::ImageRegistrationMethod< Stack::SliceType, Stack::SliceType > RegistrationType;
  
  RegistrationBuilder();
  
  void buildRegistrationComponents();

  void buildRegistration();
  
  void buildMetric();
  
  void buildOptimizer();
  
  void buildInterpolator();
  
  void setUpObservers();
	
  RegistrationType::Pointer GetRegistration();
	
	// explicitly declare virtual destructor,
  // so that base pointers to derived classes will be destroyed fully
  // Made pure virtual to make class abstract
  // virtual ~RegistrationBuilder()=0;
  
private:
  // Copy constructor and copy assignment operator Made private
  // so that no subclasses or clients can use them,
  // deliberately not implemented so not even class methods can use them
  RegistrationBuilder(const RegistrationBuilder&);
  RegistrationBuilder& operator=(const RegistrationBuilder&);
	
	RegistrationType::Pointer m_registration;
};

#endif

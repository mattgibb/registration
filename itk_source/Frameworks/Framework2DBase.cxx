#ifndef FRAMEWORK2DBASE_CXX_
#define FRAMEWORK2DBASE_CXX_

#include "Framework2dBase.hpp"


Framework2DBase::Framework2DBase(YAML::Node& parameters):
registrationParameters(parameters) {
  buildRegistrationComponents();
  wireUpRegistrationComponents();
	setUpObservers();
}

Framework2DBase::~Framework2DBase() {}

#endif
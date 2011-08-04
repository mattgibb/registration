#ifndef _PARAMETERS_HPP_
#define _PARAMETERS_HPP_

// Singleton YAML parameters objects

#include <boost/shared_ptr.hpp>
#include "yaml-cpp/yaml.h"


YAML::Node& registrationParameters();

boost::shared_ptr<YAML::Node> config(const string& filename);

#endif

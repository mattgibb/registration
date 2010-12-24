#ifndef _REGISTRATIONPARAMETERS_CXX_
#define _REGISTRATIONPARAMETERS_CXX_

#include <fstream>
#include "yaml.h"
#include "Dirs.hpp"

YAML::Node& registrationParameters()
{
  // only initialize statics when registrationParameters is first called
  static YAML::Node registrationParameters;
  static bool initialized = false;
  
  // initialize registrationParameters if hasn't been already
  if(!initialized)
  {
    ifstream config_filestream( Dirs::ParamsFile().c_str() );
    YAML::Parser parser(config_filestream);
    parser.GetNextDocument(registrationParameters);
    initialized = true;
  }
  
  // return reference to singleton
  return registrationParameters;
}

#endif

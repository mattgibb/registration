#ifndef _PARAMETERS_CXX_
#define _PARAMETERS_CXX_

#include <fstream>
#include <boost/make_shared.hpp>
#include "yaml-cpp/yaml.h"
#include "Dirs.hpp"


YAML::Node& imageDimensions()
{
  // only initialize statics when registrationParameters is first called
  static YAML::Node imageDimensions;
  static bool initialized = false;
  
  // initialize registrationParameters if hasn't been already
  if(!initialized)
  {
    ifstream config_filestream( (Dirs::ConfigDir() + "image_dimensions.yml").c_str() );
    YAML::Parser parser(config_filestream);
    parser.GetNextDocument(imageDimensions);
    initialized = true;
  }
  
  // return reference to singleton
  return imageDimensions;
}


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


boost::shared_ptr<YAML::Node> config(const string& filename)
{
  boost::shared_ptr<YAML::Node> node = boost::make_shared<YAML::Node>();
  ifstream config_filestream( (Dirs::ConfigDir() + filename).c_str() );
  YAML::Parser parser(config_filestream);
  parser.GetNextDocument(*node);
  return node;
}

#endif

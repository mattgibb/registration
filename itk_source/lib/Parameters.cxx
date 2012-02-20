#ifndef _PARAMETERS_CXX_
#define _PARAMETERS_CXX_

#include "Parameters.hpp"
#include <fstream>
#include <boost/make_shared.hpp>
#include "yaml-cpp/yaml.h"
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


boost::shared_ptr<YAML::Node> config(const string& filename)
{
  boost::shared_ptr<YAML::Node> node = boost::make_shared<YAML::Node>();
  ifstream config_filestream( (Dirs::ConfigDir() + filename).c_str() );
  YAML::Parser parser(config_filestream);
  parser.GetNextDocument(*node);
  return node;
}

double getDownsampleRatio(const string& res)
{
  float ratio;
  (*config("downsample_ratios.yml"))[res] >> ratio;
  return ratio;
}

itk::Size<2> getSize(const string& region)
{
  // get size divided by downsample ratio
  itk::Size<2> size;
  for(unsigned int i=0; i<2; i++)
  {
    (*config("ROIs/" + region + ".yml"))["Size"][i] >> size[i];
    size[i] /= getDownsampleRatio("LoRes");
  }
  
  return size;
}


#endif

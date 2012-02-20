#ifndef _PARAMETERS_HPP_
#define _PARAMETERS_HPP_

#include <boost/shared_ptr.hpp>
#include "yaml-cpp/yaml.h"
#include <string>
#include "itkSize.h"
#include "itkVector.h"

using namespace std;

YAML::Node& registrationParameters();

boost::shared_ptr<YAML::Node> config(const string& filename);

double getDownsampleRatio(const string& res);

// get 2D or 3D "HiRes" or "LoRes" image spacings
// multiplied by the selected downsample ratio
template <int dim>
typename itk::Vector< double, dim > getSpacings(const string& res)
{
  typename itk::Vector< double, dim > spacings;
  
  // get spacings multiplied by downsample ratio
  for(unsigned int i=0; i<dim; ++i)
  {
    (*config("image_spacings.yml"))[res][i] >> spacings[i];
  }
  
  for(unsigned int i=0; i<2; ++i)
  {
    spacings[i] *= getDownsampleRatio(res);
  }
  
  return spacings;
}

// get default LoRes ROI size
// it's only 2 dimensions, as the z-dimension is defined by the number of slices
itk::Size<2> getSize(const string& region = "ROI");

#endif

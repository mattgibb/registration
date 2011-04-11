#ifndef STACKINITIALIZERS_HPP_
#define STACKINITIALIZERS_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Stack.hpp"
#include "Dirs.hpp"


template <typename StackType>
boost::shared_ptr< StackType > InitializeLoResStack(typename StackType::SliceVectorType Images)
{
  // get downsample ratio
  float DownsampleRatio;
  boost::shared_ptr<YAML::Node> downsample_ratios = config(Dirs::GetDataSet() + "/downsample_ratios.yml");
  (*downsample_ratios)["LoRes"] >> DownsampleRatio;
  
  // get spacings
  typename StackType::VolumeType::SpacingType Spacings;
  for(unsigned int i=0; i<3; i++) {
    imageDimensions()["LoResSpacings"][i] >> Spacings[i];
  }
  
  // multiply in-plane spacings by downsample ratio
  for(unsigned int i=0; i<2; i++) {
    Spacings[i] *= DownsampleRatio;
  }

  // get size and translation
  typename StackType::SliceType::SizeType Size;
  for(unsigned int i=0; i<2; i++) {
    imageDimensions()["LoResSize"][i] >> Size[i];
    Size[i] /= DownsampleRatio;
  }
  
  return boost::make_shared< StackType >(Images, Spacings, Size);
}

template <typename StackType>
boost::shared_ptr< StackType > InitializeHiResStack(typename StackType::SliceVectorType Images)
{
  // get downsample ratios
  float LoResDownsampleRatio, HiResDownsampleRatio;
  boost::shared_ptr<YAML::Node> downsample_ratios = config(Dirs::GetDataSet() + "/downsample_ratios.yml");
  (*downsample_ratios)["LoRes"] >> LoResDownsampleRatio;
  (*downsample_ratios)["HiRes"] >> HiResDownsampleRatio;
  
  // get spacings
  typename StackType::VolumeType::SpacingType LoResSpacings;
  typename StackType::SliceType::SpacingType HiResOriginalSpacings;
  for(unsigned int i=0; i<3; i++) imageDimensions()["LoResSpacings"][i] >> LoResSpacings[i];
  for(unsigned int i=0; i<2; i++) imageDimensions()["HiResSpacings"][i] >> HiResOriginalSpacings[i];
  
  // multiply in-plane spacings by downsample ratios
  for(unsigned int i=0; i<2; i++)
  {
    LoResSpacings[i]         *= LoResDownsampleRatio;
    HiResOriginalSpacings[i] *= HiResDownsampleRatio;
  }
  
  // get sizes
  typename StackType::SliceType::SizeType LoResSize;
  for(unsigned int i=0; i<2; i++) {
    imageDimensions()["LoResSize"][i] >> LoResSize[i];
    LoResSize[i] /= LoResDownsampleRatio;
  }
  
  return boost::make_shared< StackType >(Images, HiResOriginalSpacings, LoResSpacings, LoResSize);
}

#endif
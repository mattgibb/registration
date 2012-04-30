#ifndef LORESSTACKBUILDER_HPP_
#define LORESSTACKBUILDER_HPP_

#include "StackBuilder.hpp"


template <typename StackType>
class LoResStackBuilder: public StackBuilder< StackType > {
  virtual string getImageLoadDir();
  
  virtual typename StackType::SliceType::SpacingType getOriginalSpacings();
};

template<typename StackType>
string LoResStackBuilder<StackType>::getImageLoadDir()
{ return Dirs::BlockDir(); }

template<typename StackType>
typename StackType::SliceType::SpacingType LoResStackBuilder<StackType>::getOriginalSpacings()
{ return getSpacings<2>("LoRes"); }

#endif

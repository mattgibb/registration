#ifndef HIRESSTACKBUILDER_HPP_
#define HIRESSTACKBUILDER_HPP_

#include "StackBuilder.hpp"


template <typename StackType>
class HiResStackBuilder: public StackBuilder< StackType > {
  virtual string getDefaultImageLoadDir();
  
  virtual typename StackType::SliceType::SpacingType getOriginalSpacings();
};

template<typename StackType>
string HiResStackBuilder<StackType>::getDefaultImageLoadDir()
{ return Dirs::SliceDir(); }

template<typename StackType>
typename StackType::SliceType::SpacingType HiResStackBuilder<StackType>::getOriginalSpacings()
{ return getSpacings<2>("HiRes"); }

#endif

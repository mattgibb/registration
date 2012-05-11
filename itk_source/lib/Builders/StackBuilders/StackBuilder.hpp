#ifndef STACKBUILDER_HPP_
#define STACKBUILDER_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "StackBuilderBase.hpp"
#include "IOHelpers.hpp"
#include "NormalizeImages.hpp"
#include "ScaleImages.hpp"


template<typename StackType>
class StackBuilder : public StackBuilderBase
{
public:
  boost::shared_ptr<StackType> getStack();
  
protected:
  void buildStack();
  void loadSlices();
  void normalizeSlices();
  void scaleSlices();
  void constructStack();
  virtual typename StackType::SliceType::SpacingType getOriginalSpacings()=0;
  
  typename StackType::SliceVectorType m_images;
  
private:
  boost::shared_ptr<StackType> m_stack;
};

template<typename StackType>
boost::shared_ptr<StackType> StackBuilder<StackType>::getStack()
{
  buildStack();
  return m_stack;
}

template<typename StackType>
void StackBuilder<StackType>::buildStack()
{
  loadSlices();
  normalizeSlices();
  scaleSlices();
  constructStack();
}

template<typename StackType>
void StackBuilder<StackType>::loadSlices()
{
  vector< string > imagePaths = constructPaths(getImageLoadDir(), m_basenames, ".bmp");
  m_images = readImages< typename StackType::SliceType >(imagePaths);
}

template<typename StackType>
void StackBuilder<StackType>::normalizeSlices()
{
  // apply normalisation
  if(this->m_normalizeSlices)
  {
    cout << "Normalising images...";
    normalizeImages< typename StackType::SliceType >(m_images);
    cout << "done." << endl;
  }
}

template<typename StackType>
void StackBuilder<StackType>::scaleSlices()
{
  scaleImages< typename StackType::SliceType >(m_images, getOriginalSpacings());
}

template<typename StackType>
void StackBuilder<StackType>::constructStack()
{
  m_stack = boost::make_shared<StackType>(m_images, getSpacings<3>("LoRes"), getSize());
  m_stack->SetBasenames(m_basenames);
}


#endif


#ifndef NORMALIZEIMAGES_HPP_
#define NORMALIZEIMAGES_HPP_

#include "Parameters.hpp"
#include "itkNormalizeImageFilter.h"

template <typename StackType >
void normalizeImages(typename StackType::SliceVectorType& images) {
  // test if configured to normalise images
  bool normalizeImages;
  registrationParameters()["normalizeImages"] >> normalizeImages;
  cout << "normalizeImages: " << normalizeImages << endl;
  
  // apply normalisation
  if(normalizeImages)
  {
    typedef itk::NormalizeImageFilter< typename StackType::SliceType, typename StackType::SliceType > NormalizerType;
    typename NormalizerType::Pointer normalizer;
    for(unsigned int slice_number=0; slice_number<images.size(); slice_number++)
    {
      normalizer = NormalizerType::New();
      normalizer->SetInput( images[slice_number] );
      normalizer->Update();
      images[slice_number] = normalizer->GetOutput();
      images[slice_number]->DisconnectPipeline();
    }
    
  cout << "finished normalising" << endl;
  }
  
}

#endif

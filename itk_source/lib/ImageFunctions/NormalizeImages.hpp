#ifndef NORMALIZEIMAGES_HPP_
#define NORMALIZEIMAGES_HPP_

#include "Parameters.hpp"
#include "itkNormalizeImageFilter.h"

template <typename ImageType >
void normalizeImages(vector<typename ImageType::Pointer>& images)
{
  // test if configured to normalise images
  bool normalizeImages;
  registrationParameters()["normalizeImages"] >> normalizeImages;
  cout << "normalizeImages: " << normalizeImages << endl;
  
  // apply normalisation
  if(normalizeImages)
  {
    typedef itk::NormalizeImageFilter< ImageType, ImageType > NormalizerType;
    typedef typename vector<typename ImageType::Pointer>::iterator IterType;
    typename NormalizerType::Pointer normalizer;
  	for(IterType it = images.begin(); it != images.end(); ++it)
    {
      normalizer = NormalizerType::New();
      normalizer->SetInput( *it );
      normalizer->Update();
      *it = normalizer->GetOutput();
      (*it)->DisconnectPipeline();
    }
    
  cout << "finished normalising" << endl;
  }
  
}

#endif

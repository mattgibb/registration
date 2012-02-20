#ifndef SCALEIMAGES_HPP_
#define SCALEIMAGES_HPP_

#include "itkChangeInformationImageFilter.h"

template <typename ImageType>
void scaleImages(vector<typename ImageType::Pointer>& images, const typename ImageType::SpacingType& spacings) {
  // rescale original images
  typedef itk::ChangeInformationImageFilter< ImageType > ScalerType;
  typedef typename vector<typename ImageType::Pointer>::iterator IterType;
  typename ScalerType::Pointer scaler;
	for(IterType it = images.begin(); it != images.end(); ++it)
	{
	  scaler = ScalerType::New();
		scaler->ChangeSpacingOn();
		scaler->SetOutputSpacing( spacings );
    scaler->SetInput( *it );
    scaler->Update();
    *it = scaler->GetOutput();
    (*it)->DisconnectPipeline();
	}
}


#endif

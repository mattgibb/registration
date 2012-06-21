#ifndef SCALEIMAGES_HPP_
#define SCALEIMAGES_HPP_

template <typename ImageType>
void scaleImages(vector<typename ImageType::Pointer>& images, const typename ImageType::SpacingType& spacings)
{
  typedef typename vector<typename ImageType::Pointer>::iterator IterType;
  for(IterType it = images.begin(); it != images.end(); ++it)
	{
    (*it)->SetSpacing(spacings);
	}
}

#endif

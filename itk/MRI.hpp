// This object encapsulates the 3D MRI data.
// Its job is to initialise the data,
//  take transform parameters,
// then provide a volume and an associated mask.

#ifndef MRI_HPP_
#define MRI_HPP_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkResampleImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkChangeInformationImageFilter.h"
#include "itkImageSpatialObject.h"
#include "itkImageRegionIterator.h"

class MRI {
public:
	// unsigned char is native type, but multires can't handle unsigned types
  // typedef unsigned char MRIPixelType;
  typedef short MRIPixelType;
  typedef itk::Image< MRIPixelType, 3 > MRIVolumeType;
  typedef itk::ImageFileReader< MRIVolumeType > MRIVolumeReaderType;
	typedef itk::RescaleIntensityImageFilter< MRIVolumeType, MRIVolumeType > RescaleFilterType;
	typedef itk::ChangeInformationImageFilter< MRIVolumeType > ShrinkerType;
  
	
	MRIVolumeType::Pointer volume;
	MRIVolumeReaderType::Pointer mriVolumeReader;
	RescaleFilterType::Pointer rescaleFilter;
	ShrinkerType::Pointer resizer;
	
	MRI(char const *inputFileName) {
		readFile(inputFileName);
		rescaleIntensity();
		resizeImage(0.8);
	}
	
	void readFile(char const *inputFileName) {
		mriVolumeReader = MRIVolumeReaderType::New();
		mriVolumeReader->SetFileName( inputFileName );
		volume = mriVolumeReader->GetOutput();
	}
	
	void rescaleIntensity() {
		rescaleFilter = RescaleFilterType::New();
		rescaleFilter->SetOutputMinimum( 0 );
		rescaleFilter->SetOutputMaximum( 255 );
		rescaleFilter->SetInput( volume );
		volume = rescaleFilter->GetOutput();
	}
	
	void resizeImage(float factor) {
		resizer = ShrinkerType::New();
		resizer->ChangeSpacingOn();
		resizer->SetInput( volume );
		resizer->Update();
		ShrinkerType::SpacingType spacings3D = volume->GetSpacing();
		for(int i=0; i<3; i++) {
			spacings3D[i] = spacings3D[i]*factor;
		}
		resizer->SetOutputSpacing( spacings3D );
		volume = resizer->GetOutput();
	}
	
	MRIVolumeType::Pointer GetVolume() {
		return volume;
	}
	
protected:
};
#endif

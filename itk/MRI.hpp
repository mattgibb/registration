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
#include "itkImageMaskSpatialObject.h"
#include "itkImageRegionIterator.h"

class MRI {
public:
	// unsigned char is native type, but multires can't handle unsigned types
  // typedef unsigned char PixelType;
  typedef short PixelType;
  typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
  typedef itk::ImageFileReader< VolumeType > VolumeReaderType;
	typedef itk::RescaleIntensityImageFilter< VolumeType, VolumeType > RescaleFilterType;
	typedef itk::ChangeInformationImageFilter< VolumeType > ShrinkerType;
  typedef itk::ImageRegionIterator< MaskVolumeType > IteratorType;
  typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
	
	
	VolumeType::Pointer volume;
	MaskVolumeType::Pointer maskVolume;
	VolumeReaderType::Pointer volumeReader;
	RescaleFilterType::Pointer rescaleFilter;
	ShrinkerType::Pointer resizer;
	
	MRI(char const *inputFileName) {
		readFile(inputFileName);
		rescaleIntensity();
		resizeImage(0.8);
		buildMask();
	}
	
	void readFile(char const *inputFileName) {
		volumeReader = VolumeReaderType::New();
		volumeReader->SetFileName( inputFileName );
		volume = volumeReader->GetOutput();
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
	
	void buildMask() {
		// make new mask volume and make it all white
		MaskVolumeType::RegionType region;
		region.SetSize( volume->GetLargestPossibleRegion().GetSize() );
		
		maskVolume = MaskVolumeType::New();
		maskVolume->SetRegions( region );
		maskVolume->CopyInformation( volume );
		maskVolume->GetMetaDataDictionary().Print( cout );
	  maskVolume->Allocate();
		
	  IteratorType it(maskVolume,region);
	  for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
	    it.Set( 255 );
	  }
	
	}
	
	VolumeType::Pointer GetVolume() {
		return volume;
	}
	
	MaskVolumeType::Pointer GetMaskVolume() {
		return maskVolume;
	}
	
protected:
};
#endif

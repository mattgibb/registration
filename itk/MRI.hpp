// This object encapsulates the 3D MRI data.
// Its job is to initialise the data,
//  take transform parameters,
// then provide slices and associated masks.

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
#include "itkExtractImageFilter.h"

class MRI {
public:
	// unsigned char is native type, but multires can't handle unsigned types
  // typedef unsigned char PixelType;
  typedef short PixelType;
  typedef itk::Image< PixelType, 2 > SliceType;
	typedef itk::Image< unsigned char, 2 > MaskSliceType;
  typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
	typedef vector< SliceType::Pointer > SliceVectorType;
	typedef vector< MaskSliceType::Pointer > MaskSliceVectorType;
  typedef itk::ImageFileReader< VolumeType > ReaderType;
	typedef itk::RescaleIntensityImageFilter< VolumeType, VolumeType > RescaleIntensityFilterType;
	typedef itk::ChangeInformationImageFilter< VolumeType > ShrinkerType;
	typedef itk::ChangeInformationImageFilter< MaskVolumeType > MaskSpacerType;
	typedef itk::VersorRigid3DTransform< double > TransformType;
	typedef TransformType::ParametersType ParametersType;
  typedef itk::LinearInterpolateImageFunction< VolumeType, double > LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction< MaskVolumeType, double > NearestNeighborInterpolatorType;
  typedef itk::ResampleImageFilter< VolumeType, VolumeType > ResamplerType;
  typedef itk::ResampleImageFilter< MaskVolumeType, MaskVolumeType > MaskResamplerType;
  typedef itk::ExtractImageFilter< VolumeType, SliceType > SliceExtractorType;
  typedef itk::ExtractImageFilter< MaskVolumeType, MaskSliceType > MaskSliceExtractorType;
  typedef itk::ImageRegionIterator< MaskVolumeType > IteratorType;
  typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
  typedef itk::ImageMaskSpatialObject< 2 > MaskType2D;
	typedef vector< MaskType2D::Pointer > MaskVectorType2D;
  
	
	VolumeType::Pointer originalImage;
	MaskVolumeType::Pointer originalMask;
  SliceVectorType slices;
  MaskSliceVectorType maskSlices;
	MaskType3D::Pointer mask3D;
  MaskVectorType2D masks2D;
  ParametersType parameters;
  TransformType::Pointer transform;
  LinearInterpolatorType::Pointer linearInterpolator;
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator;
	RescaleIntensityFilterType::Pointer intensityRescaler;
	ShrinkerType::Pointer resizer;
	MaskSpacerType::Pointer maskSpacer;
  ResamplerType::Pointer resampler;
  VolumeType::SpacingType resamplerSpacing;
  VolumeType::SizeType resamplerSize;
  MaskResamplerType::Pointer maskResampler;
  SliceExtractorType::Pointer sliceExtractor;
  MaskSliceExtractorType::Pointer maskSliceExtractor;
  
	
	MRI(char const *inputFileName, VolumeType::SpacingType spacing, VolumeType::SizeType size):
	  parameters(6),
	  resamplerSpacing(spacing),
	  resamplerSize(size) {
		readFile(inputFileName);
		rescaleIntensity();
		resizeImage(0.8);
		buildOriginalMaskVolume();
		initialiseParameters();
    initialiseFilters();
    buildSlices();
    buildMaskSlices();
	}
	
	void readFile(char const *inputFileName) {
	  ReaderType::Pointer volumeReader = ReaderType::New();
		volumeReader->SetFileName( inputFileName );
    volumeReader->Update();
		originalImage = volumeReader->GetOutput();
	}
	
	void rescaleIntensity() {
		intensityRescaler = RescaleIntensityFilterType::New();
		intensityRescaler->SetOutputMinimum( 0 );
		intensityRescaler->SetOutputMaximum( 255 );
		intensityRescaler->SetInput( originalImage );
		intensityRescaler->Update();
		originalImage = intensityRescaler->GetOutput();
	}
	
	void resizeImage(float factor) {
		resizer = ShrinkerType::New();
		resizer->ChangeSpacingOn();
		resizer->SetInput( originalImage );
		ShrinkerType::SpacingType spacings3D = originalImage->GetSpacing();
		for(int i=0; i<3; i++) {
			spacings3D[i] = spacings3D[i]*factor;
		}
		resizer->SetOutputSpacing( spacings3D );
		resizer->Update();
		originalImage = resizer->GetOutput();
	}
	
	void buildOriginalMaskVolume() {
		// make new mask volume and make it all white
		MaskVolumeType::RegionType region;
		region.SetSize( originalImage->GetLargestPossibleRegion().GetSize() );
		
		originalMask = MaskVolumeType::New();
		originalMask->SetRegions( region );
		originalMask->CopyInformation( originalImage );
	  originalMask->Allocate();
    originalMask->FillBuffer( 255 );
		
		maskSpacer = MaskSpacerType::New();
		maskSpacer->ChangeSpacingOn();
		maskSpacer->SetOutputSpacing( originalImage->GetSpacing() );
				
		maskSpacer->SetInput( originalMask );
		maskSpacer->Update();
		originalMask = maskSpacer->GetOutput();
				
	  mask3D = MaskType3D::New();
		mask3D->SetImage( originalMask );
	}
	
	void initialiseParameters() {
		parameters.Fill(0);
	}
	
	void initialiseFilters() {
		// resamplers
		transform = TransformType::New();
	  transform->SetParameters( parameters );
		linearInterpolator = LinearInterpolatorType::New();
		nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
		resampler = ResamplerType::New();
    resampler->SetInput( originalImage );
		resampler->SetInterpolator( linearInterpolator );
		resampler->SetOutputSpacing( resamplerSpacing );
		resampler->SetSize( resamplerSize );
		resampler->SetTransform( transform );
		maskResampler = MaskResamplerType::New();
		maskResampler->SetInput( originalMask );
		maskResampler->SetInterpolator( nearestNeighborInterpolator );
		maskResampler->SetOutputSpacing( resamplerSpacing );
		maskResampler->SetSize( resamplerSize );
		maskResampler->SetTransform( transform );
		
		// extract image filters
		sliceExtractor = SliceExtractorType::New();
    sliceExtractor->SetInput( resampler->GetOutput() );
		maskSliceExtractor = MaskSliceExtractorType::New();
    maskSliceExtractor->SetInput( maskResampler->GetOutput() );
	    
	  
	}
	
	void buildSlices() {
	  // set up extractor
    VolumeType::SizeType size = resamplerSize;
    size[2] = 0;
    
    VolumeType::IndexType sliceIndex = {{0, 0, 0}};
    
    VolumeType::RegionType sliceRegion;
    sliceRegion.SetSize( size );
    sliceRegion.SetIndex( sliceIndex );
    
    sliceExtractor->SetExtractionRegion( sliceRegion );
    
    slices.clear();
    
    for(unsigned int i=0; i<resamplerSize[2]; i++) {
      // Set the z-coordinate of the slice to be extracted
      sliceIndex[2] = i;
      
      // add output to slices vector
      // sliceExtractor->Update();
      slices.push_back( sliceExtractor->GetOutput() );
      slices.back()->DisconnectPipeline();
    }
    
	}
	
	void buildMaskSlices() {
	  // set up extractor
    VolumeType::SizeType size = resamplerSize;
    size[2] = 0;
    
    VolumeType::IndexType sliceIndex = {{0, 0, 0}};
    
    VolumeType::RegionType sliceRegion;
    sliceRegion.SetSize( size );
    sliceRegion.SetIndex( sliceIndex );
    
    maskSliceExtractor->SetExtractionRegion( sliceRegion );
    
    maskSlices.clear();
    
    for(unsigned int i=0; i<resamplerSize[2]; i++) {
      // Set the z-coordinate of the slice to be extracted
      sliceIndex[2] = i;
      
      // add output to slices vector
      // maskSliceExtractor->Update();
      maskSlices.push_back( maskSliceExtractor->GetOutput() );
      maskSlices.back()->DisconnectPipeline();
    }
  
	}
	
	VolumeType::Pointer GetVolume() {
		return originalImage;
	}
	
	MaskVolumeType::Pointer GetMaskVolume() {
		return originalMask;
	}
	
	MaskType3D::Pointer GetMask3D() {
		return mask3D;
	}
	
protected:
};
#endif

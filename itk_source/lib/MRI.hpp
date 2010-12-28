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
  // typedef itk::LinearInterpolateImageFunction< VolumeType, double > VolumeInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction< VolumeType, double > VolumeInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction< MaskVolumeType, double > MaskVolumeInterpolatorType;
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
  TransformType::Pointer transform;
  VolumeInterpolatorType::Pointer volumeInterpolator;
	MaskVolumeInterpolatorType::Pointer maskVolumeInterpolator;
	RescaleIntensityFilterType::Pointer intensityRescaler;
	ShrinkerType::Pointer resizer;
	MaskSpacerType::Pointer maskSpacer;
  ResamplerType::Pointer resampler;
  VolumeType::SpacingType resamplerSpacing;
  VolumeType::SizeType resamplerSize;
  MaskResamplerType::Pointer maskResampler;
  SliceExtractorType::Pointer sliceExtractor;
  MaskSliceExtractorType::Pointer maskSliceExtractor;
  
	
	MRI(char const *inputFileName, VolumeType::SpacingType spacing, VolumeType::SizeType size, double initialResizeFactor):
	  resamplerSpacing(spacing),
	  resamplerSize(size) {
		readFile(inputFileName);
		rescaleIntensity();
		resizeImage(initialResizeFactor);
		buildOriginalMaskVolume();
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
	
	void initialiseFilters() {
		// resamplers
		transform = TransformType::New();
	  transform->SetIdentity();
    volumeInterpolator = VolumeInterpolatorType::New();
		maskVolumeInterpolator = MaskVolumeInterpolatorType::New();
		resampler = ResamplerType::New();
    resampler->SetInput( originalImage );
		resampler->SetInterpolator( volumeInterpolator );
		resampler->SetOutputSpacing( resamplerSpacing );
		resampler->SetSize( resamplerSize );
		resampler->SetTransform( transform );
		resampler->SetDefaultPixelValue( 127 );
		maskResampler = MaskResamplerType::New();
		maskResampler->SetInput( originalMask );
		maskResampler->SetInterpolator( maskVolumeInterpolator );
		maskResampler->SetOutputSpacing( resamplerSpacing );
		maskResampler->SetSize( resamplerSize );
		maskResampler->SetTransform( transform );
		
		// extract image filters
    sliceExtractor = SliceExtractorType::New();
    sliceExtractor->SetInput( resampler->GetOutput() );
		maskSliceExtractor = MaskSliceExtractorType::New();
    maskSliceExtractor->SetInput( maskResampler->GetOutput() );
    
    // masks
    for(unsigned int i=0; i<resamplerSize[2]; i++) {
  		masks2D.push_back( MaskType2D::New() );
    }		
	}
	
	void buildSlices() {
	  resampler->Update();
	  
	  // set up extractor
    VolumeType::SizeType size = resamplerSize;
    size[2] = 0;
    
    VolumeType::IndexType sliceIndex = {{0, 0, 0}};
    
    VolumeType::RegionType sliceRegion;
    sliceRegion.SetSize( size );
    sliceRegion.SetIndex( sliceIndex );
    
    slices.clear();
    
    for(unsigned int i=0; i<resamplerSize[2]; i++) {
      // Set the z-coordinate of the slice to be extracted
      sliceRegion.SetIndex(2, i);
      
      sliceExtractor->SetExtractionRegion( sliceRegion );
      
      // add output to slices vector
      sliceExtractor->Update();
      slices.push_back( sliceExtractor->GetOutput() );
      slices.back()->DisconnectPipeline();
    }
    
	}
	
	void buildMaskSlices() {
    maskResampler->Update();
	  
	  // set up extractor
    VolumeType::SizeType size = resamplerSize;
    size[2] = 0;
    
    VolumeType::IndexType sliceIndex = {{0, 0, 0}};
    
    VolumeType::RegionType sliceRegion;
    sliceRegion.SetSize( size );
    sliceRegion.SetIndex( sliceIndex );
    
    maskSlices.clear();
    
    for(unsigned int i=0; i<resamplerSize[2]; i++) {
      // Set the z-coordinate of the slice to be extracted
      sliceRegion.SetIndex(2, i);
      
      maskSliceExtractor->SetExtractionRegion( sliceRegion );
    
      // add output to slices vector
      maskSliceExtractor->Update();
      maskSlices.push_back( maskSliceExtractor->GetOutput() );
  		masks2D[i]->SetImage( maskSlices.back() );
      maskSlices.back()->DisconnectPipeline();
    }
  
	}
	
	void SetTransformParameters(TransformType::Pointer inputTransform) {
    transform->SetParameters( inputTransform->GetParameters() );
    transform->SetFixedParameters( inputTransform->GetFixedParameters() );
    buildSlices();
    buildMaskSlices();
	}
	
	VolumeType::Pointer GetVolume() {
		return originalImage;
	}
	
	MaskVolumeType::Pointer GetMaskVolume() {
		return originalMask;
	}
	
	MaskType3D::Pointer Get3DMask() {
		return mask3D;
	}
	
	VolumeType::Pointer GetResampledVolume() {
    return resampler->GetOutput();
	}
	
	MaskVolumeType::Pointer GetResampledMaskVolume() {
    return maskResampler->GetOutput();
	}
	
	SliceType::Pointer GetResampledSlice(unsigned int slice_number) {
    return slices[slice_number];
	}
	
	MaskSliceType::Pointer GetResampledMaskSlice(unsigned int slice_number) {
    return maskSlices[slice_number];
	}
	
	MaskType2D::Pointer GetMask2D(unsigned int slice_number) {
		return masks2D[slice_number];
	}
	
protected:
};
#endif

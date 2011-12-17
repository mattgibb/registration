// This object is built with some original images.
// Its job is to take transform parameters,
// then build a volume and an associated mask.

#ifndef STACK_HPP_
#define STACK_HPP_

#include "itkImage.h"
#include "itkTileImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkVectorResampleImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkVectorLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkChangeInformationImageFilter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkImageRegionIterator.h"

#include "StackBase.hpp"

template <typename TPixel,
          template<typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType> class ResampleImageFilterType,
          template<typename TInputImage, typename TCoordRep> class InterpolatorType >
class Stack: public StackBase {
public:
  typedef TPixel PixelType;
	typedef itk::Image< PixelType, 2 > SliceType;
	typedef itk::Image< unsigned char, 2 > MaskSliceType;
	typedef itk::ImageRegionIterator< MaskSliceType > MaskSliceIteratorType;
	typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
	typedef vector< typename SliceType::Pointer > SliceVectorType;
  typedef vector< MaskSliceType::Pointer > MaskSliceVectorType;
  typedef InterpolatorType< SliceType, double > LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction< MaskSliceType, double > NearestNeighborInterpolatorType;
	typedef ResampleImageFilterType< SliceType, SliceType, double > ResamplerType;
	typedef itk::ResampleImageFilter< MaskSliceType, MaskSliceType, double > MaskResamplerType;
  typedef itk::TileImageFilter< SliceType, VolumeType > TileFilterType;
  typedef itk::TileImageFilter< MaskSliceType, MaskVolumeType > MaskTileFilterType;
  typedef itk::ChangeInformationImageFilter< SliceType > XYScaleType;
  typedef itk::ChangeInformationImageFilter< VolumeType > ZScaleType;
  typedef itk::ChangeInformationImageFilter< MaskVolumeType > MaskZScaleType;
	typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
  typedef itk::ImageMaskSpatialObject< 2 > MaskType2D;
	typedef vector< MaskType2D::Pointer > MaskVectorType2D;
	
private:
	SliceVectorType originalImages;
	typename XYScaleType::Pointer xyScaler;
  SliceVectorType slices;
	typename VolumeType::Pointer volume;
  typename SliceType::SizeType maxSize;
	typename SliceType::SizeType resamplerSize;
	typename SliceType::SpacingType originalSpacings;
	typename VolumeType::SpacingType spacings;
	typename MaskType3D::Pointer mask3D;
	MaskVectorType2D original2DMasks;
	MaskVectorType2D resampled2DMasks;
	typename LinearInterpolatorType::Pointer linearInterpolator;
	typename NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator;
	typename ResamplerType::Pointer resampler;
	typename MaskResamplerType::Pointer maskResampler;
	typename TileFilterType::Pointer tileFilter;
	typename TileFilterType::LayoutArrayType layout;
	typename MaskTileFilterType::Pointer maskTileFilter;
	typename ZScaleType::Pointer zScaler;
	typename MaskZScaleType::Pointer maskZScaler;
	TransformVectorType transforms;
  vector< unsigned int > numberOfTimesTooBig;
  vector< string > m_basenames;
  
public:
  // constructor to center images and size stack to fit in the longest and widest image
  Stack(const SliceVectorType& images, const typename VolumeType::SpacingType& inputSpacings);

	// constructor to specify size and start index explicitly
  Stack(const SliceVectorType& images, const typename VolumeType::SpacingType& inputSpacings,
        const typename SliceType::SizeType& inputSize);
	
	// constructor to specify stack size and spacing, and spacing of original images
  Stack(const SliceVectorType& images, const typename SliceType::SpacingType& inputOriginalSpacings,
        const typename VolumeType::SpacingType& inputSpacings, const typename SliceType::SizeType& inputSize);
	
protected:
  void initializeVectors();
  
  void scaleOriginalSlices();
	
  void buildOriginalMaskSlices();
	
  void calculateMaxSize();
	
	// Stack is just big enough to fit the longest and widest slices in
  void setResamplerSizeToMaxSize();
  
  void initializeFilters();
	
public:	
  void updateVolumes();
	
protected:
  void buildSlices();
	
  void buildVolume();
  
  void buildMaskSlices();
	
  void buildMaskSlice(unsigned int slice_number);
	
  void buildMaskVolume();
	
  void checkSliceNumber(unsigned int slice_number) const;
	
public:
  // Getter methods
  unsigned short GetSize() const { return originalImages.size(); }
  
  const typename SliceType::SizeType& GetMaxSize() const { return maxSize; }

  const typename SliceType::SizeType& GetResamplerSize() const { return resamplerSize; }
  
  const typename VolumeType::SpacingType& GetSpacings() const { return spacings; }

  const typename SliceType::SpacingType& GetOriginalSpacings() const { return originalSpacings; }
          
  typename SliceType::Pointer GetOriginalImage(unsigned int slice_number) {
    checkSliceNumber(slice_number);
  	return originalImages[slice_number];
  }
	
  typename MaskType2D::Pointer GetOriginal2DMask(unsigned int slice_number) {
  	checkSliceNumber(slice_number);
    return original2DMasks[slice_number];
  }
	
  typename SliceType::Pointer GetResampledSlice(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return slices[slice_number];
  }
  
  typename MaskType2D::Pointer GetResampled2DMask(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return resampled2DMasks[slice_number];
  }
  
  typename VolumeType::Pointer GetVolume() { return volume; }
	
  typename MaskType3D::Pointer Get3DMask() { return mask3D; }
	
  virtual TransformType::Pointer GetTransform(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return transforms[slice_number];
  }
	
  virtual const TransformVectorType& GetTransforms() { return transforms; }
  
  virtual const vector< string >& GetBasenames()
  {
    // if you're requesting basenames,
    // make sure you've sensibly set them first
    assert(m_basenames.size() == originalImages.size());
    
    return m_basenames;
  }
  
  virtual const string& GetBasename(unsigned int slice_number)
  {
    // if you're requesting basenames,
    // make sure you've sensibly set them first
    assert(m_basenames.size() == originalImages.size());
    
    return m_basenames[slice_number];
  }
  
  virtual void SetBasenames(const vector< string >& basenames)
  {
    // sanity check
    assert(basenames.size() == originalImages.size());
    m_basenames = basenames;
  }
  
  virtual void SetTransforms(const TransformVectorType& inputTransforms) { transforms = inputTransforms; }
  
  bool ImageExists(unsigned int slice_number) {
    return GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0];
  }
  
  void ShrinkMaskSlice(unsigned int slice_number);
  
  const vector< unsigned int >& GetNumberOfTimesTooBig() { return numberOfTimesTooBig; }
  
  void SetNumberOfTimesTooBig(const vector< unsigned int >& numbers);
  
  void SetDefaultPixelValue(PixelType p) { resampler->SetDefaultPixelValue(p); }
  
protected:
  void GenerateMaskSlice(unsigned int slice_number);
  
  typename SliceType::SpacingType spacings2D() const {
    typename SliceType::SpacingType spacings2D;
    for(unsigned int i=0; i<2; i++) spacings2D[i] = spacings[i];
    return spacings2D;
  }
  
};

#include "Stack.txx"
#endif

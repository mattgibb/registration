// This object is built with some original images.
// Its job is to take transform parameters,
// then build a volume and an associated mask.

#ifndef STACK_HPP_
#define STACK_HPP_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkTileImageFilter.h"
#include "itkNormalizeImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkChangeInformationImageFilter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkImageRegionIterator.h"


using namespace std;

class Stack {
public:
  typedef float PixelType;
	typedef itk::Image< PixelType, 2 > SliceType;
	typedef itk::Image< unsigned char, 2 > MaskSliceType;
	typedef itk::ImageRegionIterator< MaskSliceType > MaskSliceIteratorType;
	typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
	typedef vector< SliceType::Pointer > SliceVectorType;
  typedef vector< MaskSliceType::Pointer > MaskSliceVectorType;
  typedef itk::ImageFileReader< SliceType > ReaderType;
	typedef itk::Transform< double, 2, 2 > TransformType;
	typedef vector< TransformType::Pointer > TransformVectorType;
  typedef itk::LinearInterpolateImageFunction< SliceType, double > LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction< MaskSliceType, double > NearestNeighborInterpolatorType;
  typedef itk::NormalizeImageFilter< SliceType, SliceType > NormalizerType;
	typedef itk::ResampleImageFilter< SliceType, SliceType > ResamplerType;
	typedef itk::ResampleImageFilter< MaskSliceType, MaskSliceType > MaskResamplerType;
  typedef itk::TileImageFilter< SliceType, VolumeType > TileFilterType;
  typedef itk::TileImageFilter< MaskSliceType, MaskVolumeType > MaskTileFilterType;
  typedef itk::ChangeInformationImageFilter< SliceType > XYScaleType;
  typedef itk::ChangeInformationImageFilter< VolumeType > ZScaleType;
  typedef itk::ChangeInformationImageFilter< MaskVolumeType > MaskZScaleType;
	typedef itk::ImageMaskSpatialObject< 3 > MaskType3D;
  typedef itk::ImageMaskSpatialObject< 2 > MaskType2D;
	typedef vector< MaskType2D::Pointer > MaskVectorType2D;
	
private:
  const vector< string > fileNames;
	SliceVectorType originalImages;
	XYScaleType::Pointer xyScaler;
  SliceVectorType slices;
	VolumeType::Pointer volume;
  SliceType::SizeType maxSize;
	SliceType::SizeType resamplerSize;
	SliceType::SpacingType originalSpacings;
	VolumeType::SpacingType spacings;
	MaskType3D::Pointer mask3D;
	MaskVectorType2D original2DMasks;
	MaskVectorType2D resampled2DMasks;
	TransformVectorType transforms;
	LinearInterpolatorType::Pointer linearInterpolator;
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator;
	NormalizerType::Pointer normalizer;
	ResamplerType::Pointer resampler;
	MaskResamplerType::Pointer maskResampler;
	TileFilterType::Pointer tileFilter;
	TileFilterType::LayoutArrayType layout;
	MaskTileFilterType::Pointer maskTileFilter;
	ZScaleType::Pointer zScaler;
	MaskZScaleType::Pointer maskZScaler;
  vector< unsigned int > numberOfTimesTooBig;
	
public:
  // constructor to center images and size stack to fit in the longest and widest image
  Stack(const vector< string >& inputFileNames, const VolumeType::SpacingType& inputSpacings);

	// constructor to specify size and start index explicitly
  Stack(const vector< string >& inputFileNames, const VolumeType::SpacingType& inputSpacings,
        const SliceType::SizeType& inputSize);
	
	// constructor to specify stack size and spacing, and spacing of original images
  Stack(const vector< string >& inputFileNames, const SliceType::SpacingType& inputOriginalSpacings,
        const VolumeType::SpacingType& inputSpacings, const SliceType::SizeType& inputSize);
	
protected:
  void readImages();
  
  void normalizeImages();
  
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
  const string& GetFileName(unsigned int slice_number) const {
    checkSliceNumber(slice_number);
    return fileNames[slice_number];
  }
    
  unsigned short GetSize() const { return originalImages.size(); }
  
  const SliceType::SizeType& GetMaxSize() const { return maxSize; }

  const SliceType::SizeType& GetResamplerSize() const { return resamplerSize; }
  
  const VolumeType::SpacingType& GetSpacings() const { return spacings; }

  const SliceType::SpacingType& GetOriginalSpacings() const { return originalSpacings; }
          
  SliceType::Pointer GetOriginalImage(unsigned int slice_number) {
    checkSliceNumber(slice_number);
  	return originalImages[slice_number];
  }
	
  MaskType2D::Pointer GetOriginal2DMask(unsigned int slice_number) {
  	checkSliceNumber(slice_number);
    return original2DMasks[slice_number];
  }
	
  SliceType::Pointer GetResampledSlice(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return slices[slice_number];
  }
  
  MaskType2D::Pointer GetResampled2DMask(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return resampled2DMasks[slice_number];
  }
  
  VolumeType::Pointer GetVolume() { return volume; }
	
  MaskType3D::Pointer Get3DMask() { return mask3D; }
	
  TransformType::Pointer GetTransform(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return transforms[slice_number];
  }
	
  const TransformVectorType& GetTransforms() { return transforms; }
	
  void SetTransforms(const TransformVectorType& inputTransforms) { transforms = inputTransforms; }
  
  bool ImageExists(unsigned int slice_number) {
    return GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0];
  }
  
  void ShrinkMaskSlice(unsigned int slice_number);

protected:
  void GenerateMaskSlice(unsigned int slice_number);
	
  static bool fileExists(const string& strFilename);
  
  SliceType::SpacingType spacings2D() const {
    SliceType::SpacingType spacings2D;
    for(unsigned int i=0; i<2; i++) spacings2D[i] = spacings[i];
    return spacings2D;
  }
  
private:
  // Copy constructor and copy assignment operator deliberately not implemented
  // Made private so that nobody can use them
  Stack(const Stack&);
  Stack& operator=(const Stack&);

};
#endif

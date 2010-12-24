// This object is built with some original images.
// Its job is to take transform parameters,
// then build a volume and an associated mask.

#ifndef STACK_HPP_
#define STACK_HPP_

// TEMP
// YAML config reader
#include "yaml.h"

// ITK includes
#include "itkImage.h"
#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageMaskSpatialObject.h"

// File IO
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkSimilarity2DTransform.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
// TEMP



#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkTileImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkChangeInformationImageFilter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkImageRegionIterator.h"


class Stack {
public:
  typedef short PixelType;
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
	typedef itk::ResampleImageFilter< SliceType, SliceType > ResamplerType;
	typedef itk::ResampleImageFilter< MaskSliceType, MaskSliceType > MaskResamplerType;
  typedef itk::TileImageFilter< SliceType, VolumeType > TileFilterType;
  typedef itk::TileImageFilter< MaskSliceType, MaskVolumeType > MaskTileFilterType;
  typedef itk::ChangeInformationImageFilter< SliceType > XYScaleType;
  typedef itk::ChangeInformationImageFilter< VolumeType > ZScaleType;
  typedef itk::ChangeInformationImageFilter< MaskVolumeType > MaskZScaleType;
  typedef itk::ImageRegionIterator< MaskSliceType > IteratorType;
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
	SliceType::SizeType offset;
	VolumeType::SpacingType spacings;
	MaskType3D::Pointer mask3D;
	MaskVectorType2D original2DMasks;
	MaskVectorType2D resampled2DMasks;
	TransformVectorType transforms;
	LinearInterpolatorType::Pointer linearInterpolator;
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator;
	ResamplerType::Pointer resampler;
	MaskResamplerType::Pointer maskResampler;
	TileFilterType::Pointer tileFilter;
	TileFilterType::LayoutArrayType layout;
	MaskTileFilterType::Pointer maskTileFilter;
	ZScaleType::Pointer zScaler;
	MaskZScaleType::Pointer maskZScaler;
  vector< unsigned int > numberOfTimesTooBig;
	
public:
  Stack(const vector< string >& inputFileNames, VolumeType::SpacingType inputSpacings);

	// constructor to specify size and offset
  Stack(const vector< string >& inputFileNames, VolumeType::SpacingType inputSpacings,
        const SliceType::SizeType& inputSize, const SliceType::SizeType& inputOffset);
	
protected:
  void readImages();
  
  void initializeVectors();
  
  void scaleOriginalSlices();
	
  void buildOriginalMaskSlices();
	
  void calculateMaxSize();
	
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
  const string& GetFileName(unsigned int slice_number) const;
    
  unsigned short GetSize() const;
  
  const SliceType::SizeType& GetMaxSize() const;

  const SliceType::SizeType& GetResamplerSize() const;
  
  const SliceType::SizeType& GetOffset() const;
  
  const VolumeType::SpacingType& GetSpacings() const;
      
  SliceType::Pointer GetOriginalImage(unsigned int slice_number);
	
  MaskType2D::Pointer GetOriginal2DMask(unsigned int slice_number);
	
  SliceType::Pointer GetResampledSlice(unsigned int slice_number);
  
  MaskType2D::Pointer GetResampled2DMask(unsigned int slice_number);
  
  VolumeType::Pointer GetVolume();
	
  MaskType3D::Pointer Get3DMask();
	
  TransformType::Pointer GetTransform(unsigned int slice_number);
	
  const TransformVectorType& GetTransforms() const;
	
  void SetTransforms(const TransformVectorType& inputTransforms);
  
  bool ImageExists(unsigned int slice_number);
  
  void ShrinkSliceMask(unsigned int slice_number);
	
protected:
  static bool fileExists(const string& strFilename);
  
private:
  // Copy constructor and copy assignment operator deliberately not implemented
  // Made private so that nobody can use them
  Stack(const Stack&);
  Stack& operator=(const Stack&);

};
#endif

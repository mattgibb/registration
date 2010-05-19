// This object is built with some original images.
// Its job is to take transform parameters,
// then build a volume and an associated mask.

#ifndef STACK_HPP_
#define STACK_HPP_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkTileImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkCenteredRigid2DTransform.h"
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
	typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
	typedef vector< SliceType::Pointer > SliceVectorType;
  typedef vector< MaskSliceType::Pointer > MaskSliceVectorType;
  typedef itk::ImageFileReader< SliceType > ReaderType;
	typedef itk::CenteredRigid2DTransform< double > TransformType;
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
	
	
	SliceVectorType originalImages;
	XYScaleType::Pointer xyScaler;
  MaskSliceVectorType originalMasks;
	VolumeType::Pointer volume;
	MaskVolumeType::Pointer maskVolume;
	SliceType::SizeType maxSize;
	VolumeType::SpacingType spacings;
	MaskType3D::Pointer mask3D;
	MaskVectorType2D masks2D;
	vector< TransformType::Pointer > transforms;
	LinearInterpolatorType::Pointer linearInterpolator;
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator;
	ResamplerType::Pointer resampler;
	MaskResamplerType::Pointer maskResampler;
	TileFilterType::Pointer tileFilter;
	TileFilterType::LayoutArrayType layout;
	MaskTileFilterType::Pointer maskTileFilter;
	ZScaleType::Pointer zScaler;
	MaskZScaleType::Pointer maskZScaler;
  YAML::Node& registrationParameters;
	
	Stack(vector< string > fileNames, YAML::Node& parameters ):
	registrationParameters(parameters) {
		ReaderType::Pointer reader;
		
		for(unsigned int i=0; i<fileNames.size(); i++) {
			reader = ReaderType::New();
			reader->SetFileName( fileNames[i] );
			reader->Update();
			originalImages.push_back( reader->GetOutput() );
			originalImages.back()->DisconnectPipeline();
		}
		
		// scale slices and initialise volume and mask
    scaleOriginalSlices();
    buildOriginalMaskSlices();
    buildOriginalMasks();
    calculateMaxSize();
		initialiseFilters();
		initializeCenteredTransforms();
		buildVolume();
		buildMask();
	}
	
	void scaleOriginalSlices() {
    // initialise spacings
		for(unsigned int i=0; i<3; i++) {
      registrationParameters["stackSpacings"][i] >> spacings[i];
	  }
	  
    SliceType::SpacingType spacings2D;
		for(unsigned int i=0; i<2; i++) {
      spacings2D[i] = spacings[i];
	  }
	  
	  // rescale original images
    xyScaler = XYScaleType::New();
		xyScaler->ChangeSpacingOn();
		xyScaler->SetOutputSpacing( spacings2D );
		for(unsigned int i=0; i<originalImages.size(); i++) {
      xyScaler->SetInput( originalImages[i] );
      xyScaler->Update();
      originalImages[i] = xyScaler->GetOutput();
      originalImages[i]->DisconnectPipeline();
		}
	}
	
	void buildOriginalMaskSlices() {
		// build a vector of mask slices
		for(unsigned int i=0; i<originalImages.size(); i++) {
			// make new maskSlice and make it all white
			MaskSliceType::RegionType region;
			region.SetSize( originalImages[i]->GetLargestPossibleRegion().GetSize() );
		  
			MaskSliceType::Pointer maskSlice = MaskSliceType::New();
			maskSlice->SetRegions( region );
			maskSlice->CopyInformation( originalImages[i] );
		  maskSlice->Allocate();
			maskSlice->FillBuffer( 255 );
		  
		  originalMasks.push_back( maskSlice );
      // originalMasks.back()->DisconnectPipeline();
			
	  }
	}
	
	void buildOriginalMasks() {
	  // build a vector of masks from the mask slices
    for(unsigned int i=0; i<originalImages.size(); i++) {
      // make new 2D masks and assign mask slices to them
      masks2D.push_back( MaskType2D::New() );
      masks2D.back()->SetImage( originalMasks[i] );
    }
	}
	
	void calculateMaxSize() {
		// maxSize
		SliceType::SizeType size;
		maxSize.Fill(0);
		unsigned int dimension = size.GetSizeDimension();

		for(unsigned int i=0; i<originalImages.size(); i++) {
			size = originalImages[i]->GetLargestPossibleRegion().GetSize();

			for(unsigned int j=0; j<dimension; j++)
			{
				if(maxSize[j] < size[j]) { maxSize[j] = size[j]; }
			}
		}
	}
	
	void initialiseFilters() {
		// resamplers
		linearInterpolator = LinearInterpolatorType::New();
		nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
		resampler = ResamplerType::New();
		resampler->SetInterpolator( linearInterpolator );
		resampler->SetSize( maxSize );
		resampler->SetOutputSpacing( originalImages[0]->GetSpacing() );
		maskResampler = MaskResamplerType::New();
		maskResampler->SetInterpolator( nearestNeighborInterpolator );
		maskResampler->SetSize( maxSize );
		maskResampler->SetOutputSpacing( originalImages[0]->GetSpacing() );
		
		// z scalers
		zScaler     = ZScaleType::New();
		maskZScaler = MaskZScaleType::New();
		zScaler    ->ChangeSpacingOn();
		maskZScaler->ChangeSpacingOn();
		zScaler    ->SetOutputSpacing( spacings );
		maskZScaler->SetOutputSpacing( spacings );
		
		// tile filter layout
		layout[0] = 1;
	  layout[1] = 1;
	  layout[2] = 0;
		
		// masks
		mask3D = MaskType3D::New();
	}
	
	void initializeCenteredTransforms() {
    transforms.clear();
		SliceType::SizeType size;
    TransformType::ParametersType parameters(5);
    
    for(unsigned int i=0; i<originalImages.size(); i++)
		{
			// calculate parameters
			size = originalImages[i]->GetLargestPossibleRegion().GetSize();
			parameters[0] = 0;
			parameters[1] = spacings[0] * maxSize[0] / 2.0;
			parameters[2] = spacings[1] * maxSize[1] / 2.0;
			parameters[3] = - spacings[0] * ( maxSize[0] - size[0] ) / 2.0;
			parameters[4] = - spacings[1] * ( maxSize[1] - size[1] ) / 2.0;
			
			// set them to new transform
      transforms.push_back( TransformType::New() );
      transforms.back()->SetParameters( parameters );
		}
		
	}
	
	void buildVolume() {
	  // construct tile filter
		tileFilter = TileFilterType::New();
		tileFilter    ->SetLayout( layout );
		
    for(unsigned int i=0; i<originalImages.size(); i++) {
			// resample transformed image
			resampler->SetInput( originalImages[i] );
			resampler->SetTransform( transforms[i] );
			resampler->Update();
			SliceType::Pointer transformedImage = resampler->GetOutput();
			
			// necessary to force resampler to make new pointer when updated
			transformedImage->DisconnectPipeline();
			
			// add new image to end of filter stack
			tileFilter->PushBackInput( transformedImage );
		}
		
		zScaler->SetInput( tileFilter->GetOutput() );
		zScaler->Update();
		volume = zScaler->GetOutput();
	}
	
	void buildMask() {
	  // construct tile filter
		maskTileFilter = MaskTileFilterType::New();
		maskTileFilter->SetLayout( layout );
		
		// build mask slices and attach them to the tile filter
		for(unsigned int i=0; i<originalImages.size(); i++)
		{
			// apply as in buildVolume
			maskResampler->SetInput( originalMasks[i] );
			maskResampler->SetTransform( transforms[i] );
			maskResampler->Update();
			
			MaskSliceType::Pointer transformedMask = maskResampler->GetOutput();
			
			// necessary to force resampler to make new pointer when updated
			transformedMask->DisconnectPipeline();
			
			// add new image to end of filter stack
			maskTileFilter->PushBackInput( transformedMask );
			
		}
		
		maskZScaler->SetInput( maskTileFilter->GetOutput() );
		maskZScaler->Update();
		maskVolume = maskZScaler->GetOutput();
		
		mask3D->SetImage( maskVolume );
	}
	
	void UpdateVolumes() {
    buildVolume();
    buildMask();
	}
	
	VolumeType::Pointer GetVolume() {
		return volume;
	}
	
	MaskVolumeType::Pointer GetMaskVolume() {
		return maskVolume;
	}
	
	MaskType3D::Pointer GetMask3D() {
		return mask3D;
	}
	
	MaskType2D::Pointer GetMask2D(unsigned int slice_number) {
		return masks2D[slice_number];
	}
	
	MaskSliceType::Pointer GetMaskSlice(unsigned int slice_number) {
    return originalMasks[slice_number];
	}
	
  TransformType::Pointer GetTransform(unsigned int slice_number) {
    return transforms[slice_number];
  }
	
protected:
};
#endif

// This object is built with some original images.
// Its job is to take transform parameters,
// then build a volume and an associated mask.

#ifndef VOLUME_HPP_
#define VOLUME_HPP_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkTileImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkChangeInformationImageFilter.h"
#include "itkImageSpatialObject.h"
#include "itkImageRegionIterator.h"

class Stack {
public:
  typedef short PixelType;
	typedef itk::Image< PixelType, 2 > SliceType;
	typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 2 > MaskSliceType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
	typedef vector< SliceType::Pointer > SliceVectorType;
	typedef itk::CenteredRigid2DTransform< double > TransformType;
	typedef TransformType::ParametersType ParametersType;
  typedef itk::LinearInterpolateImageFunction< SliceType, double > LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction< MaskSliceType, double > NearestNeighborInterpolatorType;
	typedef itk::ResampleImageFilter< SliceType, SliceType > ResamplerType;
	typedef itk::ResampleImageFilter< MaskSliceType, MaskSliceType > MaskResamplerType;
  typedef itk::TileImageFilter< SliceType, VolumeType > TileFilterType;
  typedef itk::TileImageFilter< MaskSliceType, MaskVolumeType > MaskTileFilterType;
  typedef itk::ChangeInformationImageFilter< VolumeType > ZScaleType;
  typedef itk::ChangeInformationImageFilter< MaskVolumeType > MaskZScaleType;
  typedef itk::ImageRegionIterator< MaskSliceType > IteratorType;
	
	SliceVectorType originalImages;
	SliceType::SizeType maxSize;
	SliceType::SpacingType spacings2D;
	VolumeType::Pointer volume;
	MaskVolumeType::Pointer maskVolume;
	vector< ParametersType > parameters;
	TransformType::Pointer transform;
	LinearInterpolatorType::Pointer linearInterpolator;
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator;
	ResamplerType::Pointer resampler;
	MaskResamplerType::Pointer maskResampler;
	TileFilterType::Pointer tileFilter;
	MaskTileFilterType::Pointer maskTileFilter;
	ZScaleType::Pointer zScaler;
	MaskZScaleType::Pointer maskZScaler;
	
	Stack(vector< string > fileNames) {
		typedef itk::ImageFileReader< SliceType > ReaderType;
		ReaderType::Pointer reader;
		
		for(unsigned int i=0; i<fileNames.size(); i++)
		{
			reader = ReaderType::New();
			reader->SetFileName( fileNames[i] );
			reader->Update();
			originalImages.push_back( reader->GetOutput() );
			originalImages.back()->DisconnectPipeline();
		}
		
		// initialise volume and mask
		initialiseParametersVector();
		calculateMaxSize();
		initialiseFilters();
		calculateCenteredTransformParameters();
		buildVolume();
		buildMask();
	}
	
	void initialiseParametersVector() {
		if( parameters.empty() )
		{
			for(unsigned int i=0; i<originalImages.size(); i++)
			{
				ParametersType p(5);
				p.Fill(0);
				parameters.push_back( p );
			}
		}
	}
	
	void calculateMaxSize() {
		// maxSize
		SliceType::SizeType size;
		maxSize.Fill(0);
		unsigned int dimension = size.GetSizeDimension();

		for(unsigned int i=0; i<originalImages.size(); i++)
		{
			size = originalImages[i]->GetLargestPossibleRegion().GetSize();

			for(unsigned int j=0; j<dimension; j++)
			{
				if(maxSize[j] < size[j]) { maxSize[j] = size[j]; }
			}
		}
		
		// spacings in 2D
		spacings2D = originalImages[0]->GetSpacing();
	}
	
	void initialiseFilters() {
		// resamplers
		transform = TransformType::New();
		linearInterpolator = LinearInterpolatorType::New();
		nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
		resampler = ResamplerType::New();
		resampler->SetInterpolator( linearInterpolator );
		resampler->SetSize( maxSize );
		resampler->SetOutputSpacing( originalImages[0]->GetSpacing() );
		resampler->SetTransform( transform );
		maskResampler = MaskResamplerType::New();
		maskResampler->SetInterpolator( nearestNeighborInterpolator );
		maskResampler->SetSize( maxSize );
		maskResampler->SetOutputSpacing( originalImages[0]->GetSpacing() );
		maskResampler->SetTransform( transform );
		
		// z scalers
		zScaler     = ZScaleType::New();
		maskZScaler = MaskZScaleType::New();
		zScaler    ->ChangeSpacingOn();
		maskZScaler->ChangeSpacingOn();
		ZScaleType::SpacingType spacings3D;
		spacings3D[0] = 70.4;
		spacings3D[1] = 70.4;
		spacings3D[2] = 160;
		zScaler    ->SetOutputSpacing( spacings3D );
		maskZScaler->SetOutputSpacing( spacings3D );
		
		// tile filters
		tileFilter = TileFilterType::New();
		maskTileFilter = MaskTileFilterType::New();
		TileFilterType::LayoutArrayType layout;
		layout[0] = 1;
	  layout[1] = 1;
	  layout[2] = 0;
		tileFilter    ->SetLayout( layout );
		maskTileFilter->SetLayout( layout );
	}
	
	void calculateCenteredTransformParameters() {
		SliceType::SizeType size;
				
    for(unsigned int i=0; i<originalImages.size(); i++)
		{
			size = originalImages[i]->GetLargestPossibleRegion().GetSize();
			parameters[i][0] = 0;
			parameters[i][1] = spacings2D[0] * maxSize[0] / 2.0;
			parameters[i][2] = spacings2D[1] * maxSize[1] / 2.0;
			parameters[i][3] = - spacings2D[0] * ( maxSize[0] - size[0] ) / 2.0;
			parameters[i][4] = - spacings2D[1] * ( maxSize[1] - size[1] ) / 2.0;
		}
		
	}
	
	void buildVolume() {
    for(unsigned int i=0; i<originalImages.size(); i++)
		{
			// resample transformed image
			resampler->SetInput( originalImages[i] );
			transform->SetParameters( parameters[i] );
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
		// build mask slices and attach them to the tile filter
		for(unsigned int i=0; i<originalImages.size(); i++)
		{
			// make new maskSlice and make it all white
			MaskSliceType::RegionType region;
			region.SetSize( originalImages[i]->GetLargestPossibleRegion().GetSize() );
			
			MaskSliceType::Pointer maskSlice = MaskSliceType::New();
			maskSlice->SetRegions( region );
			maskSlice->CopyInformation( originalImages[i] );
			maskSlice->GetMetaDataDictionary().Print( cout );
		  maskSlice->Allocate();
			
		  IteratorType it(maskSlice,region);
		  for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
		    it.Set( 255 );
		  }
			
			// apply as in buildVolume
			maskResampler->SetInput( maskSlice );
			transform->SetParameters( parameters[i] );
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
	}
	
	VolumeType::Pointer GetVolume() {
		return volume;
	}
	
	MaskVolumeType::Pointer GetMaskVolume() {
		return maskVolume;
	}
	
	// void SetTransformParams() {}
	
protected:
};
#endif

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
#include "itkChangeInformationImageFilter.h"

class Stack {
  typedef unsigned short PixelType;
	typedef itk::Image< PixelType, 2 > SliceType;
	typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 3 > MaskType;
	typedef vector< SliceType::Pointer > SliceVectorType;
	typedef itk::CenteredRigid2DTransform< double > TransformType;
	typedef TransformType::ParametersType ParametersType;
  typedef itk::LinearInterpolateImageFunction< SliceType, double > InterpolatorType;
	typedef itk::ResampleImageFilter< SliceType, SliceType > ResamplerType;
  typedef itk::TileImageFilter< SliceType, VolumeType > TileFilterType;
  typedef itk::ChangeInformationImageFilter< VolumeType > ZScaleType;

public:
	SliceVectorType originalImages;
	SliceType::SizeType maxSize;
	VolumeType::Pointer volume;
	MaskType::Pointer mask;
	vector< ParametersType > parameters;
	TransformType::Pointer transform;
	InterpolatorType::Pointer interpolator;
	ResamplerType::Pointer resampler;
	ZScaleType::Pointer zScaler;
		
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
					
		// initialise transform
		initialiseParametersVector();
		calculateMaxSize();
		initialiseFilters();
		calculateCenteredTransformParameters();
		buildVolume();
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
		
	}
	
	void initialiseFilters() {
		// resampler
		transform = TransformType::New();
		interpolator = InterpolatorType::New();
		resampler = ResamplerType::New();
		resampler->SetInterpolator( interpolator );
		resampler->SetSize( maxSize );
		resampler->SetOutputSpacing( originalImages[0]->GetSpacing() );
		resampler->SetTransform( transform );
		
		// z scaler
		zScaler = ZScaleType::New();
		zScaler->ChangeSpacingOn();
		ZScaleType::SpacingType spacings;
		spacings[0] = 128;
		spacings[1] = 128;
		spacings[2] = 128;
		zScaler->SetOutputSpacing( spacings );
	}
	
	void calculateCenteredTransformParameters() {
		SliceType::SizeType size;
		SliceType::SpacingType spacings = originalImages[0]->GetSpacing();
    for(unsigned int i=0; i<originalImages.size(); i++)
		{
			size = originalImages[i]->GetLargestPossibleRegion().GetSize();
			parameters[i][3] = - spacings[0] * ( maxSize[0] - size[0] ) / 2.0;
			parameters[i][4] = - spacings[1] * ( maxSize[1] - size[1] ) / 2.0;
		}		
		
	}
	
	TileFilterType::Pointer initialiseTileFilter() {
		TileFilterType::Pointer tileFilter = TileFilterType::New();
		
		TileFilterType::LayoutArrayType layout;
		layout[0] = 1;
	  layout[1] = 1;
	  layout[2] = 0;
		tileFilter->SetLayout( layout );
		
		return(tileFilter);
	}
	
	void buildVolume() {
		TileFilterType::Pointer tileFilter = initialiseTileFilter();
		SliceType::Pointer transformedImage;
		
    for(unsigned int i=0; i<originalImages.size(); i++)
		{
			// resample transformed image
			resampler->SetInput( originalImages[i] );
			transform->SetParameters( parameters[i] );
			resampler->Update();
			transformedImage = resampler->GetOutput();
			
			// necessary to force resampler to make new pointer when updated
			transformedImage->DisconnectPipeline();
			
			// add new image to end of filter stack
			tileFilter->PushBackInput( transformedImage );
		}
		
		zScaler->SetInput( tileFilter->GetOutput() );
		zScaler->Update();
		
	}
	
	void buildMask() {
		
	}
	
	VolumeType::Pointer GetVolume() {
		return zScaler->GetOutput();
	}
	
	// void SetTransformParams() {}
	
protected:
};
#endif

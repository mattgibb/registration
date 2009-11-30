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
#include "itkImageSpatialObject.h"
#include "itkImageRegionIteratorWithIndex.h"

class Stack {
public:
  typedef unsigned short PixelType;
	typedef itk::Image< PixelType, 2 > SliceType;
	typedef itk::Image< PixelType, 3 > VolumeType;
	typedef itk::Image< unsigned char, 2 > MaskSliceType;
	typedef itk::Image< unsigned char, 3 > MaskVolumeType;
	typedef vector< SliceType::Pointer > SliceVectorType;
	typedef itk::CenteredRigid2DTransform< double > TransformType;
	typedef TransformType::ParametersType ParametersType;
  typedef itk::LinearInterpolateImageFunction< SliceType, double > InterpolatorType;
	typedef itk::ResampleImageFilter< SliceType, SliceType > ResamplerType;
  typedef itk::TileImageFilter< SliceType, VolumeType > TileFilterType;
  typedef itk::TileImageFilter< MaskSliceType, MaskVolumeType > MaskTileFilterType;
  typedef itk::ChangeInformationImageFilter< VolumeType > ZScaleType;
  typedef itk::ChangeInformationImageFilter< MaskVolumeType > MaskZScaleType;
	typedef itk::ImageSpatialObject< 2, PixelType > ImageSpatialObjectType;
  typedef itk::ImageRegionIteratorWithIndex< MaskSliceType > Iterator;

	SliceVectorType originalImages;
	SliceType::SizeType maxSize;
	SliceType::SpacingType spacings;
	VolumeType::Pointer volume;
	MaskVolumeType::Pointer maskVolume;
	vector< ParametersType > parameters;
	TransformType::Pointer transform;
	InterpolatorType::Pointer interpolator;
	ResamplerType::Pointer resampler;
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
					
		// initialise transform
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
		
		// spacings
		spacings = originalImages[0]->GetSpacing();
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
		
		// z scalers
		zScaler     = ZScaleType::New();
		maskZScaler = MaskZScaleType::New();
		zScaler    ->ChangeSpacingOn();
		maskZScaler->ChangeSpacingOn();
		ZScaleType::SpacingType spacings;
		spacings[0] = 128;
		spacings[1] = 128;
		spacings[2] = 128;
		zScaler    ->SetOutputSpacing( spacings );
		maskZScaler->SetOutputSpacing( spacings );
		
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
			parameters[i][3] = - spacings[0] * ( maxSize[0] - size[0] ) / 2.0;
			parameters[i][4] = - spacings[1] * ( maxSize[1] - size[1] ) / 2.0;
			cout << "parameters[" << i << "] = " << parameters[i] << endl;
		}		
		
	}
		
	void buildVolume() {
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
		volume = zScaler->GetOutput();
	}
	
	void buildMask() {
	  ImageSpatialObjectType::Pointer imageSO = ImageSpatialObjectType::New();
	  ImageSpatialObjectType::TransformType::Pointer spatialObjectTransform = imageSO->GetObjectToParentTransform();
		MaskSliceType::Pointer maskSlice;
		MaskSliceType::RegionType region;
		region.SetSize( maxSize );
				
		// build mask slices and attach them to the tile filter
		for(unsigned int i=0; i<originalImages.size(); i++)
		{
			// initialise new 2D mask slice
		  maskSlice = MaskSliceType::New();
			maskSlice->SetRegions( region );
			maskSlice->CopyInformation( originalImages[i] );
			maskSlice->GetMetaDataDictionary().Print( cout );
		  maskSlice->Allocate();
			
			// set up spatial object
			transform = TransformType::New();
			transform->SetParameters( parameters[i] );
			spatialObjectTransform->SetIdentity();
			spatialObjectTransform->SetCenter( transform->GetCenter() );
			//TEMP
			TransformType::OutputVectorType dummy = transform->GetOffset();
			dummy = -dummy;
			//TEMP
			spatialObjectTransform->SetOffset( dummy );
			// spatialObjectTransform->SetTranslation( transform->GetTranslation() );
			// spatialObjectTransform->SetMatrix( transform->GetInverseMatrix() );
			imageSO->ComputeObjectToWorldTransform();
		  imageSO->SetImage( originalImages[i] );
			
			//TEMP
			ImageSpatialObjectType::BoundingBoxType::Pointer boundingBox = imageSO->GetBoundingBox();			
			cout << "boundingBox = " << boundingBox->GetBounds() << ", ";
			cout << "boundingBox center = " << boundingBox->GetCenter() << endl;			
			//TEMP
			
			// fill in the mask slice with whether its points are inside imageSO
		  Iterator it(maskSlice,region);
		  for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
				MaskSliceType::IndexType index = it.GetIndex();
				SliceType::PointType position;
				position[0] = index[0] * spacings[0];
				position[1] = index[1] * spacings[1];
		    it.Set( imageSO->IsInside( position ) ? 255 : 0 );
		  }
			
			// add new image to end of filter stack
			maskTileFilter->PushBackInput( maskSlice );
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

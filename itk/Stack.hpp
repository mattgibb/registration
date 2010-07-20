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

#include <sys/stat.h> // for fileExists


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
	
protected:
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
	
public:
	Stack(const vector< string >& fileNames, VolumeType::SpacingType inputSpacings):
	spacings(inputSpacings) {
    readImages(fileNames);
	  
		// scale slices and initialise volume and mask
    offset.Fill(0);
    resamplerSize.Fill(0);
    scaleOriginalSlices();
    buildOriginalMaskSlices();
    calculateMaxSize();
    setResamplerSizeToMaxSize();
    initialiseFilters();
    initializeCenteredTransforms();
    updateVolumes();
  }
	
	// constructor to specify size and offset
	Stack(const vector< string >& fileNames, VolumeType::SpacingType inputSpacings, const SliceType::SizeType& inputSize, const SliceType::SizeType& inputOffset):
	resamplerSize(inputSize),
  offset(inputOffset),
	spacings(inputSpacings) {
    readImages(fileNames);
	  
		// scale slices and initialise volume and mask
    scaleOriginalSlices();
    buildOriginalMaskSlices();
    calculateMaxSize();
    initialiseFilters();
    initializeCenteredTransforms();
    updateVolumes();
  }
	
protected:
  void readImages(const vector< string >& fileNames) {
    originalImages.clear();
    ReaderType::Pointer reader;
		
		for(unsigned int i=0; i<fileNames.size(); i++) {
		  if( fileExists(fileNames[i]) ) {
  			reader = ReaderType::New();
  			reader->SetFileName( fileNames[i] );
  			reader->Update();
  			originalImages.push_back( reader->GetOutput() );
  			originalImages.back()->DisconnectPipeline();
		  }
		  else
		  {
		    // create a new image of zero size
        originalImages.push_back( SliceType::New() );
		  }
		}
  }
  
	void scaleOriginalSlices() {
    SliceType::SpacingType spacings2D;
		for(unsigned int i=0; i<2; i++) {
      spacings2D[i] = spacings[i];
	  }
	  
	  // rescale original images
		for(unsigned int i=0; i<originalImages.size(); i++) {
		  xyScaler = XYScaleType::New();
  		xyScaler->ChangeSpacingOn();
  		xyScaler->SetOutputSpacing( spacings2D );
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
		  
      // make new 2D masks and assign mask slices to them
      original2DMasks.push_back( MaskType2D::New() );
      original2DMasks.back()->SetImage( maskSlice );
      
	  }
	}
	
	void calculateMaxSize() {
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
	
	void setResamplerSizeToMaxSize() {
    for(unsigned int i=0; i<2; i++) {
      resamplerSize[i] = maxSize[i];
    }
	}
  
	
	void initialiseFilters() {
		// resamplers
		linearInterpolator = LinearInterpolatorType::New();
		nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
		resampler = ResamplerType::New();
		resampler->SetInterpolator( linearInterpolator );
		resampler->SetSize( resamplerSize );
		resampler->SetOutputSpacing( originalImages[0]->GetSpacing() );
		maskResampler = MaskResamplerType::New();
		maskResampler->SetInterpolator( nearestNeighborInterpolator );
		maskResampler->SetSize( resamplerSize );
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
			
			// rotation in radians
			parameters[0] = 0;
			// translation, applied after rotation.
			parameters[3] = (long)offset[0] - spacings[0] * ( (long)maxSize[0] - (long)size[0] ) / 2.0;
			parameters[4] = (long)offset[1] - spacings[1] * ( (long)maxSize[1] - (long)size[1] ) / 2.0;			
			// centre of rotation, before translation is applied.
			parameters[1] = parameters[3] + ( spacings[0] * resamplerSize[0] ) / 2.0;
			parameters[2] = parameters[4] + ( spacings[1] * resamplerSize[1] ) / 2.0;
			
			
			// set them to new transform
      transforms.push_back( TransformType::New() );
      transforms.back()->SetParameters( parameters );
		}
		
	}

public:	
	void updateVolumes() {
    buildSlices();
    buildVolume();
    buildMaskSlices();
    buildMaskVolume();
	}
	
protected:
	void buildSlices() {
    slices.clear();
    
	  for(unsigned int i=0; i<originalImages.size(); i++) {
			// resample transformed image
			resampler->SetInput( originalImages[i] );
			resampler->SetTransform( transforms[i] );
			resampler->Update();
      slices.push_back( resampler->GetOutput() );
			
			// necessary to force resampler to make new pointer when updated
			slices.back()->DisconnectPipeline();
		}
	
	}
	
	void buildVolume() {
	  // construct tile filter
		tileFilter = TileFilterType::New();
		tileFilter->SetLayout( layout );
		
		// add new images to filter stack
    for(SliceVectorType::iterator it = slices.begin(); it != slices.end(); it++) {
			tileFilter->PushBackInput( *it );
		}
		
		zScaler->SetInput( tileFilter->GetOutput() );
		zScaler->Update();
		volume = zScaler->GetOutput();
	}
	
	void buildMaskSlices() {
    resampled2DMasks.clear();
	  
		// make new 2D masks and assign mask slices to them      
		for(unsigned int i=0; i<originalImages.size(); i++)
		{
		  // generate mask slice
			maskResampler->SetInput( original2DMasks[i]->GetImage() );
			maskResampler->SetTransform( transforms[i] );
			maskResampler->Update();
			
			// append mask to mask vector
      resampled2DMasks.push_back( MaskType2D::New() );
      resampled2DMasks.back()->SetImage( maskResampler->GetOutput() );
			// necessary to force resampler to make new pointer when updated
      maskResampler->GetOutput()->DisconnectPipeline();
		}
		
	}
	
	void buildMaskVolume() {
	  // construct tile filter
		maskTileFilter = MaskTileFilterType::New();
		maskTileFilter->SetLayout( layout );
		
    // append mask slices to the tile filter
		for(unsigned int i=0; i<originalImages.size(); i++)
		{
      maskTileFilter->PushBackInput( resampled2DMasks[i]->GetImage() );
		}
		
		maskZScaler->SetInput( maskTileFilter->GetOutput() );
		maskZScaler->Update();		
		mask3D->SetImage( maskZScaler->GetOutput() );
	}
	
	void checkSliceNumber(unsigned int slice_number) {
	  if( slice_number >= this->GetSize() ) {
      cerr << "Wait a minute, trying to access slice number bigger than this stack!" << endl;
      exit(EXIT_FAILURE);
	  }
	}
	
public:
	// Getter methods
	unsigned short GetSize() {
	  return originalImages.size();
  }
	
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
  
	VolumeType::Pointer GetVolume() {
    return volume;
	}
		
	MaskType3D::Pointer Get3DMask() {
    return mask3D;
	}
	
  TransformType::Pointer GetTransform(unsigned int slice_number) {
    checkSliceNumber(slice_number);
    return transforms[slice_number];
  }
	
protected:
  static bool fileExists(const string& strFilename) { 
    struct stat stFileInfo; 
    bool blnReturn; 
    int intStat; 

    // Attempt to get the file attributes 
    intStat = stat(strFilename.c_str(),&stFileInfo); 
    if(intStat == 0) { 
      // We were able to get the file attributes 
      // so the file obviously exists. 
      blnReturn = true; 
    } else { 
      // We were not able to get the file attributes. 
      // This may mean that we don't have permission to 
      // access the folder which contains this file. If you 
      // need to do that level of checking, lookup the 
      // return values of stat which will give you 
      // more details on why stat failed. 
      blnReturn = false; 
    } 

    return(blnReturn); 
  }
  
};
#endif

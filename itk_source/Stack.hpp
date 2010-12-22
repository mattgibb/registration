// This object is built with some original images.
// Its job is to take transform parameters,
// then build a volume and an associated mask.

#ifndef STACK_HPP_
#define STACK_HPP_

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

#include <sys/stat.h> // for fileExists


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
	Stack(const vector< string >& inputFileNames, VolumeType::SpacingType inputSpacings):
	fileNames(inputFileNames),
  spacings(inputSpacings) {
    readImages();
    initializeVectors();
		// scale slices and initialise volume and mask
    offset.Fill(0);
    resamplerSize.Fill(0);
    scaleOriginalSlices();
    buildOriginalMaskSlices();
    
    calculateMaxSize();
    setResamplerSizeToMaxSize();
    initializeFilters();
  }
	
	// constructor to specify size and offset
	Stack(const vector< string >& inputFileNames, VolumeType::SpacingType inputSpacings, const SliceType::SizeType& inputSize, const SliceType::SizeType& inputOffset):
	fileNames(inputFileNames),
	resamplerSize(inputSize),
  offset(inputOffset),
	spacings(inputSpacings) {
    readImages();
    initializeVectors();
		// scale slices and initialise volume and mask
    scaleOriginalSlices();
    buildOriginalMaskSlices();
    calculateMaxSize();
    initializeFilters();
  }
	
protected:
  void readImages() {
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
  
  void initializeVectors() {
		// initialise various data members once the number of images is available
		numberOfTimesTooBig = vector< unsigned int >( GetSize(), 0 );
    for(unsigned int slice_number = 0; slice_number < GetSize(); slice_number++) {
      slices.push_back( SliceType::New() );
      original2DMasks.push_back( MaskType2D::New() );
      resampled2DMasks.push_back( MaskType2D::New() );
    }
    
  }
  
	void scaleOriginalSlices() {
    SliceType::SpacingType spacings2D;
		for(unsigned int i=0; i<2; i++) {
      spacings2D[i] = spacings[i];
	  }
	  
	  // rescale original images
		for(unsigned int slice_number=0; slice_number<originalImages.size(); slice_number++) {
		  xyScaler = XYScaleType::New();
  		xyScaler->ChangeSpacingOn();
  		xyScaler->SetOutputSpacing( spacings2D );
      xyScaler->SetInput( originalImages[slice_number] );
      xyScaler->Update();
      originalImages[slice_number] = xyScaler->GetOutput();
      originalImages[slice_number]->DisconnectPipeline();
		}
	}
	
	void buildOriginalMaskSlices() {
		// build a vector of mask slices
		for(unsigned int slice_number=0; slice_number<originalImages.size(); slice_number++) {
			// make new maskSlice and make it all white
			MaskSliceType::RegionType region;
			region.SetSize( originalImages[slice_number]->GetLargestPossibleRegion().GetSize() );
		  
			MaskSliceType::Pointer maskSlice = MaskSliceType::New();
			maskSlice->SetRegions( region );
			maskSlice->CopyInformation( originalImages[slice_number] );
		  maskSlice->Allocate();
			maskSlice->FillBuffer( 255 );
		  
      // assign mask slices to masks
      original2DMasks[slice_number]->SetImage( maskSlice );
      
	  }
	}
	
	void calculateMaxSize() {
		SliceType::SizeType size;
		maxSize.Fill(0);
		unsigned int dimension = size.GetSizeDimension();
    
		for(unsigned int slice_number=0; slice_number<originalImages.size(); slice_number++) {
			size = originalImages[slice_number]->GetLargestPossibleRegion().GetSize();

			for(unsigned int i=0; i<dimension; i++)
			{
				if(maxSize[i] < size[i]) { maxSize[i] = size[i]; }
			}
		}
	}
	
	void setResamplerSizeToMaxSize() {
    for(unsigned int i=0; i<2; i++) {
      resamplerSize[i] = maxSize[i];
    }
	}
  
	void initializeFilters() {
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
	
public:	
	void updateVolumes() {
    buildSlices();
    buildVolume();
    buildMaskSlices();
    buildMaskVolume();
	}
	
protected:
	void buildSlices() {
	  for(unsigned int slice_number=0; slice_number<originalImages.size(); slice_number++) {
			// resample transformed image
			resampler->SetInput( originalImages[slice_number] );
			resampler->SetTransform( transforms[slice_number] );
			resampler->Update();
      slices[slice_number] = resampler->GetOutput();
      
      // TEMP
      resampler->SetSize(    originalImages[slice_number]->GetLargestPossibleRegion().GetSize() );
      resampler->SetOutputOrigin(  originalImages[slice_number]->GetOrigin() );
      resampler->SetOutputSpacing( originalImages[slice_number]->GetSpacing() );
      resampler->SetOutputDirection( originalImages[slice_number]->GetDirection() );
      // TEMP
      
      // REMOVE
      // finalTransform->SetParameters( finalParameters );
      // finalTransform->SetFixedParameters( transform->GetFixedParameters() );
      // REMOVE
      
			
			// necessary to force resampler to make new pointer when updated
			slices[slice_number]->DisconnectPipeline();
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
		// make new 2D masks and assign mask slices to them      
		for(unsigned int slice_number=0; slice_number<GetSize(); slice_number++)
		{
      buildMaskSlice(slice_number);
		}
		
	}
	
	void buildMaskSlice(unsigned int slice_number) {
	  // generate mask slice
		maskResampler->SetInput( original2DMasks[slice_number]->GetImage() );
		maskResampler->SetTransform( transforms[slice_number] );
		maskResampler->Update();
		
		// append mask to mask vector
    resampled2DMasks[slice_number]->SetImage( maskResampler->GetOutput() );
		// necessary to force resampler to make new pointer when updated
    maskResampler->GetOutput()->DisconnectPipeline();
    
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
	
	void checkSliceNumber(unsigned int slice_number) const {
	  if( slice_number >= this->GetSize() ) {
      cerr << "Wait a minute, trying to access slice number bigger than this stack!" << endl;
      exit(EXIT_FAILURE);
	  }
	}
	
public:
  // Getter methods
    const string& GetFileName(unsigned int slice_number) const {
      checkSliceNumber(slice_number);
      return fileNames[slice_number];
    }
    
    unsigned short GetSize() const {
      return originalImages.size();
    }
  
  const SliceType::SizeType& GetMaxSize() const {
    return maxSize;
  }

  const SliceType::SizeType& GetResamplerSize() const {
    return resamplerSize;
  }
  
  const SliceType::SizeType& GetOffset() const {
    return offset;
  }
  
  const VolumeType::SpacingType& GetSpacings() const {
    return spacings;
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
	
  const TransformVectorType& GetTransforms() const {
      return transforms;
  }
	
  void SetTransforms(const TransformVectorType& inputTransforms) {
    transforms = inputTransforms;
  }
  
  bool ImageExists(unsigned int slice_number) {
    return GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0];
  }
  
  void ShrinkSliceMask(unsigned int slice_number) {
    // increment numberOfTimesTooBig
    numberOfTimesTooBig[slice_number]++;
    
    // initialise and allocate new mask image
    MaskSliceType::ConstPointer oldMaskSlice = original2DMasks[slice_number]->GetImage();
    MaskSliceType::RegionType region = oldMaskSlice->GetLargestPossibleRegion();
    MaskSliceType::Pointer newMaskSlice = MaskSliceType::New();
    newMaskSlice->SetRegions( region );
    newMaskSlice->CopyInformation( oldMaskSlice );
    newMaskSlice->Allocate();
    
    // make smaller image region
    MaskSliceType::RegionType::SizeType size;
    MaskSliceType::RegionType::IndexType index;
    unsigned int factor = pow(2.0, (int)numberOfTimesTooBig[slice_number]);
    
    for(unsigned int i=0; i<2; i++) {
      size[i] = resamplerSize[i] / factor;
      index[i] = (offset[i] / spacings[i]) + resamplerSize[i] * ( 1.0 - ( 1.0/factor ) ) / 2;
    }
    
    region.SetSize( size );
    region.SetIndex( index );
    
    // set pixels outside region to black and inside region to white
    newMaskSlice->FillBuffer( 0 );
    MaskSliceIteratorType it(newMaskSlice, region);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it ) {
      it.Set(255);
    }
	  
	  // attach new image to mask
    original2DMasks[slice_number]->SetImage( newMaskSlice );
    buildMaskSlice(slice_number);
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
  
private:
  // Copy constructor and copy assignment operator deliberately not implemented
  // Made private so that nobody can use them
  Stack(const Stack&);
  Stack& operator=(const Stack&);

};
#endif

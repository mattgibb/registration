// This object constructs and encapsulates the rat registration framework
// of HiRes to LoRes images.

#ifndef FRAMEWORK2DRAT_HPP_
#define FRAMEWORK2DRAT_HPP_

// my files
#include "Stack.hpp"
#include "Framework2DBase.hpp"

// TEMP
#include "helper_functions.hpp"
// TEMP


class Framework2DRat : public Framework2DBase {
public:
	
	Stack *LoResStack, *HiResStack;
	
	Framework2DRat(Stack *LoRes, Stack *HiRes, YAML::Node &parameters):
	Framework2DBase(parameters),
	LoResStack(LoRes),
	HiResStack(HiRes) {
	}
	
	void StartRegistration(const string& outputFileName) {
    observerOutput.open( outputFileName.c_str() );
    
    unsigned int number_of_slices = LoResStack->GetSize();
    
    for(unsigned int slice_number=0; slice_number < number_of_slices; slice_number++) {
      cout << "size = " << LoResStack->GetSize();
      
      cout << "LoResSize: " << LoResStack->GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize() << endl
           << "HiResSize: " << HiResStack->GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize() << endl;
      cout << "LoResSpacing: " << LoResStack->GetOriginalImage(slice_number)->GetSpacing() << endl
           << "HiResSpacing: " << HiResStack->GetOriginalImage(slice_number)->GetSpacing() << endl;
      
      cout << "LoResMaskSize: " << LoResStack->GetOriginal2DMask(slice_number)->GetImage()->GetLargestPossibleRegion().GetSize() << endl
           << "HiResMaskSize: " << HiResStack->GetOriginal2DMask(slice_number)->GetImage()->GetLargestPossibleRegion().GetSize() << endl;
      cout << "LoResMaskSpacing: " << LoResStack->GetOriginal2DMask(slice_number)->GetImage()->GetSpacing() << endl
           << "HiResMaskSpacing: " << HiResStack->GetOriginal2DMask(slice_number)->GetImage()->GetSpacing() << endl;
      
      bool bothImagesExist = (LoResStack->GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0] &&
                              HiResStack->GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0] );
      
    	writeImage< Stack::SliceType >( LoResStack->GetResampledSlice(slice_number), "results/Rat24/LoRes/downsamples_8/deleteme/FirstLoResStackSlice.mhd" );
      writeImage< Stack::MaskSliceType >( LoResStack->GetResampled2DMask(slice_number)->GetImage(), "results/Rat24/LoRes/downsamples_8/deleteme/FirstLoResStackMaskSlice.mhd" );
    	writeImage< Stack::SliceType >( HiResStack->GetOriginalImage(slice_number), "results/Rat24/LoRes/downsamples_8/deleteme/FirstHiResStackSlice.mhd" );
      writeImage< Stack::MaskSliceType >( HiResStack->GetOriginal2DMask(slice_number)->GetImage(), "results/Rat24/LoRes/downsamples_8/deleteme/FirstHiResStackMaskSlice.mhd" );
      
      // exit(0);
      
      if( bothImagesExist ) {
        cout << "slice number: " << slice_number << endl;
        registration->SetFixedImage( LoResStack->GetResampledSlice(slice_number) );
        cout << "after SetFixedImage...\n";
        registration->SetMovingImage( HiResStack->GetOriginalImage(slice_number) );
        
        // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
        // registration->SetFixedImageRegion( LoResStack->originalImages[slice_number]->GetLargestPossibleRegion() );
        // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
      
        metric->SetFixedImageMask( LoResStack->GetResampled2DMask(slice_number) );
        metric->SetMovingImageMask( HiResStack->GetOriginal2DMask(slice_number) );

        registration->SetTransform( HiResStack->GetTransform(slice_number) );
        registration->SetInitialTransformParameters( HiResStack->GetTransform(slice_number)->GetParameters() );
        
        cout << "just before try...\n";
        try {
          registration->StartRegistration();
          cout << "Optimizer stop condition: "
               << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
        }
        catch( itk::ExceptionObject & err ) {
          std::cerr << "ExceptionObject caught !" << std::endl;
          std::cerr << err << std::endl;
          exit(EXIT_FAILURE);
        }
        
      }
    }
    
    observerOutput.close();
	}
	
protected:
};
#endif

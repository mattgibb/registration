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
	
	explicit Framework2DRat(Stack *LoRes, Stack *HiRes, YAML::Node &parameters):
	Framework2DBase(parameters),
	LoResStack(LoRes),
	HiResStack(HiRes) {
	}
	
	void StartRegistration(const string& outputFileName) {
    observerOutput.open( outputFileName.c_str() );
    
    unsigned int number_of_slices = LoResStack->GetSize();
            
    for(unsigned int slice_number=0; slice_number < number_of_slices; slice_number++) {
      cout << "slice_number: " << slice_number << endl;
      
      bool bothImagesExist = (LoResStack->GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0] &&
                              HiResStack->GetOriginalImage(slice_number)->GetLargestPossibleRegion().GetSize()[0] );
            
      if( bothImagesExist ) {
        cout << "Performing registration...";
        registration->SetFixedImage( LoResStack->GetResampledSlice(slice_number) );
        registration->SetMovingImage( HiResStack->GetOriginalImage(slice_number) );
        
        // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
        // registration->SetFixedImageRegion( LoResStack->originalImages[slice_number]->GetLargestPossibleRegion() );
        // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
        
        metric->SetFixedImageMask( LoResStack->GetResampled2DMask(slice_number) );
        metric->SetMovingImageMask( HiResStack->GetOriginal2DMask(slice_number) );

        registration->SetTransform( HiResStack->GetTransform(slice_number) );
        registration->SetInitialTransformParameters( HiResStack->GetTransform(slice_number)->GetParameters() );
        
        try {
          registration->StartRegistration();
          cout << "Optimizer stop condition: "
               << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
        }
        catch( itk::ExceptionObject & err ) {
          cerr << "ExceptionObject caught !" << endl;
          cerr << err << endl;
          cerr << err.GetNameOfClass() << endl;
          exit(EXIT_FAILURE);
        }
        
        cout << "done!" << endl;
      }
    }
    
    observerOutput.close();
	}
	
protected:
};
#endif

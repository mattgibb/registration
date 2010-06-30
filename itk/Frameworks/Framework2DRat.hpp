// This object constructs and encapsulates the rat registration framework
// of HiRes to LoRes images.

#ifndef FRAMEWORK2DRAT_HPP_
#define FRAMEWORK2DRAT_HPP_

// my files
#include "Stack.hpp"
#include "Framework2DBase.hpp"


class Framework2DRat : public Framework2DBase {
public:
	
	Stack *LoResStack, *HiResStack;
	
	Framework2DRat(Stack *LoRes, Stack *HiRes, YAML::Node &parameters):
	Framework2DBase(parameters),
	LoResStack(LoResStack),
	HiResStack(HiResStack) {
	}
	
	void StartRegistration(const string& outputFileName) {
    // observerOutput.open( outputFileName.c_str() );
    // 
    // unsigned int number_of_slices = stack->originalImages.size();
    //     
    //     for(unsigned int slice_number=0; slice_number < number_of_slices; slice_number++) {
    //      registration->SetFixedImage( mriVolume->GetResampledSlice(slice_number) );
    //      registration->SetMovingImage( stack->originalImages[slice_number] );
    //      
    //      // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
    //      registration->SetFixedImageRegion( mriVolume->GetResampledSlice(slice_number)->GetLargestPossibleRegion() );
    //      // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
    //      
    //      metric->SetFixedImageMask( mriVolume->GetMask2D(slice_number) );
    //      metric->SetMovingImageMask( stack->GetMask2D(slice_number) );
    //      
    //      registration->SetTransform( stack->GetTransform(slice_number) );
    //      registration->SetInitialTransformParameters( stack->GetTransform(slice_number)->GetParameters() );
    //      
    //      try
    //        {
    //        registration->StartRegistration();
    //        cout << "Optimizer stop condition: "
    //             << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
    //        }
    //      catch( itk::ExceptionObject & err )
    //        {
    //        std::cerr << "ExceptionObject caught !" << std::endl;
    //        std::cerr << err << std::endl;
    //        exit(EXIT_FAILURE);
    //        }
    //     }
    //     
    //    observerOutput.close();
	}
	
protected:
};
#endif

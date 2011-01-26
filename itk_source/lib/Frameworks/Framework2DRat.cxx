#ifndef __FRAMEWORK2DRAT_CXX_
#define __FRAMEWORK2DRAT_CXX_

#include "Framework2DRat.hpp"


Framework2DRat::Framework2DRat(Stack &LoRes, Stack &HiRes): Framework2DBase(),
                                                            LoResStack(LoRes),
                                                            HiResStack(HiRes) {}


void Framework2DRat::StartRegistration() {
  unsigned int number_of_slices = LoResStack.GetSize();
  
  for(unsigned int slice_number=0; slice_number < number_of_slices; slice_number++) {
    cout << "slice number: " << slice_number << endl;
    cout << "file name: " << LoResStack.GetFileName(slice_number) << endl;
    
    if( bothImagesExist(slice_number) ) {
      // Could change this to register against original fixed image and fixed image masks,
      // by applying the inverse fixed transform to the moving one, registering, then
      // applying the fixed transform back again afterwards.
      registration->SetFixedImage( LoResStack.GetResampledSlice(slice_number) );
      registration->SetMovingImage( HiResStack.GetOriginalImage(slice_number) );
      
      // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
      // registration->SetFixedImageRegion( LoResStack.GetOriginalImage(slice_number)->GetLargestPossibleRegion() );
      // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
      
      // metric->SetFixedImageMask( LoResStack.GetResampled2DMask(slice_number) );
      // metric->SetMovingImageMask( HiResStack.GetOriginal2DMask(slice_number) );
      
      registration->SetTransform( HiResStack.GetTransform(slice_number) );
      registration->SetInitialTransformParameters( HiResStack.GetTransform(slice_number)->GetParameters() );
      
      // halve the width and height of the LoRes mask for each slice
      // until optimiser stops throwing errors
      cout << "Trying registration..." << endl;
      
      while( !tryRegistration() ) {
        LoResStack.ShrinkMaskSlice(slice_number);
      }
    }
  }
  
  cout << "Finished registration." << endl;
}

bool Framework2DRat::bothImagesExist(unsigned int slice_number) {
  return (LoResStack.ImageExists(slice_number) &&
          HiResStack.ImageExists(slice_number) );
}

bool Framework2DRat::tryRegistration() {
  try {
    cout << "Before registration->Update()" << endl;
    registration->Update();
    cout << "After registration->Update()" << endl;    
    cout << "Optimizer stop condition: "
         << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
    return true;
  }
  catch( itk::ExceptionObject & err ) {
    cerr << err.GetNameOfClass() << " caught, halving block image width and height." << endl;
    cerr << err << endl;
    return false;
  }
}


#endif

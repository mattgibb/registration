#ifndef __STACKALIGNER_CXX_
#define __STACKALIGNER_CXX_

#include "StackAligner.hpp"


template <typename StackType>
StackAligner< StackType >::StackAligner(StackType &LoResStack,
                           StackType &HiResStack,
                           typename RegistrationType::Pointer registration):
                           m_LoResStack(LoResStack),
                           m_HiResStack(HiResStack),
                           m_registration(registration)
                           {}

template <typename StackType>
void StackAligner< StackType >::Update() {
  unsigned int number_of_slices = m_LoResStack.GetSize();
  
  for(unsigned int slice_number=0; slice_number < number_of_slices; slice_number++) {
    cout << "slice number: " << slice_number << endl;
    
    if( bothImagesExist(slice_number) ) {
      // Could change this to register against original fixed image and fixed image masks,
      // by applying the inverse fixed transform to the moving one, registering, then
      // applying the fixed transform back again afterwards.
      m_registration->SetFixedImage( m_LoResStack.GetResampledSlice(slice_number) );
      m_registration->SetMovingImage( m_HiResStack.GetOriginalImage(slice_number) );
      // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
      // m_registration->SetFixedImageRegion( m_LoResStack.GetOriginalImage(slice_number)->GetLargestPossibleRegion() );
      // TEST TO SEE IF THIS MAKES ANY DIFFERENCE
      
      m_registration->GetMetric()->SetFixedImageMask( m_LoResStack.GetResampled2DMask(slice_number) );
      m_registration->GetMetric()->SetMovingImageMask( m_HiResStack.GetOriginal2DMask(slice_number) );
      
      m_registration->SetTransform( m_HiResStack.GetTransform(slice_number) );
      
      m_registration->SetInitialTransformParameters( m_HiResStack.GetTransform(slice_number)->GetParameters() );
      // halve the width and height of the LoRes mask for each slice
      // until optimiser stops throwing errors
      cout << "Trying registration..." << endl;
      unsigned int tries = 0;

      while( !tryRegistration() ) {
        if(++tries > 5)
        {
          cerr << "Tried registration too many times." << endl;
          break;
        }
        cerr << "Tried " << tries << " times...\n\n";
        m_LoResStack.ShrinkMaskSlice(slice_number);
      }
    }
  }
  
  cout << "Finished registration." << endl;
}

template <typename StackType>
bool StackAligner< StackType >::bothImagesExist(unsigned int slice_number) {
  return (m_LoResStack.ImageExists(slice_number) &&
          m_HiResStack.ImageExists(slice_number) );
}

template <typename StackType>
bool StackAligner< StackType >::tryRegistration() {
  try {
    m_registration->Update();
    cout << "Optimizer stop condition: "
         << m_registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
    return true;
  }
  catch( itk::ExceptionObject & err ) {
    cerr << err.GetNameOfClass() << " caught, halving block image width and height." << endl;
    cerr << err << endl;
    return false;
  }
}

#endif

#ifndef TRANSFORMINITIALIZERS_HPP_
#define TRANSFORMINITIALIZERS_HPP_

#include "itkCenteredRigid2DTransform.h"
#include "Stack.hpp"

using namespace std;

namespace InitializeStackTransforms {
  void ToCommonCentre(Stack& stack) {
    typedef itk::CenteredRigid2DTransform< double > TransformType;
    Stack::TransformVectorType newTransforms;
    TransformType::ParametersType parameters(5);
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
			const Stack::SliceType::SizeType &size( stack.GetOriginalImage(i)->GetLargestPossibleRegion().GetSize() ),
			                                 &maxSize( stack.GetMaxSize() ),
                                       &offset( stack.GetOffset() ),
                                       &resamperSize( stack.GetResamplerSize() );
      const Stack::VolumeType::SpacingType &spacings( stack.GetSpacings() );
      
			
			// rotation in radians
			parameters[0] = 0;
			// translation, applied after rotation.
			parameters[3] = (double)offset[0] - spacings[0] * ( (double)maxSize[0] - (double)size[0] ) / 2.0;
			parameters[4] = (double)offset[1] - spacings[1] * ( (double)maxSize[1] - (double)size[1] ) / 2.0;
			// centre of rotation, before translation is applied.
      parameters[1] = parameters[3] + ( spacings[0] * resamperSize[0] ) / 2.0;
			parameters[2] = parameters[4] + ( spacings[1] * resamperSize[1] ) / 2.0;
			
			// set them to new transform
      Stack::TransformType::Pointer transform( TransformType::New() );
      transform->SetParametersByValue( parameters );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
  }
  
  
  template< typename NewTransformType >
  void FromCurrentTransforms(Stack& stack) {
    Stack::TransformVectorType newTransforms;
    
    for(unsigned int i=0; i<stack.GetSize(); i++) {
      typename NewTransformType::Pointer newTransform = NewTransformType::New();
      newTransform->SetIdentity();
      // specialize from vanilla Transform to lowest common denominator in order to call GetCenter()
      typedef itk::MatrixOffsetTransformBase< double, 2, 2 > LinearTransformType;
      LinearTransformType::Pointer oldTransform( dynamic_cast< LinearTransformType* >( stack.GetTransform(i).GetPointer() ) );
      newTransform->SetCenter( oldTransform->GetCenter() );
      newTransform->Compose( oldTransform );
      Stack::TransformType::Pointer baseTransform( newTransform );
      newTransforms.push_back( baseTransform );
    }
    
    // set stack's transforms to newTransforms and update volumes
    stack.SetTransforms(newTransforms);
    
  }
  
}

#endif
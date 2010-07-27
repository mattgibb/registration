#ifndef TRANSFORMINITIALIZERS_HPP_
#define TRANSFORMINITIALIZERS_HPP_

#include "Stack.hpp"

using namespace std;

namespace InitializeStackTransforms {
  void ToCommonCentre(Stack& stack) {
    typedef itk::CenteredRigid2DTransform< double > TransformType;
    Stack::SliceType::SizeType size;
    Stack::TransformVectorType newTransforms;
    TransformType::ParametersType parameters(5);
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
			// calculate parameters
			size = stack.GetOriginalImage(i)->GetLargestPossibleRegion().GetSize();
			
			// rotation in radians
			parameters[0] = 0;
			// translation, applied after rotation.
			parameters[3] = (long)stack.GetOffset()[0] - stack.GetSpacings()[0] * ( (long)stack.GetMaxSize()[0] - (long)size[0] ) / 2.0;
			parameters[4] = (long)stack.GetOffset()[1] - stack.GetSpacings()[1] * ( (long)stack.GetMaxSize()[1] - (long)size[1] ) / 2.0;
			// centre of rotation, before translation is applied.
      parameters[1] = parameters[3] + ( stack.GetSpacings()[0] * stack.GetResamplerSize()[0] ) / 2.0;
			parameters[2] = parameters[4] + ( stack.GetSpacings()[1] * stack.GetResamplerSize()[1] ) / 2.0;
			
			
			// set them to new transform
      Stack::TransformType::Pointer transform( TransformType::New() );
      transform->SetParameters( parameters );
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
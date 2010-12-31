#ifndef STACKTRANSFORMS_HPP_
#define STACKTRANSFORMS_HPP_

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkCenteredRigid2DTransform.h"
#include "Stack.hpp"

using namespace std;

namespace StackTransforms {
  void InitializeToCommonCentre(Stack& stack) {
    typedef itk::CenteredRigid2DTransform< double > TransformType;
    Stack::TransformVectorType newTransforms;
    TransformType::ParametersType parameters(5);
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
			const Stack::SliceType::SizeType &size( stack.GetOriginalImage(i)->GetLargestPossibleRegion().GetSize() ),
			                                 &maxSize( stack.GetMaxSize() ),
                                       &offset( stack.GetOffset() ),
                                       &resamplerSize( stack.GetResamplerSize() );
      const Stack::VolumeType::SpacingType &spacings( stack.GetSpacings() );
			
			// rotation in radians
			parameters[0] = 0;
			// translation, applied after rotation.
			parameters[3] = (double)offset[0] - spacings[0] * ( (double)maxSize[0] - (double)size[0] ) / 2.0;
			parameters[4] = (double)offset[1] - spacings[1] * ( (double)maxSize[1] - (double)size[1] ) / 2.0;
			// centre of rotation, before translation is applied.
      // parameters[1] = parameters[3] + ( spacings[0] * resamplerSize[0] ) / 2.0;
      // parameters[2] = parameters[4] + ( spacings[1] * resamplerSize[1] ) / 2.0;
			
			// set them to new transform
      Stack::TransformType::Pointer transform( TransformType::New() );
      transform->SetParametersByValue( parameters );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
  }
  
  void InitializeFixedStackWithMovingStack( Stack& fixedStack, Stack& movingStack )
  {
    const Stack::TransformVectorType& movingTransforms = movingStack.GetTransforms();
    
    // set the moving slices' centre of rotation to the centre of the fixed image
    for(unsigned int i=0; i<movingStack.GetSize(); i++)
    {
      Stack::TransformType::ParametersType params = movingTransforms[i]->GetParameters();
     
      const Stack::SliceType::SizeType &size( fixedStack.GetOriginalImage(i)->GetLargestPossibleRegion().GetSize() ),
			                                 &maxSize( fixedStack.GetMaxSize() ),
                                       &offset( fixedStack.GetOffset() ),
                                       &resamplerSize( fixedStack.GetResamplerSize() );
      const Stack::VolumeType::SpacingType &spacings( fixedStack.GetSpacings() );
      
      // centre of rotation, before translation is applied
      params[1] = (double)offset[0] + spacings[0] * ( (double)size[0] + resamplerSize[0] - (double)maxSize[0] ) / 2.0;
      params[2] = (double)offset[1] + spacings[1] * ( (double)size[1] + resamplerSize[1] - (double)maxSize[1] ) / 2.0;
      
      movingTransforms[i]->SetParameters(params);
    }
    
  }
  
  template< typename NewTransformType >
  void InitializeFromCurrentTransforms(Stack& stack) {
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
  
  void Save(Stack& stack, const string& fileName)
  {
    typedef itk::TransformFileWriter WriterType;
    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName( fileName.c_str() );
  	
    for(int i=0; i < stack.GetSize(); i++)
  	{
      writer->AddTransform( stack.GetTransform(i) );
    }
    
    try
    {
    	writer->Update();
    }
  	catch( itk::ExceptionObject & err )
  	{
      cerr << "ExceptionObject caught while saving transforms." << endl;
      cerr << err << endl;
  		std::abort();
  	}
    
  }
  
  
  void Load(Stack& stack, const string& fileName)
  {
    typedef itk::TransformFileReader ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    // Software Guide : EndCodeSnippet

    // Some transforms (like the BSpline transform) might not be registered
    // with the factory so we add them manually.
    // itk::TransformFactory<BSplineTransformType>::RegisterTransform();

    reader->SetFileName( fileName.c_str() );

    try
    {
      reader->Update();
    }
    catch( itk::ExceptionObject & err )
    {
      std::cerr << "ExceptionObject caught while reading transforms." << std::endl;
      cerr << err << endl;
  		std::abort();
    }

    // The transform reader is not template and therefore it returns a list
    // of Transform. However, the reader instantiate the appropriate
    // transform class when reading the file but it is up to the user to
    // do the approriate cast.
    typedef ReaderType::TransformListType TransformListType;
    TransformListType * transforms = reader->GetTransformList();
    std::cout << "Number of transforms read = " << transforms->size() << std::endl;

    // Cast each transform
    for(TransformListType::const_iterator it = transforms->begin();
        it != transforms->end(); it++)
    {
      // FROM TransformReadWrite.cxx
      // if(!strcmp((*it)->GetNameOfClass(),"AffineTransform"))
      // {
      //   AffineTransformType::Pointer affine_read = static_cast<AffineTransformType*>((*it).GetPointer());
      //   affine_read->Print(std::cout);
      // }
      // 
      // if(!strcmp((*it)->GetNameOfClass(),"BSplineDeformableTransform"))
      // {
      //   BSplineTransformType::Pointer bspline_read = static_cast<BSplineTransformType*>((*it).GetPointer());
      //   bspline_read->Print(std::cout);
      // }
      // FROM TransformReadWrite.cxx
    }
  }
  
}

#endif
#ifndef STACKTRANSFORMS_HPP_
#define STACKTRANSFORMS_HPP_


#include "boost/filesystem.hpp"

#include "itkTranslationTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkBSplineDeformableTransform.h"
#include "itkBSplineDeformableTransformInitializer.h"
#include "itkLBFGSBOptimizer.h"


#include "Stack.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "StdOutIterationUpdate.hpp"
#include "CenteredTransformPCAInitializer.h"

using namespace std;

namespace StackTransforms {
  typedef itk::MatrixOffsetTransformBase< double, 2, 2 > LinearTransformType;
  typedef itk::TranslationTransform< double, 2 > TranslationTransformType;
  typedef itk::CenteredRigid2DTransform< double > CenteredRigid2DTransformType;
  
  itk::Vector< double, 2 > GetLoResTranslation(const string& roi) {
    itk::Vector< double, 2 > LoResTranslation;
    boost::shared_ptr< YAML::Node > roiNode = config("ROIs/" + roi + ".yml");
    for(unsigned int i=0; i<2; i++) {
      (*roiNode)["Translation"][i] >> LoResTranslation[i];
    }
    return LoResTranslation;
  }
  
  template <typename StackType>
  void InitializeToIdentity(StackType& stack) {
    typename StackType::TransformVectorType newTransforms;
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
      TranslationTransformType::Pointer transform = TranslationTransformType::New();
      transform->SetIdentity();      
      typename StackType::TransformType::Pointer baseTransform( transform );
      newTransforms.push_back( baseTransform );
		}
		
    stack.SetTransforms(newTransforms);
    
  }
  
  template <typename StackType>
  void InitializeWithTranslation(StackType& stack, const itk::Vector< double, 2 > &translation) {
    typename StackType::TransformVectorType newTransforms;
    TranslationTransformType::ParametersType parameters(2);
    
    parameters[0] = translation[0];
    parameters[1] = translation[1];
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
      typename StackType::TransformType::Pointer transform( TranslationTransformType::New() );
      transform->SetParametersByValue( parameters );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
    
  }
  
  template <typename StackType>
  void InitializeWithPCA(StackType& fixedStack, StackType& movingStack) {
    typedef itk::CenteredTransformPCAInitializer<
                   CenteredRigid2DTransformType,
                   typename StackType::SliceType,
                   typename StackType::SliceType > InitializerType;
    
    typename StackType::TransformVectorType newTransforms;
    
    for(unsigned int slice_number=0; slice_number<movingStack.GetSize(); slice_number++)
		{
		  // initialise new transform
      CenteredRigid2DTransformType::Pointer transform = CenteredRigid2DTransformType::New();
      typename InitializerType::Pointer initializer = InitializerType::New();
      initializer->SetTransform( transform );
      initializer->SetFixedImage( fixedStack.GetResampledSlice(slice_number) );
      initializer->SetMovingImage( movingStack.GetOriginalImage(slice_number) );
      try { initializer->InitializeTransform(); }
      catch( itk::ExceptionObject & err )
    	{
        cerr << "itk::ExceptionObject caught while initialising transforms." << endl;
        cerr << err << endl;
    		std::abort();
    	}
      
			// add it to transforms vector
      typename StackType::TransformType::Pointer baseTransform( transform );
      newTransforms.push_back( baseTransform );
		}
		
    movingStack.SetTransforms(newTransforms);
  }
  
  template <typename StackType>
  void InitializeToCommonCentre(StackType& stack) {
    typename StackType::TransformVectorType newTransforms;
    CenteredRigid2DTransformType::ParametersType parameters(5);
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
			const typename StackType::SliceType::SizeType &originalSize( stack.GetOriginalImage(i)->GetLargestPossibleRegion().GetSize() ),
                                       &resamplerSize( stack.GetResamplerSize() );
      const typename StackType::SliceType::SpacingType &originalSpacings( stack.GetOriginalSpacings() );
      const typename StackType::VolumeType::SpacingType &spacings( stack.GetSpacings() );
			
			// rotation in radians
			parameters[0] = 0;
			// translation, applied after rotation.
			parameters[3] = ( originalSpacings[0] * (double)originalSize[0] - spacings[0] * (double)resamplerSize[0] ) / 2.0;
			parameters[4] = ( originalSpacings[1] * (double)originalSize[1] - spacings[1] * (double)resamplerSize[1] ) / 2.0;
			
			// set them to new transform
      typename StackType::TransformType::Pointer transform( CenteredRigid2DTransformType::New() );
      transform->SetParametersByValue( parameters );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
  }
  
  // Moves centre of rotation without changing the transform
  void MoveCenter(LinearTransformType * transform, const LinearTransformType::CenterType& newCenter)
  {
    LinearTransformType::OffsetType offset = transform->GetOffset();
    transform->SetCenter(newCenter);
    transform->SetOffset(offset);
  }
  
  template <typename StackType>
  void SetMovingStackCenterWithFixedStack( StackType& fixedStack, StackType& movingStack )
  {
    // set the moving slices' centre of rotation to the centre of the fixed image
    for(unsigned int slice_number=0; slice_number<movingStack.GetSize(); ++slice_number)
    {
      LinearTransformType::Pointer transform = dynamic_cast< LinearTransformType* >( movingStack.GetTransform(slice_number).GetPointer() );
      if(transform)
      {
        const typename StackType::SliceType::SizeType &resamplerSize( fixedStack.GetResamplerSize() );
        const typename StackType::VolumeType::SpacingType &spacings( fixedStack.GetSpacings() );
        LinearTransformType::CenterType center;
        
        center[0] = spacings[0] * (double)resamplerSize[0] / 2.0;
        center[1] = spacings[1] * (double)resamplerSize[1] / 2.0;
        
        MoveCenter(transform, center);
      }
      else
      {
        cerr << "transform " << slice_number << " isn't a MatrixOffsetTransformBase :-(\n";
        cerr << "transform: " << transform << endl;
        std::abort();
      }
      
    }
    
  }
  
  template <typename StackType, typename NewTransformType>
  void InitializeFromCurrentTransforms(StackType& stack)
  {
    typename StackType::TransformVectorType newTransforms;
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
    {
      typename NewTransformType::Pointer newTransform = NewTransformType::New();
      newTransform->SetIdentity();
      // specialize from vanilla Transform to lowest common denominator in order to call GetCenter()
      LinearTransformType::Pointer oldTransform( dynamic_cast< LinearTransformType* >( stack.GetTransform(i).GetPointer() ) );
      newTransform->SetCenter( oldTransform->GetCenter() );
      newTransform->Compose( oldTransform );
      typename StackType::TransformType::Pointer baseTransform( newTransform );
      newTransforms.push_back( baseTransform );
    }
    
    // set stack's transforms to newTransforms
    stack.SetTransforms(newTransforms);
    
  }
  
  template <typename StackType>
  void InitializeBSplineDeformableFromBulk(StackType& LoResStack, StackType& HiResStack)
  {
    // Perform non-rigid registration
    typedef double CoordinateRepType;
    const unsigned int SpaceDimension = 2;
    const unsigned int SplineOrder = 3;
    typedef itk::BSplineDeformableTransform< CoordinateRepType, SpaceDimension, SplineOrder > TransformType;
    typedef itk::BSplineDeformableTransformInitializer< TransformType, typename StackType::SliceType > InitializerType;
    typename StackType::TransformVectorType newTransforms;
    
    for(unsigned int slice_number=0; slice_number<HiResStack.GetSize(); slice_number++)
    {
      // instantiate transform
      TransformType::Pointer transform( TransformType::New() );
      
      // initialise transform
      typename InitializerType::Pointer initializer = InitializerType::New();
      initializer->SetTransform( transform );
      initializer->SetImage( LoResStack.GetResampledSlice(slice_number) );
      unsigned int gridSize;
      boost::shared_ptr<YAML::Node> deformableParameters = config("deformable_parameters.yml");
      (*deformableParameters)["bsplineTransform"]["gridSize"] >> gridSize;
      TransformType::RegionType::SizeType gridSizeInsideTheImage;
      gridSizeInsideTheImage.Fill(gridSize);
      initializer->SetGridSizeInsideTheImage( gridSizeInsideTheImage );
      
      initializer->InitializeTransform();
      
      transform->SetBulkTransform( HiResStack.GetTransform(slice_number) );
      
      // set initial parameters to zero
      TransformType::ParametersType initialDeformableTransformParameters( transform->GetNumberOfParameters() );
      initialDeformableTransformParameters.Fill( 0.0 );
      transform->SetParametersByValue( initialDeformableTransformParameters );
      
      // add transform to vector
      typename StackType::TransformType::Pointer baseTransform( transform );
      newTransforms.push_back( baseTransform );
    }
    HiResStack.SetTransforms(newTransforms);
  }
  
  // Translate a single slice
  template <typename StackType>
  void Translate(StackType& stack, const itk::Vector< double, 2 > translation, unsigned int slice_number)
  {
    // attempt to cast stack transform and apply translation
    LinearTransformType::Pointer linearTransform
      = dynamic_cast< LinearTransformType* >( stack.GetTransform(slice_number).GetPointer() );
    TranslationTransformType::Pointer translationTransform
      = dynamic_cast< TranslationTransformType* >( stack.GetTransform(slice_number).GetPointer() );
    if(linearTransform)
    {
      // construct transform to represent translation
      LinearTransformType::Pointer translationTransform = LinearTransformType::New();
      translationTransform->SetIdentity();
      translationTransform->SetTranslation(translation);
      // if second argument is true, translationTransform is applied first,
      // then linearTransform
      linearTransform->Compose(translationTransform, true);
    }
    else if(translationTransform)
    {
      translationTransform->Translate(translation);
    }
    else
    {
      cerr << "slice " << slice_number << " isn't a MatrixOffsetTransformBase or a TranslationTransform :-(\n";
      cerr << "stack.GetTransform(slice_number): " << stack.GetTransform(slice_number) << endl;
      std::abort();
    }
  }

  // Translate the entire stack
  template <typename StackType>
  void Translate(StackType& stack, const itk::Vector< double, 2 > translation)
  {
    // attempt to cast stack transforms and apply translation
    for(unsigned int slice_number=0; slice_number<stack.GetSize(); ++slice_number)
    {
      Translate(stack, translation, slice_number);
    }
  }
  
}

#endif

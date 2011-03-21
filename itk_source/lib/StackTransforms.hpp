#ifndef STACKTRANSFORMS_HPP_
#define STACKTRANSFORMS_HPP_

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkIdentityTransform.h"
#include "itkTranslationTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkBSplineDeformableTransform.h"
#include "itkBSplineDeformableTransformInitializer.h"
#include "itkLBFGSBOptimizer.h"
#include "itkTransformFactory.h"


#include "Stack.hpp"
#include "Parameters.hpp"
#include "StdOutIterationUpdate.hpp"

using namespace std;

namespace StackTransforms {
  void InitializeToIdentity(Stack& stack) {
    typedef itk::IdentityTransform< double, 2 > TransformType;
    Stack::TransformVectorType newTransforms;
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
      Stack::TransformType::Pointer transform( TransformType::New() );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
    
  }
  
  void InitializeWithTranslation(Stack& stack, const itk::Vector< double, 2 > &translation) {
    typedef itk::TranslationTransform< double, 2 > TransformType;
    Stack::TransformVectorType newTransforms;
    TransformType::ParametersType parameters(2);
    
    parameters[0] = translation[0];
    parameters[1] = translation[1];
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
      Stack::TransformType::Pointer transform( TransformType::New() );
      transform->SetParametersByValue( parameters );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
    
  }
  
  void InitializeToCommonCentre(Stack& stack) {
    typedef itk::CenteredRigid2DTransform< double > TransformType;
    Stack::TransformVectorType newTransforms;
    TransformType::ParametersType parameters(5);
    
    for(unsigned int i=0; i<stack.GetSize(); i++)
		{
			const Stack::SliceType::SizeType &originalSize( stack.GetOriginalImage(i)->GetLargestPossibleRegion().GetSize() ),
                                       &resamplerSize( stack.GetResamplerSize() );
      const Stack::SliceType::SpacingType &originalSpacings( stack.GetOriginalSpacings() );
      const Stack::VolumeType::SpacingType &spacings( stack.GetSpacings() );
			
			// rotation in radians
			parameters[0] = 0;
			// translation, applied after rotation.
			parameters[3] = ( originalSpacings[0] * (double)originalSize[0] - spacings[0] * (double)resamplerSize[0] ) / 2.0;
			parameters[4] = ( originalSpacings[1] * (double)originalSize[1] - spacings[1] * (double)resamplerSize[1] ) / 2.0;
			
			// set them to new transform
      Stack::TransformType::Pointer transform( TransformType::New() );
      transform->SetParametersByValue( parameters );
      newTransforms.push_back( transform );
		}
		
    stack.SetTransforms(newTransforms);
  }
    
  void SetMovingStackCORWithFixedStack( Stack& fixedStack, Stack& movingStack )
  {
    const Stack::TransformVectorType& movingTransforms = movingStack.GetTransforms();
    
    // set the moving slices' centre of rotation to the centre of the fixed image
    for(unsigned int i=0; i<movingStack.GetSize(); i++)
    {
      Stack::TransformType::ParametersType params = movingTransforms[i]->GetParameters();
     
      const Stack::SliceType::SizeType &resamplerSize( fixedStack.GetResamplerSize() );
      const Stack::VolumeType::SpacingType &spacings( fixedStack.GetSpacings() );
      
      // centre of rotation, before translation is applied
      params[1] = spacings[0] * (double)resamplerSize[0] / 2.0;
      params[2] = spacings[1] * (double)resamplerSize[1] / 2.0;
      
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
  
  void InitializeBSplineDeformableFromBulk(Stack& LoResStack, Stack& HiResStack)
  {
    // Perform non-rigid registration
    typedef double CoordinateRepType;
    const unsigned int SpaceDimension = 2;
    const unsigned int SplineOrder = 3;
    typedef itk::BSplineDeformableTransform< CoordinateRepType, SpaceDimension, SplineOrder > TransformType;
    typedef itk::BSplineDeformableTransformInitializer< TransformType, Stack::SliceType > InitializerType;
    Stack::TransformVectorType newTransforms;
    
    for(unsigned int slice_number=0; slice_number<HiResStack.GetSize(); slice_number++)
    {
      // instantiate transform
      TransformType::Pointer transform( TransformType::New() );
      
      // initialise transform
      InitializerType::Pointer initializer = InitializerType::New();
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
      Stack::TransformType::Pointer baseTransform( transform );
      newTransforms.push_back( baseTransform );
    }
    HiResStack.SetTransforms(newTransforms);
  }
  
  void SetOptimizerScalesForCenteredRigid2DTransform(itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
  {
    double translationScale, rotationScale;
    registrationParameters()["optimizer"]["scale"]["translation"] >> translationScale;
    registrationParameters()["optimizer"]["scale"]["rotation"] >> rotationScale;
  	itk::Array< double > scales( 5 );
    scales[0] = 1.0;
    scales[1] = translationScale;
    scales[2] = translationScale;
    scales[3] = translationScale;
    scales[4] = translationScale;
    optimizer->SetScales( scales );
  }
  
  void SetOptimizerScalesForCenteredSimilarity2DTransform(itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
  {
    double translationScale, rotationScale, sizeScale;
    registrationParameters()["optimizer"]["scale"]["translation"] >> translationScale;
    registrationParameters()["optimizer"]["scale"]["rotation"] >> rotationScale;
    registrationParameters()["optimizer"]["scale"]["size"] >> sizeScale;
  	itk::Array< double > scales( 6 );
    scales[0] = sizeScale;
    scales[1] = rotationScale;
    scales[2] = translationScale;
    scales[3] = translationScale;
    scales[4] = translationScale;
    scales[5] = translationScale;
    optimizer->SetScales( scales );
  }
  
  void SetOptimizerScalesForCenteredAffineTransform(itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
  {
    double translationScale, sizeScale;
    registrationParameters()["optimizer"]["scale"]["translation"] >> translationScale;
    registrationParameters()["optimizer"]["scale"]["size"] >> sizeScale;
  	itk::Array< double > scales( 8 );
  	// four matrix elements
    scales[0] = sizeScale;
    scales[1] = sizeScale;
    scales[2] = sizeScale;
    scales[3] = sizeScale;
  	// two centre coordinates
    scales[4] = translationScale;
    scales[5] = translationScale;
  	// two translation coordinates
    scales[6] = translationScale;
    scales[7] = translationScale;
    optimizer->SetScales( scales );
  }

  void ConfigureLBFGSBOptimizer(unsigned int numberOfParameters, itk::LBFGSBOptimizer::Pointer optimizer)
  {
    // From Example
    itk::LBFGSBOptimizer::BoundSelectionType boundSelect( numberOfParameters );
    itk::LBFGSBOptimizer::BoundValueType upperBound( numberOfParameters );
    itk::LBFGSBOptimizer::BoundValueType lowerBound( numberOfParameters );
    
    boundSelect.Fill( 0 );
    upperBound.Fill( 0.0 );
    lowerBound.Fill( 0.0 );
    
    optimizer->SetBoundSelection( boundSelect );
    optimizer->SetUpperBound( upperBound );
    optimizer->SetLowerBound( lowerBound );
    
    optimizer->SetCostFunctionConvergenceFactor( 1e+12 );
    optimizer->SetProjectedGradientTolerance( 1.0 );
    optimizer->SetMaximumNumberOfIterations( 500 );
    optimizer->SetMaximumNumberOfEvaluations( 500 );
    optimizer->SetMaximumNumberOfCorrections( 5 );
    
    // Create an observer and register it with the optimizer
    typedef StdOutIterationUpdate< itk::LBFGSBOptimizer > StdOutObserverType;
    StdOutObserverType::Pointer stdOutObserver = StdOutObserverType::New();
    optimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
    
  }
  
  void SetOptimizerScalesForBSplineDeformableTransform(Stack &stack, itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
  {
    typedef itk::SingleValuedNonLinearOptimizer::ScalesType ScalesType;
    ScalesType optimizerScales = ScalesType( stack.GetTransform(0)->GetNumberOfParameters() );
    optimizerScales.Fill( 1.0 );
    
    optimizer->SetScales( optimizerScales );
    
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
    
    // Some transforms (like the BSpline transform) might not be registered
    // with the factory so we add them manually.
    itk::TransformFactory< itk::TranslationTransform< double, 2 > >::RegisterTransform();
    
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
    
    // Assert stack has the same number of slices as there are transforms from the reader
    if (stack.GetSize() != transforms->size())
    {
      cerr << "Stack size and transform file size are different!" << endl;
      std::abort();
    }
    
    // assign transforms to stack
    Stack::TransformVectorType newTransforms;
    for(TransformListType::const_iterator it = transforms->begin();
        it != transforms->end(); it++)
    {
      // build vector out of list
      Stack::TransformType::Pointer transform = static_cast<Stack::TransformType*>( (*it).GetPointer() );
      newTransforms.push_back( transform );
    }
    
    stack.SetTransforms(newTransforms);
  }
  
}

#endif

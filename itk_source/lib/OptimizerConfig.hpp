#ifndef OPTIMIZERCONFIG_HPP_
#define OPTIMIZERCONFIG_HPP_


// #include "boost/filesystem.hpp"

// #include "itkIdentityTransform.h"
// #include "itkTranslationTransform.h"
// #include "itkCenteredRigid2DTransform.h"
// #include "itkCenteredAffineTransform.h"
#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkBSplineDeformableTransform.h"
#include "itkBSplineDeformableTransformInitializer.h"
#include "itkLBFGSBOptimizer.h"


// #include "Stack.hpp"
// #include "Dirs.hpp"
// #include "Parameters.hpp"
// #include "StdOutIterationUpdate.hpp"

using namespace std;

namespace OptimizerConfig {
  // typedef itk::MatrixOffsetTransformBase< double, 2, 2 > LinearTransformType;
  // typedef itk::TranslationTransform< double, 2 > TranslationTransformType;
  
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

  template <typename StackType>
  void SetOptimizerScalesForBSplineDeformableTransform(StackType &stack, itk::SingleValuedNonLinearOptimizer::Pointer optimizer)
  {
    typedef itk::SingleValuedNonLinearOptimizer::ScalesType ScalesType;
    ScalesType optimizerScales = ScalesType( stack.GetTransform(0)->GetNumberOfParameters() );
    optimizerScales.Fill( 1.0 );
    
    optimizer->SetScales( optimizerScales );
    
  }
  
  template <typename StackType>
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
  
}

#endif

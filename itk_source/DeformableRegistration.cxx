#include "itkNormalizedCorrelationImageToImageMetric.h"

// my files
#include "Stack.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "Profiling.hpp"

void checkUsage(int argc, char const *argv[]) {
  if( argc != 3 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet resultsDir\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  string outputDir(Dirs::ResultsDir() + argv[2] + "/");
	
	// initialise stack objects
  Stack::VolumeType::SpacingType LoResSpacings, HiResSpacings;
	for(unsigned int i=0; i<3; i++) {
    imageDimensions()["LoResSpacings"][i] >> LoResSpacings[i];
    imageDimensions()["HiResSpacings"][i] >> HiResSpacings[i];
  }
  
  Stack::SliceType::SizeType LoResSize;
  itk::Vector< double, 2 > LoResTranslation;
  for(unsigned int i=0; i<2; i++) {
    imageDimensions()["LoResSize"][i] >> LoResSize[i];
    imageDimensions()["LoResTranslation"][i] >> LoResTranslation[i];
  }
  
  Stack LoResStack( getFileNames(Dirs::BlockDir(), Dirs::SliceFile()), LoResSpacings , LoResSize);
  
  Stack::SliceType::SpacingType HiResOriginalSpacings;
  for(unsigned int i=0; i<2; i++) HiResOriginalSpacings[i] = HiResSpacings[i];
  
  Stack HiResStack(getFileNames(Dirs::SliceDir(), Dirs::SliceFile()), HiResOriginalSpacings,
        LoResStack.GetSpacings(), LoResStack.GetResamplerSize());
  
  // initialise stacks' transforms with saved transform files
  StackTransforms::Load(LoResStack, outputDir + "LoResTransforms.meta");
  StackTransforms::Load(HiResStack, outputDir + "HiResTransforms.meta");
  
  // Generate fixed images to register against
  LoResStack.updateVolumes();
  writeImage< Stack::VolumeType >( LoResStack.GetVolume(), outputDir + "LoResPersistedStack.mha" );
  
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResPersistedStack.mha" );
  
  std::exit(0);
  
  // initialise registration framework
  RegistrationBuilder registrationBuilder;
  RegistrationBuilder::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner stackAligner(LoResStack, HiResStack, registration);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredRigid2DTransform( registration->GetOptimizer() );
  
  // Add time and memory probes
  itkProbesCreate();
  
  // perform centered rigid 2D registration on each pair of slices
  itkProbesStart( "Aligning stacks" );
  stackAligner.Update();
  itkProbesStop( "Aligning stacks" );
  
  // Report the time and memory taken by the registration
  itkProbesReport( std::cout );
  
  // write rigid transforms
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResRigidStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResRigidMask.mha" );
  // StackTransforms::InitializeFromCurrentTransforms< itk::CenteredSimilarity2DTransform< double > >(HiResStack);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredSimilarity2DTransform( registration->GetOptimizer() );
  
  // perform similarity rigid 2D registration
  stackAligner.Update();
  
  // write similarity transforms
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResSimilarityStack.mha" );
  
  // repeat registration with affine transform
  StackTransforms::InitializeFromCurrentTransforms< itk::CenteredAffineTransform< double, 2 > >(HiResStack);
  StackTransforms::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );
  stackAligner.Update();
  HiResStack.updateVolumes();
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResAffineStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResAffineMask.mha" );
  
  // Update LoRes as the masks might have shrunk
  LoResStack.updateVolumes();
  
  // Write final transforms to file
  StackTransforms::Save(LoResStack, Dirs::ResultsDir() + "LoResStackTransforms.meta");
  StackTransforms::Save(HiResStack, Dirs::ResultsDir() + "HiResStackAffineTransforms.meta");
  
  
  // Perform non-rigid registration
  StackTransforms::InitializeBSplineDeformableFromBulk(LoResStack, HiResStack);
  
  typedef itk::LBFGSBOptimizer DeformableOptimizerType;
  DeformableOptimizerType::Pointer deformableOptimizer = DeformableOptimizerType::New();
  
  // StackTransforms::ConfigureLBFGSBOptimizer(LoResStack.GetTransform(0)->GetNumberOfParameters(), deformableOptimizer);
  unsigned int numberOfParameters = HiResStack.GetTransform(0)->GetNumberOfParameters();
  cout << "numberOfParameters: " << numberOfParameters << endl;
  StackTransforms::SetOptimizerScalesForBSplineDeformableTransform(HiResStack, registration->GetOptimizer());
  // itk::LBFGSBOptimizer::BoundSelectionType boundSelect( numberOfParameters );
  // itk::LBFGSBOptimizer::BoundValueType upperBound( numberOfParameters );
  // itk::LBFGSBOptimizer::BoundValueType lowerBound( numberOfParameters );
  // 
  // boundSelect.Fill( 0 );
  // upperBound.Fill( 0.0 );
  // lowerBound.Fill( 0.0 );
  // 
  // deformableOptimizer->SetBoundSelection( boundSelect );
  // deformableOptimizer->SetUpperBound( upperBound );
  // deformableOptimizer->SetLowerBound( lowerBound );
  // 
  // deformableOptimizer->SetCostFunctionConvergenceFactor( 1e+7 );
  // deformableOptimizer->SetProjectedGradientTolerance( 0.0000001 );
  // deformableOptimizer->SetMaximumNumberOfIterations( 500 );
  // deformableOptimizer->SetMaximumNumberOfEvaluations( 500 );
  // deformableOptimizer->SetMaximumNumberOfCorrections( 10 );
  // deformableOptimizer->MaximizeOn();
    
  
  // Create an observer and register it with the optimizer
  // typedef StdOutIterationUpdate< itk::LBFGSBOptimizer > StdOutObserverType;
  // StdOutObserverType::Pointer stdOutObserver = StdOutObserverType::New();
  // deformableOptimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
  // 
  // registration->SetOptimizer( deformableOptimizer );
  
  typedef itk::NormalizedCorrelationImageToImageMetric< Stack::SliceType, Stack::SliceType > MetricType;
  // typedef itk::MattesMutualInformationImageToImageMetric< Stack::SliceType, Stack::SliceType > MetricType;
  MetricType::Pointer metric = MetricType::New();

  // specific settings
  // metric->SetNumberOfSpatialSamples( 3000 );
  // metric->SetNumberOfHistogramBins( 50 );
  
  registration->SetMetric( metric );
  
  // StackTransforms::SetOptimizerScalesForBSplineDeformableTransform(LoResStack, deformableOptimizer );
  stackAligner.Update();
  HiResStack.updateVolumes();
  
  writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResDeformedStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResSimilarityMask.mha" );
  
  return EXIT_SUCCESS;
}

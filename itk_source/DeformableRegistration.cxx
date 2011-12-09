// Perform non-rigid registration based on bulk transform output of BuildVolumes

#include "itkNormalizedCorrelationImageToImageMetric.h"

// my files
#include "Stack.hpp"
#include "NormalizeImages.hpp"
#include "StackInitializers.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "IOHelpers.hpp"
#include "StackIOHelpers.hpp"
#include "StackTransforms.hpp"
#include "OptimizerConfig.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "Profiling.hpp"

void checkUsage(int argc, char const *argv[]) {
  if( argc < 3 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet resultsDir (slice)\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  Dirs::SetOutputDirName(argv[2]);
  
  // basenames is either single name from command line
  // or list from config file
  vector< string > basenames = argc >= 4 ?
                               vector< string >(1, argv[3]) :
                               getFileNames(Dirs::ImageList());
  
  // prepend directory to each filename in list
  vector< string > LoResFilePaths = constructPaths(Dirs::BlockDir(), basenames, ".bmp");
  vector< string > HiResFilePaths = constructPaths(Dirs::SliceDir(), basenames, ".bmp");
  
	// initialise stack objects
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(LoResFilePaths);
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFilePaths);
  normalizeImages< StackType >(LoResImages);
  normalizeImages< StackType >(HiResImages);
  boost::shared_ptr< StackType > LoResStack = InitializeLoResStack<StackType>(LoResImages);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages);
  
  // initialise stacks' transforms with saved transform files
  Load(*LoResStack, Dirs::LoResTransformsDir(), basenames);
  Load(*HiResStack, Dirs::HiResTransformsDir(), basenames);
  
  // shrink mask slices
  cout << "Test mask load.\n";
  cout << "THE LINE BELOW SHOULD BE LoResStack...?\n";
  vector< unsigned int > numberOfTimesTooBig = loadVectorFromFiles< unsigned int >(Dirs::ResultsDir() + "number_of_times_too_big",  basenames);
  HiResStack->SetNumberOfTimesTooBig( numberOfTimesTooBig );
  
  // Generate fixed images to register against
  LoResStack->updateVolumes();
  writeImage< StackType::VolumeType >( LoResStack->GetVolume(), Dirs::ResultsDir() + "LoResPersistedStack.mha" );
  
  HiResStack->updateVolumes();
  writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResPersistedStack.mha" );
  
  // initialise registration framework
  boost::shared_ptr<YAML::Node> pDeformableParameters = config("deformable_parameters.yml");
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder(*pDeformableParameters);
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner< StackType > stackAligner(*LoResStack, *HiResStack, registration);
  
  // Perform non-rigid registration
  StackTransforms::InitializeBSplineDeformableFromBulk(*LoResStack, *HiResStack);
  OptimizerConfig::SetOptimizerScalesForBSplineDeformableTransform(*HiResStack, registration->GetOptimizer());
  
  // typedef itk::LBFGSBOptimizer DeformableOptimizerType;
  // DeformableOptimizerType::Pointer deformableOptimizer = DeformableOptimizerType::New();
  // 
  // StackTransforms::ConfigureLBFGSBOptimizer(LoResStack->GetTransform(0)->GetNumberOfParameters(), deformableOptimizer);
  // unsigned int numberOfParameters = HiResStack->GetTransform(0)->GetNumberOfParameters();
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
  // 
  // Create an observer and register it with the optimizer
  // typedef StdOutIterationUpdate< itk::LBFGSBOptimizer > StdOutObserverType;
  // StdOutObserverType::Pointer stdOutObserver = StdOutObserverType::New();
  // deformableOptimizer->AddObserver( itk::IterationEvent(), stdOutObserver );
  // 
  // registration->SetOptimizer( deformableOptimizer );
  
  itkProbesCreate();
  itkProbesStart( "Aligning stacks" );
  stackAligner.Update();
  itkProbesStop( "Aligning stacks" );
  itkProbesReport( std::cout );
  
  HiResStack->updateVolumes();
  
  writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResDeformedStack.mha" );
  writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), Dirs::ResultsDir() + "HiResSimilarityMask.mha" );
  
  return EXIT_SUCCESS;
}

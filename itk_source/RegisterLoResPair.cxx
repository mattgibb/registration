// Register two LoRes slices to each other

#include "boost/filesystem.hpp"

// my files
#include "Stack.hpp"
#include "StackInitializers.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "Profiling.hpp"

void checkUsage(int argc, char const *argv[]) {
  if( argc < 4 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet slice1 slice2 (writeImages=false)\n\n";
    cerr << "e.g. " << argv[0] << " Rat28 0053 0054 true\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
  using namespace boost::filesystem;
  
  // Verify the number of parameters in the command line
  checkUsage(argc, argv);
  
  // Process command line arguments
  Dirs::SetDataSet(argv[1]);
  string slice1BaseName(argv[2]);
  string slice2BaseName(argv[3]);
  Dirs::SetOutputDirName("LoResCorrectionTransforms");
  string blockDir(Dirs::ImagesDir() + "LoRes_rgb/downsamples_1/");
  vector< string > slice1FileName(1, Dirs::BlockDir() + slice1BaseName + ".bmp");
  vector< string > slice2FileName(1, Dirs::BlockDir() + slice2BaseName + ".bmp");
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType slice1Image = readImages< StackType::SliceType >(slice1FileName);
  StackType::SliceVectorType slice2Image = readImages< StackType::SliceType >(slice2FileName);
  boost::shared_ptr< StackType > slice1Stack = InitializeLoResStack<StackType>(slice1Image);
  boost::shared_ptr< StackType > slice2Stack = InitializeLoResStack<StackType>(slice2Image);
  slice1Stack->SetBasenames(vector< string >(1, slice1BaseName));
  slice2Stack->SetBasenames(vector< string >(1, slice2BaseName));
  
  // initialize stacks' transforms
  StackTransforms::InitializeWithTranslation( *slice1Stack, StackTransforms::GetLoResTranslation("whole_heart") );
  StackTransforms::InitializeWithTranslation( *slice2Stack, StackTransforms::GetLoResTranslation("whole_heart") );
  ApplyAdjustments( *slice1Stack, Dirs::ConfigDir() + "LoRes_adjustments/");
  ApplyAdjustments( *slice2Stack, Dirs::ConfigDir() + "LoRes_adjustments/");
  
  // record original slice2 transform
  // convert Array of initial translation to Vector
  itk::Array< double > oldSlice2parameters( slice2Stack->GetTransform(0)->GetParameters() );
  itk::Vector< double, 2 > oldTranslation;
  oldTranslation[0] = oldSlice2parameters[0];
  oldTranslation[1] = oldSlice2parameters[1];
  
  // create output dir before write operations
  create_directory( Dirs::ResultsDir() );
  
  // Build LoRes slices
  slice1Stack->updateVolumes();
  slice2Stack->updateVolumes();
  if(argc >= 5)
  {
    writeImage< StackType::SliceType >( slice1Stack->GetResampledSlice(0), Dirs::ResultsDir() + slice1BaseName + ".mha" );
    writeImage< StackType::SliceType >( slice2Stack->GetResampledSlice(0), Dirs::ResultsDir() + slice2BaseName + "_before_registration.mha" );
  }
  // initialise registration framework
  boost::shared_ptr<YAML::Node> pParameters = config("2_slice_parameters.yml");
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder(*pParameters);
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  
  // Could change this to register against original fixed image and fixed image masks,
  // by applying the inverse fixed transform to the moving one, registering, then
  // applying the fixed transform back again afterwards.
  registration->SetFixedImage( slice1Stack->GetResampledSlice(0) );
  registration->SetMovingImage( slice2Stack->GetOriginalImage(0) );
  
  registration->GetMetric()->SetFixedImageMask( slice1Stack->GetResampled2DMask(0) );
  registration->GetMetric()->SetMovingImageMask( slice2Stack->GetOriginal2DMask(0) );
  
  registration->SetTransform( slice2Stack->GetTransform(0) );
  
  registration->SetInitialTransformParameters( slice2Stack->GetTransform(0)->GetParameters() );
  
  try { registration->Update(); }
  catch( itk::ExceptionObject & err ) 
  { 
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }
  
  cout << "Optimizer stop condition: "
       << registration->GetOptimizer()->GetStopConditionDescription() << endl << endl;
  
  // write registered image
  slice2Stack->updateVolumes();
  if(argc >= 5)
  {
    writeImage< StackType::SliceType >( slice2Stack->GetResampledSlice(0), Dirs::ResultsDir() + slice2BaseName + "_after_registration.mha" );
  }
  
  // Write resultant transform
  // subtract initial translation, so that
  // transform represents the relative translation from old to new position
  typedef StackTransforms::TranslationTransformType TranslationType;
  TranslationType::Pointer translation
    = static_cast< TranslationType* >( slice2Stack->GetTransform(0).GetPointer() );
  translation->Translate(-oldTranslation);
  
  string transformPath =
    Dirs::ResultsDir() + slice1BaseName + "_" + slice2BaseName + ".meta";
  
  writeTransform(translation, transformPath);
  
  return EXIT_SUCCESS;
  
}

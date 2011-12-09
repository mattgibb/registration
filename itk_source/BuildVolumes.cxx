#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include <assert.h>
#include "itkCenteredSimilarity2DTransform.h"

// my files
#include "Stack.hpp"
#include "NormalizeImages.hpp"
#include "StackInitializers.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "StackTransforms.hpp"
#include "OptimizerConfig.hpp"
#include "Profiling.hpp"


namespace po = boost::program_options;
using namespace boost::filesystem;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
  // Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  string blockDir = vm.count("blockDir") ? vm["blockDir"].as<string>() + "/" : Dirs::BlockDir();
  string sliceDir = vm.count("sliceDir") ? vm["sliceDir"].as<string>() + "/" : Dirs::SliceDir();
  const bool writeImages = vm["writeImages"].as<bool>();
  
  // basenames is either single name from command line
  // or list from config file
  vector< string > basenames = vm.count("slice") ?
                               vector< string >(1, vm["slice"].as<string>()) :
                               getFileNames(Dirs::ImageList());
  
  // prepend directory to each filename in list
  vector< string > LoResFilePaths = constructPaths(blockDir, basenames, ".bmp");
  vector< string > HiResFilePaths = constructPaths(sliceDir, basenames, ".bmp");
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(LoResFilePaths);
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFilePaths);
  normalizeImages< StackType >(LoResImages);
  normalizeImages< StackType >(HiResImages);
  boost::shared_ptr< StackType > LoResStack = InitializeLoResStack<StackType>(LoResImages);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages);
  
  // Assert stacks have the same number of slices
  assert(LoResStack->GetSize() == HiResStack->GetSize());
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  if( vm.count("blockDir") )
    // if working with segmentations already in the right coordinate system,
    // no need to apply transforms
    StackTransforms::InitializeToIdentity(*LoResStack);
  else
  {
    // if working from the original images, apply the necessary translation
    StackTransforms::InitializeWithTranslation( *LoResStack, StackTransforms::GetLoResTranslation("whole_heart") );
    ApplyAdjustments( *LoResStack, LoResFilePaths, Dirs::ConfigDir() + "LoRes_adjustments/");
  }
  
  // Generate fixed images to register against
  LoResStack->updateVolumes();
  
  if( vm["pca"].as<bool>() )
  {
    // update both volumes so that their principal components align
    StackTransforms::InitializeWithPCA(*LoResStack, *HiResStack);
  }
  else
  {
    StackTransforms::InitializeToCommonCentre( *HiResStack );
    StackTransforms::SetMovingStackCenterWithFixedStack( *LoResStack, *HiResStack );
  }
  
  // create output dir before write operations
  create_directory( Dirs::ResultsDir() );
  
  if( writeImages )
  {
    writeImage< StackType::VolumeType >( LoResStack->GetVolume(), Dirs::ResultsDir() + "LoResStack.mha" );
  }
  
  // initialise registration framework
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder;
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner< StackType > stackAligner(*LoResStack, *HiResStack, registration);
  
  // Scale parameter space
  OptimizerConfig::SetOptimizerScalesForCenteredRigid2DTransform( registration->GetOptimizer() );
  
  // Add time and memory probes
  itkProbesCreate();
  
  // perform centered rigid 2D registration on each pair of slices
  itkProbesStart( "Aligning stacks" );
  stackAligner.Update();
  itkProbesStop( "Aligning stacks" );
  
  // Report the time and memory taken by the registration
  itkProbesReport( std::cout );
  
  // write rigid transforms
  if( writeImages )
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResRigidStack.mha" );
    // writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), Dirs::ResultsDir() + "HiResRigidMask.mha" );
  }
  StackTransforms::InitializeFromCurrentTransforms< StackType, itk::CenteredSimilarity2DTransform< double > >(*HiResStack);
  
  // Scale parameter space
  OptimizerConfig::SetOptimizerScalesForCenteredSimilarity2DTransform( registration->GetOptimizer() );
  
  // perform similarity rigid 2D registration
  stackAligner.Update();
  
  // write similarity transforms
  if( writeImages )
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResSimilarityStack.mha" );
  }
  
  // repeat registration with affine transform
  StackTransforms::InitializeFromCurrentTransforms< StackType, itk::CenteredAffineTransform< double, 2 > >(*HiResStack);
  OptimizerConfig::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );
  stackAligner.Update();
  
  if( writeImages )
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResAffineStack.mha" );
    // writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), Dirs::ResultsDir() + "HiResAffineMask.mha" );
  }
  
  // Update LoRes as the masks might have shrunk
  LoResStack->updateVolumes();
  
  // persist mask numberOfTimesTooBig and final metric values
  saveVectorToFiles(HiResStack->GetNumberOfTimesTooBig(), "number_of_times_too_big", basenames );
  saveVectorToFiles(stackAligner.GetFinalMetricValues(), "metric_values", basenames );
  
  // write transforms to directories labeled by both ds ratios
  create_directory(Dirs::LoResTransformsDir());
  create_directory(Dirs::HiResTransformsDir());
  Save(*LoResStack, LoResFilePaths, Dirs::LoResTransformsDir());
  Save(*HiResStack, HiResFilePaths, Dirs::HiResTransformsDir());
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("dataSet", po::value<string>(), "which rat to use")
      ("outputDir", po::value<string>(), "directory to place results")
      ("slice", po::value<string>(), "optional individual slice to register")
      ("blockDir", po::value<string>(), "directory containing LoRes originals")
      ("sliceDir", po::value<string>(), "directory containing HiRes originals")
      ("writeImages", po::bool_switch(), "output images and masks")
      ("pca", po::bool_switch(), "align principal axes of HiRes images with LoRes")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("slice", 1);
  
  // parse command line
  po::variables_map vm;
	try
	{
  po::store(po::command_line_parser(argc, argv)
            .options(opts)
            .positional(p)
            .run(),
            vm);
	}
	catch (std::exception& e)
	{
	  cerr << "caught command-line parsing error" << endl;
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  po::notify(vm);
  
  // if help is specified, or positional args aren't present
  if (vm.count("help") || !vm.count("dataSet") || !vm.count("outputDir")) {
    cerr << "Usage: "
      << argv[0] << " [--dataSet=]RatX [--outputDir=]my_dir [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

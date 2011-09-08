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
  const bool writeImages = vm["writeImages"].as<bool>();
  
  vector< string > LoResFileNames, HiResFileNames;
  if( vm.count("slice") )
  {
    LoResFileNames.push_back(blockDir + vm["slice"].as<string>());
    HiResFileNames.push_back(Dirs::SliceDir() + vm["slice"].as<string>());
  }
  else
  {
    LoResFileNames = getFilePaths(blockDir, Dirs::SliceFile());
    HiResFileNames = getFilePaths(Dirs::SliceDir(), Dirs::SliceFile());
  }
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(LoResFileNames);
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFileNames);
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
    ApplyAdjustments( *LoResStack, LoResFileNames, Dirs::ConfigDir() + "LoRes_adjustments/");
  }
  
  StackTransforms::InitializeToCommonCentre( *HiResStack );
  StackTransforms::SetMovingStackCenterWithFixedStack( *LoResStack, *HiResStack );
  
  // create output dir before write operations
  create_directory( Dirs::ResultsDir() );
  
  // Generate fixed images to register against
  LoResStack->updateVolumes();
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
  
  // persist mask numberOfTimesTooBig
  saveNumberOfTimesTooBig(*HiResStack, Dirs::ResultsDir() + "numberOfTimesTooBig.txt");
  
  // write transforms to directories labeled by both ds ratios
  create_directory(Dirs::LoResTransformsDir());
  create_directory(Dirs::HiResTransformsDir());
  Save(*LoResStack, LoResFileNames, Dirs::LoResTransformsDir());
  Save(*HiResStack, HiResFileNames, Dirs::HiResTransformsDir());
  
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
      ("writeImages", po::bool_switch(), "output images and masks")
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

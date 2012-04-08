// Register adjacent HiRes images to each other,
// initialised by loaded transforms from previous registration

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

// my files
#include "Stack.hpp"
#include "RegistrationBuilder.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "OptimizerConfig.hpp"
#include "ScaleImages.hpp"
#include "SimpleTransformWriter.hpp"
#include "MetricValueWriter.hpp"

using namespace boost::filesystem;
namespace po = boost::program_options;
using namespace boost;

// function declarations
po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // Process command line arguments
  Dirs::SetDataSet(argv[1]);
  Dirs::SetOutputDirName(argv[2]);
  string transform = vm["transform"].as<string>();
  string roi = vm.count("roi") ? vm["roi"].as<string>() : "ROI";
  
  // for globally available registrationParameters(),
  // used in e.g. OptimizerConfig
  Dirs::SetParamsFile( Dirs::ConfigDir() + "HiRes_pair_parameters.yml" );
  
  // construct basenames and paths
  vector< string > basenames = getBasenames(Dirs::ImageList());
  vector< string > paths = constructPaths(Dirs::SliceDir(), basenames, ".bmp");
  
  // initialise stack with correct spacings, sizes, basenames etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType originalImages = readImages< StackType::SliceType >(paths);
  scaleImages< StackType::SliceType >(originalImages, getSpacings<2>("HiRes"));
  shared_ptr< StackType > originalStack = make_shared< StackType >(originalImages, getSpacings<3>("LoRes"), getSize(roi));
  originalStack->SetBasenames(basenames);
  
  Load(*originalStack, Dirs::HiResTransformsDir() + transform + "/");
  
  // move stack origins to ROI
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
  StackTransforms::Translate(*originalStack, translation);
  
  // generate images
  originalStack->updateVolumes();
  
  // construct 2 stacks from resampled images: 1:n-1 and 2:n
  StackType::SliceVectorType resampledImages = originalStack->GetResampledSlices();
  StackType::SliceVectorType fixedImages(++resampledImages.begin(), resampledImages.end());
  StackType::SliceVectorType movingImages(resampledImages.begin(), --resampledImages.end());
  StackType::MaskVectorType2D resampledMasks = originalStack->GetResampled2DMasks();
  StackType::MaskVectorType2D fixedMasks(++resampledMasks.begin(), resampledMasks.end());
  StackType::MaskVectorType2D movingMasks(resampledMasks.begin(), --resampledMasks.end());
  shared_ptr< StackType > fixedStack  = make_shared< StackType >(fixedImages,  fixedMasks,  getSpacings<3>("LoRes"), getSize(roi));
  shared_ptr< StackType > movingStack = make_shared< StackType >(movingImages, movingMasks, getSpacings<3>("LoRes"), getSize(roi));

  // Build registration
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder;
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  
  // Initialize both stacks with CenteredRigid2DTransform
  StackTransforms::InitializeToCommonCentre(*fixedStack);
  StackTransforms::InitializeToCommonCentre(*movingStack);
  
  // Configure moving centre and optimiser scales
  StackTransforms::SetMovingStackCenterWithFixedStack( *fixedStack, *movingStack );
  
  // Convert moving CenteredRigid2DTransform to AffineTransform
  typedef itk::CenteredAffineTransform< double, 2 > AffineTransformType;
  StackTransforms::InitializeFromCurrentTransforms< StackType, AffineTransformType >(*movingStack);
  
  OptimizerConfig::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );
  
  fixedStack->updateVolumes();
  movingStack->updateVolumes();
  string outputDir = Dirs::ResultsDir() + "HiResPairs/";
  create_directory( outputDir );
  writeImage< StackType::VolumeType >( movingStack->GetVolume(), outputDir + "moving_before.mha");
  
  // Get number of slices for registration and transform basenames
  unsigned int number_of_slices = fixedStack->GetSize();
  
  // Build movingStack's basenames
  vector< string > transformBasenames;
  for(unsigned int slice_number=0; slice_number < number_of_slices; ++slice_number)
  {
    // construct basenames e.g. "0001_0002"
    transformBasenames.push_back(basenames[slice_number] + "_" + basenames[slice_number + 1]);
  }
  movingStack->SetBasenames(transformBasenames);
  
  // Configure intermediate transform writer
  SimpleTransformWriter::Pointer simpleTransformWriter = SimpleTransformWriter::New();
  string intermediateTransformsDir = outputDir + "IntermediateTransforms/" + transform + "/";
  remove_all( intermediateTransformsDir );
  simpleTransformWriter->setOutputRootDir(intermediateTransformsDir);
  simpleTransformWriter->setStack(movingStack.get());
  registration->GetOptimizer()->AddObserver( itk::IterationEvent(), simpleTransformWriter );
  
  // Configure metric value writer
  MetricValueWriter::Pointer metricValueWriter = MetricValueWriter::New();
  string metricValueDir = outputDir + "MetricValues/" + transforms + "/";
  remove_all(metricValueDir);
  metricValueWriter->setOutputRootDir(metricValueDir);
  metricValueWriter->setStack(movingStack.get());
  registration->GetOptimizer()->AddObserver( itk::IterationEvent(), metricValueWriter );
  
  // Perform registration
  for(unsigned int slice_number=0; slice_number < number_of_slices; ++slice_number)
  {
    cout << "Registering slices " << basenames[slice_number] <<
      " and " << basenames[slice_number + 1] << "..." << endl;
      
    simpleTransformWriter->setSliceNumber(slice_number);
    metricValueWriter->setSliceNumber(slice_number);
    
    registration->SetFixedImage( fixedStack->GetOriginalImage(slice_number) );
    registration->SetMovingImage( movingStack->GetOriginalImage(slice_number) );
    registration->GetMetric()->SetFixedImageMask( fixedStack->GetOriginal2DMask(slice_number) );
    registration->GetMetric()->SetMovingImageMask( movingStack->GetOriginal2DMask(slice_number) );
    registration->SetTransform( movingStack->GetTransform(slice_number) );
    registration->SetInitialTransformParameters( movingStack->GetTransform(slice_number)->GetParameters() );
    
    try { registration->Update(); }
    catch( itk::ExceptionObject & err )
    { 
      std::cerr << "ExceptionObject caught !" << std::endl;
      std::cerr << err << std::endl;
      return EXIT_FAILURE;
    }
  }
  
  // Save transforms and inverse transforms
  string transformsDir = outputDir + "FinalTransforms/";
  create_directory(transformsDir);
  Save(*movingStack, transformsDir);
  
  // Write images
  fixedStack->updateVolumes();
  movingStack->updateVolumes();
  writeImage< StackType::VolumeType >( originalStack->GetVolume(), outputDir + "original.mha");
  writeImage< StackType::VolumeType >( fixedStack->GetVolume(), outputDir + "fixed.mha");
  writeImage< StackType::VolumeType >( movingStack->GetVolume(), outputDir + "moving_after.mha");
  writeImage< StackType::MaskVolumeType >( fixedStack->Get3DMask()->GetImage(), outputDir + "fixed_mask.mha");
  writeImage< StackType::MaskVolumeType >( movingStack->Get3DMask()->GetImage(), outputDir + "moving_mask.mha");
  
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
      ("transform", po::value<string>(), "Type of ITK transform to start from")
      ("roi", po::value<string>(), "region of interest e.g. papillary_insertion")
      ("fixedBasename", po::value<string>(), "basename of fixed slice e.g. 0196")
      ("movingBasename", po::value<string>(), "basename of moving slice e.g. 0197")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("transform", 1)
   .add("roi", 1);
  
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
  if(vm.count("help") ||
    !vm.count("dataSet") ||
    !vm.count("outputDir") ||
    !vm.count("transform") ||
    (vm.count("fixedBasename") != vm.count("movingBasename")) )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--transform=]CenteredAffineTransform"
      << " [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

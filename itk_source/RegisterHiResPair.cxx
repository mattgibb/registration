// Register 2 HiRes images to each other,
// initialised by loaded transforms from previous registration

using namespace boost::filesystem;
  
// function declarations
po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char const *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
  // Verify the number of parameters in the command line
  checkUsage(argc, argv);
  
  // Process command line arguments
  Dirs::SetDataSet(argv[1]);
  Dirs::SetOutputDirName(argv[2]);
  string fixedBasename( vm["fixedBasename"].as<string>() );
  string movingBasename( vm["movingBasename"].as<string>() );
  vector< string > fixedPath( 1, Dirs::SliceDir() + fixedBasename  + ".bmp");
  vector< string > movingPath(1, Dirs::SliceDir() + movingBasename + ".bmp");
  string roi = vm.count("roi") ? vm["roi"].as<string>() : "whole_heart";
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType FixedOriginalImage = readImages< StackType::SliceType >(fixedPath);
  StackType::SliceVectorType MovingOriginalImage = readImages< StackType::SliceType >(movingPath);
  boost::shared_ptr< StackType > FixedOriginalStack = InitializeHiResStack<StackType>(FixedOriginalImage, roi);
  boost::shared_ptr< StackType > MovingOriginalStack = InitializeHiResStack<StackType>(MovingOriginalImage, roi);
  FixedOriginalStack->SetBasenames(vector< string >(1, fixedBasename));
  MovingOriginalStack->SetBasenames(vector< string >(1, movingBasename));
  
  Load(*FixedOriginalStack,  Dirs::HiResTransformsDir(), fixedBasename);
  Load(*MovingOriginalStack, Dirs::HiResTransformsDir(), movingBasename);
  
  // move stack origins to ROI
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
  StackTransforms::Translate(*FixedOriginalStack, translation);
  StackTransforms::Translate(*MovingOriginalStack, translation);
  
  // generate images
  FixedOriginalStack->updateVolumes();
  MovingOriginalStack->updateVolumes();
  
  // Make stacks out of resampled slices
  StackType::SliceVectorType FixedImage, MovingImage;
  FixedImage.push_back(FixedOriginalStack->GetResampledSlice(0));
  MovingImage.push_back(MovingOriginalStack->GetResampledSlice(0));
  boost::shared_ptr< StackType > FixedStack = InitializeHiResStack<StackType>(FixedImage, roi);
  boost::shared_ptr< StackType > MovingStack = InitializeHiResStack<StackType>(MovingImage, roi);
  
  // Build registration
  boost::shared_ptr<YAML::Node> pParameters = config("HiRes_pair_parameters.yml");
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder(pParameters);
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  
  // SET AFFINE? TRANSFORM FOR MOVING STACK
  
  // Configure moving centre and optimiser scales
  StackTransforms::SetMovingStackCenterWithFixedStack( *FixedStack, *MovingStack );
  OptimizerConfig::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );
  
  // Perform registration
  registration->SetFixedImage( FixedOriginalStack->GetResampledSlice(0) );
  registration->SetMovingImage( MovingOriginalStack->GetResampledSlice(0) );
  registration->GetMetric()->SetFixedImageMask( FixedOriginalStack->GetResampled2DMask(0) );
  registration->GetMetric()->SetMovingImageMask( MovingOriginalStack->GetResampled2DMask(0) );
  registration->SetTransform( MovingStack->GetTransform(0) );
  registration->SetInitialTransformParameters( MovingStack->GetTransform(0)->GetParameters() );
  
  try { registration->Update(); }
  catch( itk::ExceptionObject & err ) 
  { 
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }
  
  // save transform
  string transformFile = Dirs::HiResPairTransformsDir() + fixedBasename + "_" + movingBasename;
  writeTransform(MovingStack->GetTransform(0), transformFile);
  
  // Write bmps
  using namespace boost::filesystem;
  create_directory( Dirs::SlicePairDir() );
  
  writeImage< StackType::VolumeType >( FixedOriginalStack->GetVolume(), (path( Dirs::SlicePairDir() ) / "LoRes.mha").string());
  writeImage< StackType::VolumeType >( MovingOriginalStack->GetVolume(), (path( Dirs::SlicePairDir() ) / "HiRes.mha").string());
  
  
  // create_directory(LoResTransformsDir);
  // create_directory(HiResTransformsDir);
  // Save(*FixedOriginalStack, LoResTransformsDir, fixedBasename);
  // Save(*MovingOriginalStack, HiResTransformsDir, movingBasename);
  
  return EXIT_SUCCESS;
}

void checkUsage(int argc, char const *argv[]) {
  if( argc < 5 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet outputDir fixedSlice movingSlice (roi)\n\n";
    exit(EXIT_FAILURE);
  }
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("dataSet", po::value<string>(), "which rat to use")
      ("outputDir", po::value<string>(), "directory to place results")
      ("fixedBasename", po::value<string>(), "basename of fixed slice")
      ("movingBasename", po::value<string>(), "basename of moving slice")
      ("outputTransform", po::value<string>(), "file name of the w ")
      ("roi", po::value<string>(), "region of interest")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
  .add("outputDir", 1)
  .add("fixedBasename", 1)
  .add("movingBasename", 1)
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
    !vm.count("fixedBasename") ||
    !vm.count("movingBasename") ||
    !vm.count("outputTransform") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--fixedBasename=]0196 [--movingBasename=]0197"
      << " [[--roi=]papillary_insertion] [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}


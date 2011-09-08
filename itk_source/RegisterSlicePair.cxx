void checkUsage(int argc, char const *argv[]) {
  if( argc < 5 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet outputDir fixedSlice movingSlice (roi)\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
  using namespace boost::filesystem;
  
  // Verify the number of parameters in the command line
  checkUsage(argc, argv);
  
  // Process command line arguments
  Dirs::SetDataSet(argv[1]);
  Dirs::SetOutputDirName(argv[2]);
  vector< string > FixedFileName, MovingFileName;
  FixedFileName.push_back(Dirs::SliceDir() + argv[3]);
  MovingFileName.push_back(Dirs::SliceDir() + argv[4]);
  string roi = argc >= 6 ? argv[5] : "whole_heart";
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType FixedOriginalImage = readImages< StackType >(FixedFileName);
  StackType::SliceVectorType MovingOriginalImage = readImages< StackType >(MovingFileName);
  boost::shared_ptr< StackType > FixedOriginalStack = InitializeHiResStack<StackType>(FixedOriginalImage, roi);
  boost::shared_ptr< StackType > MovingOriginalStack = InitializeHiResStack<StackType>(MovingOriginalImage, roi);
  
  Load(*FixedOriginalStack, FixedFileName, Dirs::HiResTransformsDir());
  Load(*MovingOriginalStack, MovingFileName, Dirs::HiResTransformsDir());
  
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
  
  // Register stacks
  StackTransforms::SetMovingStackCenterWithFixedStack( *FixedStack, *MovingStack );
  

  // save transform







  // Write bmps
  using namespace boost::filesystem;
  create_directory( Dirs::SlicePairDir() );
  
  writeImage< StackType::VolumeType >( FixedOriginalStack->GetVolume(), (path( Dirs::SlicePairDir() ) / "LoRes.mha").string());
  writeImage< StackType::VolumeType >( MovingOriginalStack->GetVolume(), (path( Dirs::SlicePairDir() ) / "HiRes.mha").string());





  
  // create_directory(LoResTransformsDir);
  // create_directory(HiResTransformsDir);
  // Save(*FixedOriginalStack, FixedFileName, LoResTransformsDir);
  // Save(*MovingOriginalStack, MovingFileName, HiResTransformsDir);
  
  return EXIT_SUCCESS;
}

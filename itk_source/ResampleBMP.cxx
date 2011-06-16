// Load transforms and build RGB volumes from images

#include "boost/filesystem.hpp"

#include "itkRGBPixel.h"
#include "itkVectorResampleImageFilter.h"

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
  if( argc < 3 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " dataSet outputDir (loResDSRatio hiResDSRatio roi)\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  Dirs::SetOutputDirName(argv[2]);
  
  // set region of interest
  string roi;
  if(argc >=6) roi = argv[5];
  else         roi = "whole_heart";
  
  // get file names
  vector< string > LoResFilePaths, HiResFilePaths;
  LoResFilePaths = getFilePaths(Dirs::BlockDir(), Dirs::SliceFile());
  HiResFilePaths = getFilePaths(Dirs::SliceDir(), Dirs::SliceFile());
	
  // initialise stack with correct spacings, sizes, transforms etc
  typedef itk::RGBPixel< unsigned char > PixelType;
  typedef Stack< PixelType, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(LoResFilePaths);
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFilePaths);
  boost::shared_ptr< StackType > LoResStack = InitializeLoResStack<StackType>(LoResImages, roi);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages, roi);
  
  HiResStack->SetDefaultPixelValue( 255 );
  
  // Load transforms from files
  // get downsample ratios
  string LoResDownsampleRatio, HiResDownsampleRatio;
  if( argc >= 5 )
  {
    LoResDownsampleRatio = argv[3];
    HiResDownsampleRatio = argv[4];
  }
  else
  {
    boost::shared_ptr<YAML::Node> downsample_ratios = config(Dirs::GetDataSet() + "/downsample_ratios.yml");
    (*downsample_ratios)["LoRes"] >> LoResDownsampleRatio;
    (*downsample_ratios)["HiRes"] >> HiResDownsampleRatio;
  }
  
  // read transforms from directories labeled by both ds ratios
  using namespace boost::filesystem;
  string LoResTransformsDir = Dirs::ResultsDir() + "LoResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  string HiResTransformsDir = Dirs::ResultsDir() + "HiResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
    
  Load(*LoResStack, LoResFilePaths, LoResTransformsDir);
  Load(*HiResStack, HiResFilePaths, HiResTransformsDir);
  
  // move stack origins to ROI
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
  StackTransforms::Translate(*LoResStack, translation);
  StackTransforms::Translate(*HiResStack, translation);
  
  // generate images
  LoResStack->updateVolumes();
  HiResStack->updateVolumes();
  
  // Write bmps
  using namespace boost::filesystem;
  create_directory( Dirs::ColourDir() );
  
  writeImage< StackType::VolumeType >( LoResStack->GetVolume(), (path( Dirs::ColourDir() ) / "LoRes.mha").string());
  writeImage< StackType::VolumeType >( HiResStack->GetVolume(), (path( Dirs::ColourDir() ) / "HiRes.mha").string());
  
  return EXIT_SUCCESS;
}

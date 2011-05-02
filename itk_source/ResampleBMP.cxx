#include "boost/filesystem.hpp"

#include <assert.h>
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
    cerr << argv[0] << " dataSet outputDir (loResDSRatio hiResDSRatio)\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  string outputDir(Dirs::ResultsDir() + argv[2] + "/");
  
  // get file names
  vector< string > HiResFilePaths, HiResFileNames;
  HiResFileNames = getFileNames(Dirs::SliceFile());
  HiResFilePaths = getFilePaths(Dirs::SliceDir(), Dirs::SliceFile());
	
  // initialise stack with correct spacings, sizes, transforms etc
  typedef itk::RGBPixel< unsigned char > PixelType;
  typedef Stack< PixelType, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFilePaths);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages);
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
  string LoResTransformsDir = outputDir + "LoResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  string HiResTransformsDir = outputDir + "HiResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  
  Load(*HiResStack, HiResFilePaths, HiResTransformsDir);
  HiResStack->updateVolumes();
  
  // Write bmps
  using namespace boost::filesystem;
  path HiResBMPDir = outputDir + "ColourResamples_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  create_directory(HiResBMPDir);
  
  writeImage< StackType::VolumeType >( HiResStack->GetVolume(), (HiResBMPDir / "volume.mha").string());
  
  return EXIT_SUCCESS;
}

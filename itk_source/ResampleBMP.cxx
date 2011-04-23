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
    cerr << argv[0] << " dataSet outputDir (slice)\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet(argv[1]);
  string outputDir(Dirs::ResultsDir() + argv[2] + "/");
  vector< string > HiResFilePaths, HiResFileNames;
  if( argc >= 4)
  {
    HiResFileNames.push_back(argv[3]);
    HiResFilePaths.push_back(Dirs::SliceDir() + argv[3]);
  }
  else
  {
    HiResFileNames = getFileNames(Dirs::SliceFile());
    HiResFilePaths = getFilePaths(Dirs::SliceDir(), Dirs::SliceFile());
  }
	
  // initialise stack with correct spacings, sizes, transforms etc
  typedef Stack< itk::RGBPixel< unsigned char >, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFilePaths);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages);
  
  // Load transforms from files
  // get downsample ratios
  boost::shared_ptr<YAML::Node> downsample_ratios = config(Dirs::GetDataSet() + "/downsample_ratios.yml");
  string LoResDownsampleRatio, HiResDownsampleRatio;
  (*downsample_ratios)["LoRes"] >> LoResDownsampleRatio;
  (*downsample_ratios)["HiRes"] >> HiResDownsampleRatio;
  
  // write transforms to directories labeled by both ds ratios
  using namespace boost::filesystem;
  string LoResTransformsDir = outputDir + "LoResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  string HiResTransformsDir = outputDir + "HiResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  
  Load(*HiResStack, HiResFilePaths, HiResTransformsDir);
  HiResStack->updateVolumes();
  
  // Write bmps
  using namespace boost::filesystem;
  path HiResBMPDir = outputDir + "ColourResamples";
  create_directory(HiResBMPDir);
  for(unsigned int slice_number=0; slice_number < HiResStack->GetSize(); ++slice_number)
  {
    writeImage< StackType::SliceType >( HiResStack->GetResampledSlice(slice_number), (HiResBMPDir / HiResFileNames[slice_number]).string() );
  }
  
  return EXIT_SUCCESS;
}

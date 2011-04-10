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
  vector< string > HiResFileNames;
  if( argc >= 4)
  {
    HiResFileNames.push_back(Dirs::SliceDir() + argv[3]);
  }
  else
  {
    HiResFileNames = getFileNames(Dirs::SliceDir(), Dirs::SliceFile());
  }
	
  // initialise stack with correct spacings, sizes, transforms etc
  typedef Stack< itk::RGBPixel< unsigned char >, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFileNames);
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages);
  
  Load(*HiResStack, HiResFileNames, outputDir + "HiResTransforms");
  HiResStack->updateVolumes();
  
  // Write bmps
  using namespace boost::filesystem;
  string HiResBMPDir = outputDir + "ColourResamples";
  create_directory(HiResBMPDir);
  for(unsigned int slice_number=0; slice_number < HiResStack->GetSize(); ++slice_number)
  {
    writeImage< StackType::SliceType >( HiResStack->GetResampledSlice(slice_number), outputDir + HiResFileNames[slice_number] );
  }
  
  return EXIT_SUCCESS;
}

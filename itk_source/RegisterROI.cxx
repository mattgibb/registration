#include "boost/filesystem.hpp"

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
  vector< string > LoResFileNames, HiResFileNames;
  if( argc >= 4)
  {
    LoResFileNames.push_back(Dirs::BlockDir() + argv[3]);
    HiResFileNames.push_back(Dirs::SliceDir() + argv[3]);
  }
  else
  {
    LoResFileNames = getFilePaths(Dirs::BlockDir(), Dirs::SliceFile());
    HiResFileNames = getFilePaths(Dirs::SliceDir(), Dirs::SliceFile());
  }
	
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(LoResFileNames);
  StackType::SliceVectorType HiResImages = readImages< StackType >(HiResFileNames);
  normalizeImages< StackType >(LoResImages);
  normalizeImages< StackType >(HiResImages);
  boost::shared_ptr< StackType > LoResStack = InitializeLoResStack<StackType>(LoResImages, "ROI");
  boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImages, "ROI");
  
  // initialise stacks' transforms with saved transform files
  Load(*LoResStack, LoResFileNames, outputDir + "LoResTransforms");
  Load(*HiResStack, HiResFileNames, outputDir + "HiResTransforms");
  
  // move stack origins to ROI
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation("ROI") - StackTransforms::GetLoResTranslation("whole_heart");
  StackTransforms::Translate(*LoResStack, translation);
  StackTransforms::Translate(*HiResStack, translation);
  StackTransforms::SetMovingStackCenterWithFixedStack( *LoResStack, *HiResStack );
  
  // Generate fixed images to register against
  LoResStack->updateVolumes();
  if( argc < 4)
  {
    writeImage< StackType::VolumeType >( LoResStack->GetVolume(), outputDir + "LoResROI.mha" );
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), outputDir + "HiResInitialROI.mha" );
  }
  
  // initialise registration framework
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder;
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner< StackType > stackAligner(*LoResStack, *HiResStack, registration);
  
  // make sure loaded transforms are centered affine
  typedef itk::CenteredAffineTransform< double, 2 > AffineTransformType;
  AffineTransformType::Pointer affineTransform = dynamic_cast< AffineTransformType* >( HiResStack->GetTransform(0).GetPointer() );
  assert( affineTransform );
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );

  itkProbesCreate();
  itkProbesStart( "Aligning stacks" );
  stackAligner.Update();
  itkProbesStop( "Aligning stacks" );
  itkProbesReport( std::cout );
  
  if(argc < 4)
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), outputDir + "HiResAffineROI.mha" );
    // writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), outputDir + "HiResAffineMask.mha" );
  }
  
  // Update LoRes as the masks might have shrunk
  LoResStack->updateVolumes();
  
  // Write final transforms to file
  using namespace boost::filesystem;
  string LoResTransformsDir = outputDir + "LoResROITransforms";
  string HiResTransformsDir = outputDir + "HiResROITransforms";
  create_directory(LoResTransformsDir);
  create_directory(HiResTransformsDir);
  Save(*LoResStack, LoResFileNames, LoResTransformsDir);
  Save(*HiResStack, HiResFileNames, HiResTransformsDir);
  
  return EXIT_SUCCESS;
}

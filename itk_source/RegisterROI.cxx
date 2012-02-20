#include "boost/filesystem.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <assert.h>
#include "itkCenteredSimilarity2DTransform.h"

// my files
#include "Stack.hpp"
#include "NormalizeImages.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "StackTransforms.hpp"
#include "OptimizerConfig.hpp"
#include "Profiling.hpp"

using namespace boost::filesystem;
using namespace boost;


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
  Dirs::SetOutputDirName(argv[2]);
  
  // basenames is either single name from command line
  // or list from config file
  vector< string > basenames = argc >= 4 ?
                               vector< string >(1, argv[3]) :
                               getBasenames(Dirs::ImageList());
  
  // prepend directory to each filename in list
  vector< string > LoResFilePaths = constructPaths(Dirs::BlockDir(), basenames, ".bmp");
  vector< string > HiResFilePaths = constructPaths(Dirs::SliceDir(), basenames, ".bmp");
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType::SliceType >(LoResFilePaths);
  StackType::SliceVectorType HiResImages = readImages< StackType::SliceType >(HiResFilePaths);
  normalizeImages< StackType::SliceType >(LoResImages);
  normalizeImages< StackType::SliceType >(HiResImages);
  shared_ptr< StackType > LoResStack = make_shared< StackType >(LoResImages, getSpacings<3>("LoRes"), getSize());
  shared_ptr< StackType > HiResStack = make_shared< StackType >(HiResImages, getSpacings<3>("LoRes"), getSize());
  LoResStack->SetBasenames(basenames);
  HiResStack->SetBasenames(basenames);
  
  // initialise stacks' transforms with saved transform files
  Load(*LoResStack, Dirs::LoResTransformsDir());
  Load(*HiResStack, Dirs::HiResTransformsDir());
  
  // move stack origins to ROI
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation("ROI") - StackTransforms::GetLoResTranslation("whole_heart");
  StackTransforms::Translate(*LoResStack, translation);
  StackTransforms::Translate(*HiResStack, translation);
  StackTransforms::SetMovingStackCenterWithFixedStack( *LoResStack, *HiResStack );
  
  // Generate fixed images to register against
  LoResStack->updateVolumes();
  if( argc < 4)
  {
    writeImage< StackType::VolumeType >( LoResStack->GetVolume(), Dirs::ResultsDir() + "LoResROI.mha" );
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResInitialROI.mha" );
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
  OptimizerConfig::SetOptimizerScalesForCenteredAffineTransform( registration->GetOptimizer() );

  itkProbesCreate();
  itkProbesStart( "Aligning stacks" );
  stackAligner.Update();
  itkProbesStop( "Aligning stacks" );
  itkProbesReport( std::cout );
  
  if(argc < 4)
  {
    HiResStack->updateVolumes();
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ResultsDir() + "HiResAffineROI.mha" );
    // writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), Dirs::ResultsDir() + "HiResAffineMask.mha" );
  }
  
  // Update LoRes as the masks might have shrunk
  LoResStack->updateVolumes();
  
  // write transforms to directories labeled by both ds ratios
  create_directory(Dirs::LoResTransformsDir());
  create_directory(Dirs::HiResTransformsDir());
  Save(*LoResStack, Dirs::LoResTransformsDir());
  Save(*HiResStack, Dirs::HiResTransformsDir());
  
  return EXIT_SUCCESS;
}

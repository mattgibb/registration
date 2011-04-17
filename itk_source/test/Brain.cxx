#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"

int main(int argc, char const *argv[]) {
  Dirs::SetParamsFile(Dirs::TestDir() + "data/registration_parameters.yml");
  string outputDir(Dirs::TestDir() + "data/results/");
	
	// initialise stack objects
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
	StackType::VolumeType::SpacingType LoResSpacings, HiResSpacings;
  for(unsigned int i=0; i<3; i++)
  {
    LoResSpacings[i] = HiResSpacings[i] = 1;
  }
  
  // make 2D version of HiResSpacings
  StackType::SliceType::SpacingType HiResOriginalSpacings;
  for(unsigned int i=0; i<2; i++) HiResOriginalSpacings[i] = HiResSpacings[i];
  
  vector< string > original, rotated;
  original.push_back(Dirs::TestDir() + "data/images/original.png");
  rotated.push_back(Dirs::TestDir() + "data/images/rotated.png");
  
  // initialise stack objects with correct spacings, sizes etc
  typedef Stack< float, itk::ResampleImageFilter, itk::LinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages = readImages< StackType >(original);
  StackType::SliceVectorType HiResImages = readImages< StackType >(rotated);
  boost::shared_ptr< StackType > LoResStack = boost::make_shared< StackType >(LoResImages, LoResSpacings);
  boost::shared_ptr< StackType > HiResStack = boost::make_shared< StackType >(HiResImages, HiResOriginalSpacings,
            LoResStack->GetSpacings(), LoResStack->GetResamplerSize());
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  StackTransforms::InitializeToCommonCentre( *LoResStack );
  StackTransforms::InitializeToCommonCentre( *HiResStack );
  StackTransforms::SetMovingStackCenterWithFixedStack( *LoResStack, *HiResStack );
  
  LoResStack->updateVolumes();
  HiResStack->updateVolumes();
  
  // initialise registration framework
  typedef RegistrationBuilder< StackType > RegistrationBuilderType;
  RegistrationBuilderType registrationBuilder;
  RegistrationBuilderType::RegistrationType::Pointer registration = registrationBuilder.GetRegistration();
  StackAligner< StackType > stackAligner(*LoResStack, *HiResStack, registration);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredRigid2DTransform( registration->GetOptimizer() );
  
	// perform centered rigid 2D registration
  stackAligner.Update();
  
  HiResStack->updateVolumes();
  
  // write transform and results
  writeData< itk::TransformFileWriter, StackType::TransformType >
    (HiResStack->GetTransform(0), outputDir + "Transforms.meta");
  
  cout << "outputDir: " << outputDir << endl;
  
	writeImage< StackType::VolumeType >( LoResStack->GetVolume(), outputDir + "LoResStack.mha" );
  writeImage< StackType::MaskVolumeType >( LoResStack->Get3DMask()->GetImage(), outputDir + "LoResMask.mha" );
	writeImage< StackType::VolumeType >( HiResStack->GetVolume(), outputDir + "HiResStack.mha" );
  writeImage< StackType::MaskVolumeType >( HiResStack->Get3DMask()->GetImage(), outputDir + "HiResMask.mha" );
	
  return EXIT_SUCCESS;
}

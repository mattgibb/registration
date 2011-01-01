// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "Framework2DRat.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "RegistrationParameters.hpp"

int main(int argc, char const *argv[]) {
  string outputDir(Dirs::TestDir() + "data/results/");
	
	// initialise stack objects
	Stack::VolumeType::SpacingType LoResSpacings, HiResSpacings;
  for(unsigned int i=0; i<3; i++)
  {
    LoResSpacings[i] = HiResSpacings[i] = 1;
  }
  
  vector< string > original, rotated;
  original.push_back(Dirs::TestDir() + "data/images/original.png");
  rotated.push_back(Dirs::TestDir() + "data/images/rotated.png");
  Stack LoResStack( original, LoResSpacings );
  
  // make 2D version of HiResSpacings
  Stack::SliceType::SpacingType HiResOriginalSpacings;
  for(unsigned int i=0; i<2; i++) HiResOriginalSpacings[i] = HiResSpacings[i];
  
  Stack HiResStack(rotated, HiResOriginalSpacings,
        LoResStack.GetSpacings(), LoResStack.GetResamplerSize());
  
  // initialize stacks' transforms so that 2D images line up at their centres.
  StackTransforms::InitializeToCommonCentre( LoResStack );
  StackTransforms::InitializeToCommonCentre( HiResStack );
  StackTransforms::SetMovingStackCORWithFixedStack( LoResStack, HiResStack );
  
  LoResStack.updateVolumes();
  HiResStack.updateVolumes();
  
  Framework2DRat framework2DRat(LoResStack, HiResStack);
  
  // Scale parameter space
  StackTransforms::SetOptimizerScalesForCenteredRigid2DTransform( framework2DRat.GetOptimizer() );
  
	// perform centered rigid 2D registration
  framework2DRat.StartRegistration();
  
  HiResStack.updateVolumes();
  
  // write transform and results
  writeData< itk::TransformFileWriter, Stack::TransformType >
    (HiResStack.GetTransform(0), outputDir + "Transforms.meta");
  
  cout << "outputDir: " << outputDir << endl;
  
	writeImage< Stack::VolumeType >( LoResStack.GetVolume(), outputDir + "LoResStack.mha" );
  writeImage< Stack::MaskVolumeType >( LoResStack.Get3DMask()->GetImage(), outputDir + "LoResMask.mha" );
	writeImage< Stack::VolumeType >( HiResStack.GetVolume(), outputDir + "HiResStack.mha" );
  writeImage< Stack::MaskVolumeType >( HiResStack.Get3DMask()->GetImage(), outputDir + "HiResMask.mha" );
	
  return EXIT_SUCCESS;
}

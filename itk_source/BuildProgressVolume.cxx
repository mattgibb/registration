// Compose a volume from the moving image slices
// at each progressive optimisation step

// boost
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

// itk
#include "itkRGBPixel.h"
#include "itkVectorResampleImageFilter.h"

// my files
#include "Stack.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "ScaleImages.hpp"

namespace po = boost::program_options;
using namespace boost::filesystem;
using namespace boost;

typedef itk::RGBPixel< unsigned char > PixelType;
typedef Stack< PixelType, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;

// function declarations
po::variables_map parse_arguments(int argc, char *argv[]);
vector< string > directoryContents(const string& directory);
shared_ptr< StackType > buildLoResFixedStack    (StackType::SliceVectorType image, const string& basename);
shared_ptr< StackType > buildHiResMovingStack   (StackType::SliceVectorType image);
shared_ptr< StackType > buildAdjacentFixedStack (StackType::SliceVectorType image);
shared_ptr< StackType > buildAdjacentMovingStack(StackType::SliceVectorType image);


int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  bool buildFixedVolume = vm.count("buildFixedVolume");
  bool adjacentPairs = vm.count("adjacentPairs");
  string transform = vm["transform"].as<string>();
  string transformDirectory = Dirs::IntermediateTransformsDir() + transform + "/";
  vector< string > slicePairs = vm.count("slicePair") ?
                                        vector< string >(1, vm["slicePair"].as<string>()) :
                                        directoryContents(transformDirectory);
  
  // construct fixed and moving image paths
  // if building adjacent pair volumes, then extract the two slices
  // if building HiRes against LoRes volumes, then just use slicePairs
  vector< string > fixedBasenames, fixedPaths, movingPaths;
  
  if(adjacentPairs)
  {
    for(vector< string >::iterator it=slicePairs.begin(); it!=slicePairs.end(); ++it)
    {
      fixedBasenames.push_back(it->substr(0,4));
      fixedPaths.push_back (Dirs::SliceDir() + it->substr(0,4) + ".bmp");
      movingPaths.push_back(Dirs::SliceDir() + it->substr(5,4) + ".bmp");
    }
  }
  else
  {
    fixedBasenames = slicePairs;
    fixedPaths  = constructPaths(Dirs::BlockDir(), slicePairs, ".bmp");
    movingPaths = constructPaths(Dirs::SliceDir(), slicePairs, ".bmp");
  }
  
  // Generate the progress volumes, and maybe their associated reference volumes
  // 1) Load the stack
  // 2) Load the transforms
  // 3) Write the volumes
  for(unsigned int i=0; i<slicePairs.size(); ++i)
  {
    // Load transforms from files
    string stepsDirectory = transformDirectory +
                            slicePairs[i] + "/";
    
    vector< string > steps = directoryContents(stepsDirectory);
    
    // initialise stack with correct spacings, sizes, transforms etc
    cout << "Building " << slicePairs[i] << " moving progress volume...";
    StackType::SliceVectorType movingImage(steps.size(), readImage< StackType::SliceType >(movingPaths[i]));
    
    shared_ptr< StackType > movingStack = adjacentPairs ?
                                          buildAdjacentMovingStack(movingImage) :
                                          buildHiResMovingStack(movingImage);
    movingStack->SetBasenames(steps);
    
    // load transform at each iteration
    Load(*movingStack, stepsDirectory);
    
    movingStack->SetDefaultPixelValue( 255 );
    
    // generate images
    movingStack->updateVolumes();
    
    // Write bmps
    writeImage< StackType::VolumeType >( movingStack->GetVolume(), stepsDirectory + "moving.mha");
    
    cout << "done." << endl;
    
    // generate comparison volume
    if(buildFixedVolume)
    {
      cout << "Building " << slicePairs[i] << " fixed comparison volume..." << flush;
      
      StackType::SliceVectorType fixedImage(steps.size(), readImage< StackType::SliceType >(fixedPaths[i]) );
      shared_ptr< StackType > fixedStack = adjacentPairs ?
                                           buildAdjacentFixedStack(fixedImage) :
                                           buildLoResFixedStack(fixedImage, fixedBasenames[i]);
      
      // load comparison transforms
      fixedStack->updateVolumes();
      writeImage< StackType::VolumeType >( fixedStack->GetVolume(), stepsDirectory + "fixed.mha");
      cout << "done." << endl;
    }
  }
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("dataSet", po::value<string>(), "which rat to use")
      ("outputDir", po::value<string>(), "directory to place results")
      ("transform", po::value<string>(), "Type of ITK transform that was optimised")
      ("slicePair", po::value<string>(), "the slice pair identifier e.g. 0001 for LoRes/HiRes, 0001_0002 for adjacent pairs")
      ("buildFixedVolume,f", po::value<bool>()->zero_tokens(), "generate fixed image comparison volume")
      ("adjacentPairs", po::value<bool>()->zero_tokens(), "Generate volumes for adjacent pair registration")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("transform", 1);
     
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
    !vm.count("transform") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--transform=]CenteredRigid2DTransform "
      << " [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

vector< string > directoryContents(const string& directory)
{
  // retrieve and sort paths
  vector< path > contents;
  try
  {
    copy(directory_iterator(directory), directory_iterator(), back_inserter(contents));
  }
  catch(std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  sort(contents.begin(), contents.end());
  
  // extract leaves
  vector< string > contents_strings;
  for(vector< path >::const_iterator it = contents.begin(); it != contents.end(); ++it)
  {
    contents_strings.push_back(it->leaf().string());
  }
  
  return contents_strings;
}

shared_ptr< StackType > buildLoResFixedStack(StackType::SliceVectorType image, const string& basename)
{
  scaleImages< StackType::SliceType >(image, getSpacings<2>("LoRes"));
  shared_ptr< StackType > stack = make_shared< StackType >(image, getSpacings<3>("LoRes"), getSize());
  StackTransforms::InitializeWithTranslation( *stack, StackTransforms::GetLoResTranslation("whole_heart") );
  stack->SetBasenames(basename);
  ApplyAdjustments( *stack, Dirs::ConfigDir() + "LoRes_adjustments/");
  return stack;
}

shared_ptr< StackType > buildHiResMovingStack(StackType::SliceVectorType image)
{
  scaleImages< StackType::SliceType >(image, getSpacings<2>("HiRes"));
  return make_shared< StackType >(image, getSpacings<3>("LoRes"), getSize());
}

shared_ptr< StackType > buildAdjacentFixedStack(StackType::SliceVectorType image)
{
  
}

shared_ptr< StackType > buildAdjacentMovingStack(StackType::SliceVectorType image)
{
  // // resample HiRes slice
  // // build stack of resampled slice
  // StackType::SliceVectorType movingImages(steps.size(), originalImage->GetResampledSlices()[0]);
  // 
  // shared_ptr< StackType > originalStack = make_shared< StackType >(originalImages, getSpacings<3>("LoRes"), getSize(roi));
  // originalStack->SetBasenames(slicePairs);
  // 
  // Load(*originalStack, Dirs::HiResTransformsDir() + "CenteredAffineTransform/");
  // 
  // // move stack origins to ROI
  // itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
  // StackTransforms::Translate(*originalStack, translation);
  // 
  // // generate images
  // originalStack->updateVolumes();
  // 
  // shared_ptr< StackType > movingStack = make_shared< StackType >(movingImages, getSpacings<3>("LoRes"))
  // make_shared< StackType >(movingImages, movingSpacings, getSize());
  // 
  // scaleImages< StackType::SliceType >(originalImages, getSpacings<2>("HiRes"));
  // shared_ptr< StackType > originalStack = make_shared< StackType >(originalImages, getSpacings<3>("LoRes"), getSize(roi));
  // 
}


// Compose a volume from the moving image slices
// at each progressive optimisation step

// boost
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

// itk
#include "itkRGBPixel.h"
#include "itkVectorResampleImageFilter.h"

// my files
#include "Stack.hpp"
#include "StackInitializers.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"

namespace po = boost::program_options;
using namespace boost::filesystem;


// function declarations
po::variables_map parse_arguments(int argc, char *argv[]);
vector< string > directoryContents(const string& directory);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  bool LoRes = vm.count("LoRes");
  string transform = vm["transform"].as<string>();
  string transformDirectory = Dirs::IntermediateTransformsDir() + transform + "/";
  vector< string > basenames = vm.count("basename") ?
                               vector< string >(1, vm["basename"].as<string>()) :
                               directoryContents(transformDirectory);
  
  for(vector<string>::const_iterator basename = basenames.begin(); basename != basenames.end(); ++basename)
  {
    // get image paths
    string HiResPath = Dirs::SliceDir() + *basename + ".bmp";
	
    // Load transforms from files
    string stepsDirectory = transformDirectory + 
                            *basename + "/";
  
    vector< string > steps = directoryContents(stepsDirectory);
  
    // initialise stack with correct spacings, sizes, transforms etc
    cout << "Building HiRes progress volume for " << *basename << "...";
    typedef itk::RGBPixel< unsigned char > PixelType;
    typedef Stack< PixelType, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
    StackType::SliceVectorType HiResImage(steps.size(), readImage< StackType::SliceType >(HiResPath));
    boost::shared_ptr< StackType > HiResStack = InitializeHiResStack<StackType>(HiResImage);
    HiResStack->SetBasenames(steps);
    HiResStack->SetDefaultPixelValue( 255 );
  
    // load transform at each iteration
    Load(*HiResStack, stepsDirectory);
  
    // move stack origins to ROI
    // itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
    // if(LoRes) StackTransforms::Translate(*LoResStack, translation);
    // if(HiRes) StackTransforms::Translate(*HiResStack, translation);
  
    // generate images
    HiResStack->updateVolumes();
  
    // Write bmps
    writeImage< StackType::VolumeType >( HiResStack->GetVolume(), stepsDirectory + "HiRes.mha");
  
    cout << "done." << endl;
  
    // generate LoRes volume for comparison
    if(LoRes)
    {
      cout << "Building LoRes comparison volume...";
      string LoResPath = Dirs::BlockDir() + *basename + ".bmp";
      StackType::SliceVectorType LoResImage = StackType::SliceVectorType(steps.size(), readImage< StackType::SliceType >(LoResPath) );
      boost::shared_ptr< StackType > LoResStack = InitializeLoResStack<StackType>(LoResImage);
      LoResStack->SetBasenames(vector< string >(steps.size(), *basename));
      // load LoRes transforms
      StackTransforms::InitializeWithTranslation( *LoResStack, StackTransforms::GetLoResTranslation("whole_heart") );
      ApplyAdjustments( *LoResStack, Dirs::ConfigDir() + "LoRes_adjustments/");
      LoResStack->updateVolumes();
      writeImage< StackType::VolumeType >( LoResStack->GetVolume(), stepsDirectory + "LoRes.mha");
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
      ("basename", po::value<string>(), "the particular slice identifier e.g. 0196")
      ("LoRes,L", po::value<bool>()->zero_tokens(), "generate LoRes image")
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
  copy(directory_iterator(directory), directory_iterator(), back_inserter(contents));
  sort(contents.begin(), contents.end());
  
  // 
  vector< string > contents_strings;
  for(vector< path >::const_iterator it = contents.begin(); it != contents.end(); ++it)
  {
    contents_strings.push_back(it->leaf().string());
  }
  return contents_strings;
}

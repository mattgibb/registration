// Load transforms and build RGB volumes from images

#include "itkRGBPixel.h"
#include "itkVectorResampleImageFilter.h"

// my files
#include "Stack.hpp"
#include "RegistrationBuilder.hpp"
#include "StackAligner.hpp"
#include "StackIOHelpers.hpp"
#include "IOHelpers.hpp"
#include "StackTransforms.hpp"
#include "Dirs.hpp"
#include "Parameters.hpp"
#include "Profiling.hpp"
#include "ScaleImages.hpp"

// boost
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace po = boost::program_options;
using namespace boost::filesystem;
using namespace boost;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  string blockDir = vm.count("blockDir") ? vm["blockDir"].as<string>() + "/" : Dirs::BlockDir();
  bool LoRes = !vm.count("no-LoRes");
  bool HiRes = !vm["no-HiRes"].as<bool>();
  string roi = vm["roi"].as<string>();
  
  // get file names
  vector< string > basenames = getBasenames(Dirs::ImageList());
  vector< string > LoResFilePaths, HiResFilePaths;
  if(LoRes) LoResFilePaths = constructPaths(blockDir,         basenames, ".bmp");
  if(HiRes) HiResFilePaths = constructPaths(Dirs::SliceDir(), basenames, ".bmp");
	
  // initialise stack with correct spacings, sizes, transforms etc
  typedef itk::RGBPixel< unsigned char > PixelType;
  typedef Stack< PixelType, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages, HiResImages;
  if(LoRes) LoResImages = readImages< StackType::SliceType >(LoResFilePaths);
  if(HiRes) HiResImages = readImages< StackType::SliceType >(HiResFilePaths);
  if(LoRes) scaleImages< StackType::SliceType >(LoResImages, getSpacings<2>("LoRes"));
  if(HiRes) scaleImages< StackType::SliceType >(HiResImages, getSpacings<2>("HiRes"));
  shared_ptr< StackType > LoResStack, HiResStack;
  if(LoRes) LoResStack = make_shared< StackType >(LoResImages, getSpacings<3>("LoRes"), getSize(roi));
  if(HiRes) HiResStack = make_shared< StackType >(HiResImages, getSpacings<3>("LoRes"), getSize(roi));
  if(LoRes) LoResStack->SetBasenames(basenames);
  if(HiRes) HiResStack->SetBasenames(basenames);
  if(HiRes) HiResStack->SetDefaultPixelValue( 255 );
  
  // Load transforms from files
  // get downsample ratios
  string LoResDownsampleRatio, HiResDownsampleRatio;
  if( vm.count("loResRatio") && vm.count("hiResRatio") )
  {
    LoResDownsampleRatio = vm["loResRatio"].as<string>();
    HiResDownsampleRatio = vm["hiResRatio"].as<string>();
  }
  else
  {
    shared_ptr<YAML::Node> downsample_ratios = config("downsample_ratios.yml");
    (*downsample_ratios)["LoRes"] >> LoResDownsampleRatio;
    (*downsample_ratios)["HiRes"] >> HiResDownsampleRatio;
  }
  
  // read transforms from directories labeled by both ds ratios
  string LoResTransformsDir = Dirs::ResultsDir() + "LoResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio;
  string HiResTransformsDir = Dirs::ResultsDir() + "HiResTransforms_" + LoResDownsampleRatio + "_" + HiResDownsampleRatio
                              + "/CenteredAffineTransform";
  
  if(LoRes) Load(*LoResStack, LoResTransformsDir);
  if(HiRes) Load(*HiResStack, HiResTransformsDir);
  
  // move stack origins to ROI
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
  if(LoRes) StackTransforms::Translate(*LoResStack, translation);
  if(HiRes) StackTransforms::Translate(*HiResStack, translation);
  
  // generate images
  if(LoRes) LoResStack->updateVolumes();
  if(HiRes) HiResStack->updateVolumes();
  
  // Write bmps
  create_directory( Dirs::ColourDir() );
  
  if(LoRes) writeImage< StackType::VolumeType >( LoResStack->GetVolume(), (path( Dirs::ColourDir() ) / "LoRes.mha").string());
  if(HiRes) writeImage< StackType::VolumeType >( HiResStack->GetVolume(), (path( Dirs::ColourDir() ) / "HiRes.mha").string());
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("dataSet", po::value<string>(), "which rat to use")
      ("outputDir", po::value<string>(), "directory to source transforms and place results")
      ("roi", po::value<string>()->default_value("whole_heart"), "set region of interest")
      ("loResRatio", po::value<string>(), "LoRes ratio used to source transforms")
      ("hiResRatio", po::value<string>(), "HiRes ratio used to source transforms")
      ("blockDir", po::value<string>(), "directory containing LoRes originals")

      // three different ways of not specifying value for flag
      // implicit_value(true) allows syntax like -H0 and --no-HiRes=0,
      // rather than just the flag e.g. -H or --no-HiRes
      ("no-LoRes,L", po::value<bool>()->zero_tokens(), "do not generate LoRes image")
      ("no-HiRes,H", po::bool_switch(), "do not generate HiRes image")
      // ("no-HiRes,H", po::value(&noHiRes)->implicit_value(true), "do not generate HiRes image")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1);
  
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
  if (vm.count("help") || !vm.count("dataSet") || !vm.count("outputDir")) {
    cerr << "Usage: "
      << argv[0] << " [--dataSet=]RatX [--outputDir=]my_dir [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

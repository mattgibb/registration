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
  string hiResName = vm["hiResName"].as<string>();
  
  // get file names
  vector< string > basenames = vm.count("slice") ? vector<string>( 1, vm["slice"].as<string>() ) : getBasenames(Dirs::ImageList());
  vector< string > LoResFilePaths, HiResFilePaths;
	
  // initialise stacks with correct spacings, sizes, transforms etc
  typedef itk::RGBPixel< unsigned char > PixelType;
  typedef Stack< PixelType, itk::VectorResampleImageFilter, itk::VectorLinearInterpolateImageFunction > StackType;
  StackType::SliceVectorType LoResImages, HiResImages;
  shared_ptr< StackType > LoResStack, HiResStack;
  if(LoRes)
  {
    cout << "Creating LoRes stack..." << flush;
    LoResFilePaths = constructPaths(blockDir, basenames, ".bmp");
    LoResImages = readImages< StackType::SliceType >(LoResFilePaths);
    scaleImages< StackType::SliceType >(LoResImages, getSpacings<2>("LoRes"));
    LoResStack = make_shared< StackType >(LoResImages, getSpacings<3>("LoRes"), getSize(roi));
    LoResStack->SetBasenames(basenames);
    cout << "done." << endl;
  }
  if(HiRes)
  {
    cout << "Creating HiRes stack..." << flush;
    HiResFilePaths = constructPaths(Dirs::SliceDir(), basenames, ".bmp");
    HiResImages = readImages< StackType::SliceType >(HiResFilePaths);
    scaleImages< StackType::SliceType >(HiResImages, getSpacings<2>("HiRes"));
    HiResStack = make_shared< StackType >(HiResImages, getSpacings<3>("LoRes"), getSize(roi));
    HiResStack->SetBasenames(basenames);
    HiResStack->SetDefaultPixelValue( vm["defaultPixelValue"].as<unsigned int>() );
    cout << "done." << endl;
  }
  
  // prepare for possible saves
  itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation(roi) - StackTransforms::GetLoResTranslation("whole_heart");
  create_directory( Dirs::ColourDir() );
  
  // save LoRes image
  if(LoRes)
  {
    cout << "Saving LoRes volume..." << flush;
    // load transforms
    string loResTransformsDir = vm.count("loResTransformsDir") ? Dirs::ResultsDir() + vm["loResTransformsDir"].as<string>() : Dirs::LoResTransformsDir();
    Load(*LoResStack, loResTransformsDir);
    // move stack origins to ROI
    StackTransforms::Translate(*LoResStack, translation);
    // generate and save images
    LoResStack->updateVolumes();
    writeImage< StackType::VolumeType >( LoResStack->GetVolume(), Dirs::ColourDir() + "LoRes.mha");
    cout << "done." << endl;
  }
  
  // save HiRes rigid, similarity and affine image
  if(HiRes)
  {
    if(vm.count("hiResTransformsDir")) // just process single directory from command line
    {
      cout << "Saving HiRes volume..." << flush;
      string dir = Dirs::ResultsDir() + vm["hiResTransformsDir"].as<string>() + "/";
      Load(*HiResStack, dir);
      // move stack origins to ROI
      StackTransforms::Translate(*HiResStack, translation);
      // generate and save images
      HiResStack->updateVolumes();
      writeImage< StackType::VolumeType >( HiResStack->GetVolume(), dir + hiResName);
      
    }
    else // process all three transform optimisations
    {
      // load transforms, translate to correct ROI and save
      cout << "Saving 3 HiRes volumes..." << flush;
      vector< string > HiResTransformsDirs;
      HiResTransformsDirs.push_back("CenteredRigid2DTransform");
      HiResTransformsDirs.push_back("CenteredSimilarity2DTransform");
      HiResTransformsDirs.push_back("CenteredAffineTransform");
      for(vector< string >::iterator it = HiResTransformsDirs.begin(); it != HiResTransformsDirs.end(); ++it)
      {
        Load(*HiResStack, Dirs::HiResTransformsDir() + *it);
        // move stack origins to ROI
        StackTransforms::Translate(*HiResStack, translation);
        // generate and save images
        HiResStack->updateVolumes();
        writeImage< StackType::VolumeType >( HiResStack->GetVolume(), Dirs::ColourDir() + *it + ".mha");
      }
    }
    cout << "done." << endl;
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
      ("outputDir", po::value<string>(), "directory to source transforms and place results")
      ("slice", po::value<string>(), "basename of single slice to build")
      ("roi", po::value<string>()->default_value("ROI"), "set region of interest")
      ("loResTransformsDir", po::value<string>(), "directory containing LoRes transform files, relative to ResultsDir")
      ("hiResTransformsDir", po::value<string>(), "directory containing HiRes transform files, relative to ResultsDir")
      ("blockDir", po::value<string>(), "directory containing LoRes originals")
      ("hiResName", po::value<string>()->default_value("HiRes.mha"), "name of the HiRes output file")
      ("defaultPixelValue", po::value<unsigned int>()->default_value(255), "value applied to pixels outside the moving image")

      // three different ways of not specifying value for flag
      // implicit_value(true) allows syntax like -H0 and --no-HiRes=0,
      // rather than just the flag e.g. -H or --no-HiRes
      ("no-LoRes,L", po::value<bool>()->zero_tokens(), "do not generate LoRes image")
      ("no-HiRes,H", po::bool_switch(), "do not generate HiRes image")
      // ("no-HiRes,H", po::value(&noHiRes)->implicit_value(true), "do not generate HiRes image")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("slice", 1);
  
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
      << argv[0] << " [--dataSet=]RatX [--outputDir=]my_dir [[--slice=]0001] [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}

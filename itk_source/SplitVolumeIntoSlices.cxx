#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <assert.h>
#include <iomanip>

#include "itkExtractImageFilter.h"
#include "itkRGBPixel.h"

// my files
#include "Dirs.hpp"
#include "IOHelpers.hpp"


void checkUsage(int argc, char *argv[]) {
  if( argc < 4 )
  {
    cerr << "\nUsage: " << endl;
    std::cerr << argv[0] << " inputFile outputDir" << std::endl;
    exit(EXIT_FAILURE);
  }
}

namespace po = boost::program_options;
using namespace boost::filesystem;

template <typename PixelType>
void doSplitVolumeIntoSlices(const string& inputFile, const string& outputDir, const unsigned int sliceDimension);

po::variables_map parse_arguments(int argc, char *argv[]);

int main( int argc, char *argv[] )
{
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
  // Process command line arguments
  create_directories(vm["outputDir"].as<string>());
  
  unsigned int sliceDimension = vm["sliceDimension"].as<unsigned int>();
  
  if(vm["pixelType"].as<string>() == "rgb")
  {
    doSplitVolumeIntoSlices< itk::RGBPixel< unsigned char > >(vm["inputFile"].as<string>(),
                                                              vm["outputDir"].as<string>(),
                                                              sliceDimension);
  }
  
  return EXIT_SUCCESS;
}

template <typename PixelType>
void doSplitVolumeIntoSlices(const string& inputFile, const string& outputDir, const unsigned int sliceDimension)
{
  // typedefs
  typedef itk::Image< PixelType, 3 > VolumeType;
  typedef itk::Image< PixelType, 2 > SliceType;
  typedef itk::ExtractImageFilter< VolumeType, SliceType > SplitterType;
  
  // read volume
  cerr << "Reading volume...";
  typename VolumeType::Pointer volume = readImage<VolumeType>(inputFile);
  cerr << "done." << endl;
  
  // set up splitter
  typename SplitterType::Pointer splitter = SplitterType::New();
  splitter->SetInput( volume );
  typename VolumeType::SizeType volumeSize = volume->GetLargestPossibleRegion().GetSize();
  typename VolumeType::SizeType sliceSize  = volumeSize;
  sliceSize[sliceDimension] = 0;
  
  typename VolumeType::IndexType sliceIndex = {{0, 0, 0}};
  
  typename VolumeType::RegionType sliceRegion;
  sliceRegion.SetSize( sliceSize );
  sliceRegion.SetIndex( sliceIndex );
  
  for(unsigned int i=0; i<volumeSize[sliceDimension]; ++i) {
    cerr << "slice " << setw(4) << i << ": ";
    // Set the z-coordinate of the slice to be extracted
    sliceRegion.SetIndex(sliceDimension, i);
    
    splitter->SetExtractionRegion( sliceRegion );
    splitter->SetDirectionCollapseToIdentity();
    
    // split volume
    try
    {
      cerr << "extracting slice...";
      splitter->Update();
    }
    catch( itk::ExceptionObject & err )
    {
      std::cerr << "ExceptionObject caught while updating volume splitter." << std::endl;
      cerr << err << endl;
  		exit(EXIT_FAILURE);
    }
    
    // write slice
    stringstream outputFile;
    outputFile << outputDir << "/"
               // leading zeros
               << setfill('0')
               // 4 digits wide
               << setw(3)
               << i
               << ".tiff";
    
    typename SliceType::Pointer outputSlice = splitter->GetOutput();
    
    cerr << "writing slice...";
    writeImage< SliceType >(outputSlice, outputFile.str());
    cerr << "done." << endl;
  }
  
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("inputFile", po::value<string>(), "input volume")
      ("outputDir", po::value<string>(), "directory to contain output slices")
      ("pixelType,p", po::value<string>()->default_value("rgb"), "type of image pixel e.g. rgb, unsigned char, float etc.")
      ("sliceDimension,d", po::value<unsigned int>()->default_value(0), "dimension perpendicular to slices")
  ;
  
  po::positional_options_description p;
  p.add("inputFile", 1)
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
  
  // if help is specified, or positional args aren't present,
  // or more than one loadX flag
  if(    vm.count("help")
     || !vm.count("inputFile")
     || !vm.count("outputDir")
    )
  {
    cerr << "Usage: "
      << argv[0] << " [--inputFile=]volume.mha [--outputDir=]slices [Options]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  // check sliceDim is 0, 1 or 2
  if(vm["sliceDimension"].as<unsigned int>() > 2)
  {
    cerr << "sliceDim must be 0, 1 or 2." << endl;
    exit(EXIT_FAILURE);
  }
  
  return vm;
}


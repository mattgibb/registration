#include "boost/program_options.hpp"

#include "itkRGBPixel.h"
#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkNumericSeriesFileNames.h"
#include "itkChangeInformationImageFilter.h"

#include "IOHelpers.hpp"
#include "Parameters.hpp"

namespace po = boost::program_options;
po::variables_map parse_arguments(int argc, char *argv[]);

int main( int argc, char ** argv )
{
  // set dataset, in order to use getSpacings
  
  
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
  typedef itk::RGBPixel< unsigned char > PixelType;
  typedef itk::Image< PixelType, 3 > ImageType;
  typedef itk::ImageSeriesReader< ImageType > SeriesReaderType;
  typedef itk::NumericSeriesFileNames NameGeneratorType;
  
  SeriesReaderType::Pointer seriesReader = SeriesReaderType::New();
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();
  
  nameGenerator->SetStartIndex( 1 );
  nameGenerator->SetEndIndex( vm["numberOfSlices"].as<unsigned int>() );
  nameGenerator->SetIncrementIndex( 1 );
  nameGenerator->SetSeriesFormat( vm["inputSeriesFormat"].as<string>() );
  seriesReader->SetFileNames( nameGenerator->GetFileNames() );
  seriesReader->Update();
  
  ImageType::Pointer input = seriesReader->GetOutput();
  ImageType::SpacingType spacing = input->GetSpacing();
  spacing[2] = vm["zSpacing"].as<double>();
  
  typedef itk::ChangeInformationImageFilter< ImageType > ZScalerType;
  ZScalerType::Pointer zScaler = ZScalerType::New();
  zScaler->ChangeSpacingOn();
	zScaler->SetOutputSpacing(spacing);
  zScaler->SetInput(input);
  
  ImageType::Pointer output = zScaler->GetOutput();
  writeImage<ImageType>(output, vm["outputFile"].as<string>() );

  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("numberOfSlices,n", po::value<unsigned int>(), "number of input files")
      ("inputSeriesFormat", po::value<string>()->default_value("HiRes_%03d.mha"), "format of image series file names")
      ("outputFile", po::value<string>()->default_value("HiRes.mha"), "name of output volume")
      ("zSpacing", po::value<double>()->default_value(10.0), "spacing in microns between slices")
  ;
  
  po::positional_options_description p;
  p.add("numberOfSlices", 1)
   .add("inputSeriesFormat", 1)
   .add("outputFile", 1)
   .add("zSpacing", 1)
   ;
  
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
  if( vm.count("help") || !vm.count("numberOfSlices") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--numberOfSlices=]100"
      << " [--inputSeriesFormat=]HiRes_%03d"
      << " [--outputFile=]HiRes.mha"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

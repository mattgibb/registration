#include "boost/program_options.hpp"

#include "itkSimpleFilterWatcher.h"

#include "itkBinaryShapeOpeningImageFilter.h"
#include "itkBinaryShapeKeepNObjectsImageFilter.h"

#include "IOHelpers.hpp"

namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

template<typename OpeningType, typename ImageType>
typename OpeningType::Pointer buildOpener(const po::variables_map& vm)
{
  typename OpeningType::Pointer opener = OpeningType::New();
  opener->SetBackgroundValue( vm["background"].as<unsigned int>() );
  opener->SetForegroundValue( vm["foreground"].as<unsigned int>() );
  opener->SetReverseOrdering( vm["reverseOrdering"].as<bool>() );
  // Set/Get whether the connected components are defined strictly by
  // face connectivity or by face+edge+vertex connectivity.  Default is
  // FullyConnectedOff.  For objects that are 1 pixel wide, use
  // FullyConnectedOn.
  opener->SetFullyConnected( vm["fullyConnected"].as<bool>() );
  opener->SetAttribute( vm["attribute"].as<string>() );
  itk::SimpleFilterWatcher watcher(opener, "filter");
  return opener;
}

int main(int argc, char * argv[])
{
  po::variables_map vm = parse_arguments(argc, argv);
  
  typedef itk::Image< unsigned char, 3 > ImageType;
  
  cout << "Reading image..." << flush;
  ImageType::Pointer input = readImage< ImageType >( argv[1] );
  cout << "done." << endl;
  ImageType::Pointer output;
  
  if(vm.count("lambda"))
  // filter by attribute
  {
    typedef itk::BinaryShapeOpeningImageFilter< ImageType > OpeningType;
    OpeningType::Pointer opener = buildOpener< OpeningType, ImageType>(vm);
    opener->SetInput( input );
    opener->SetLambda( vm["lambda"].as<double>() );
    cout << "Opening image..." << flush;
    opener->Update();
    cout << "done." << endl;
    output = opener->GetOutput();
  }
  else
  // filter by number
  {
    typedef itk::BinaryShapeKeepNObjectsImageFilter< ImageType > OpeningType;
    OpeningType::Pointer opener = buildOpener< OpeningType, ImageType>(vm);
    opener->SetInput( input );
    opener->SetNumberOfObjects( vm["number"].as<unsigned int>() );
    cout << "Opening image..." << flush;
    opener->Update();
    cout << "done." << endl;
    output = opener->GetOutput();
  }
  
  cout << "Writing image..." << flush;
  writeImage< ImageType >(output, vm["output"].as<string>());
  cout << "done." << endl;
  
  return EXIT_SUCCESS;
}

void print_attribute_info()
{
  cerr << "Attribute must be set to one of the following:" << endl
    << "  NumberOfPixels" << endl
    << "  PhysicalSize" << endl
    << "  Centroid" << endl
    << "  BoundingBox" << endl
    << "  NumberOfPixelsOnBorder" << endl
    << "  PerimeterOnBorder" << endl
    << "  FeretDiameter" << endl
    << "  PrincipalMoments" << endl
    << "  PrincipalAxes" << endl
    << "  Elongation" << endl
    << "  Perimeter" << endl
    << "  Roundness" << endl
    << "  EquivalentSphericalRadius" << endl
    << "  EquivalentSphericalPerimeter" << endl
    << "  EquivalentEllipsoidDiameter" << endl
    << "  Flatness" << endl
    << "  PerimeterOnBorderRatio" << endl << endl;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("input", po::value<string>(), "input image to read")
      ("output", po::value<string>(), "output image file to write to")
      ("lambda", po::value<double>(), "cutoff size of object to keep")
      ("number,n", po::value<unsigned int>(), "number of objects to keep")
      ("attribute", po::value<string>()->default_value("NumberOfPixels"), "attribute by which to sort objects")
      ("reverseOrdering", po::bool_switch(), "whether to keep smallest or largest objects")
      ("background", po::value<unsigned int>()->default_value(0),   "background pixel value")
      ("foreground", po::value<unsigned int>()->default_value(255), "foreground pixel value")
      ("fullyConnected", po::bool_switch(), "whether to use strict face or face+edge+vertex connectivity")
  ;
  
  po::positional_options_description p;
  p.add("input", 1)
   .add("output", 1)
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
  
  // if help is specified, or positional args aren't present
  if(  vm.count("help")
    || !vm.count("input")
    || !vm.count("output")
    || vm.count("lambda") == vm.count("number") // either lambda or number must be specified, but not both
    ) {
    cerr << "Usage: "
      << argv[0] << " [--input=]crap.mha [--output=]sweet.mha [Options]" << endl
      << "Either lambda or number must be specified."
      << endl << endl;
    cerr << opts << "\n";
    print_attribute_info();
    
    exit(EXIT_FAILURE);
  }
  
  // check that attribute is in valid set
  string attribute = vm["attribute"].as<string>();
  if(
    attribute != "NumberOfPixels" &&
    attribute != "PhysicalSize" &&
    attribute != "Centroid" &&
    attribute != "BoundingBox" &&
    attribute != "NumberOfPixelsOnBorder" &&
    attribute != "PerimeterOnBorder" &&
    attribute != "FeretDiameter" &&
    attribute != "PrincipalMoments" &&
    attribute != "PrincipalAxes" &&
    attribute != "Elongation" &&
    attribute != "Perimeter" &&
    attribute != "Roundness" &&
    attribute != "EquivalentSphericalRadius" &&
    attribute != "EquivalentSphericalPerimeter" &&
    attribute != "EquivalentEllipsoidDiameter" &&
    attribute != "Flatness" &&
    attribute != "PerimeterOnBorderRatio"
  )
  {
    print_attribute_info();
    
    exit(EXIT_FAILURE);
  }
  
  return vm;
}


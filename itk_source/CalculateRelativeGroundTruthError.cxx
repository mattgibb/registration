// calculate a scalar measure of the displacement
// of a noisy transform from its original position

#include "boost/program_options.hpp"
#include <boost/math/constants/constants.hpp>

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkIdentityTransform.h"
#include "itkMatrixOffsetTransformBase.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileReader.h"

#include "IOHelpers.hpp"

// define pi
const double pi = boost::math::constants::pi<double>();

using namespace std;
namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

typedef itk::Transform< double, 2, 2 > TransformType;

TransformType::Pointer readCheckedTransform(string path);

int main( int argc, char * argv[] )
{
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  
  // read transforms
  TransformType::Pointer perfectTransform1 = readCheckedTransform(vm["perfectTransform1"].as<string>());
  TransformType::Pointer perfectTransform2 = readCheckedTransform(vm["perfectTransform2"].as<string>());
  TransformType::Pointer noisyTransform1   = readCheckedTransform(vm["noisyTransform1"].as<string>());
  TransformType::Pointer noisyTransform2   = readCheckedTransform(vm["noisyTransform2"].as<string>());
  
  // calculate the mean Euclidian distance of each non-zero pixel
  // after being transformed by the two transforms
  double meanDistance = 0;
  unsigned int countedPixels = 0, uncountedPixels = 0;
  
  typedef unsigned char PixelType;
  typedef itk::Image< PixelType, 2 > ImageType;
  typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
  typedef itk::Point< double, 2 > PointType;
  
  ImageType::Pointer mask = readImage< ImageType >(vm["mask"].as<string>());
  ImageType::RegionType inputRegion = mask->GetLargestPossibleRegion();
  ConstIteratorType it(mask, inputRegion);
  it.GoToBegin();
  
  while( !it.IsAtEnd() )
  {
    // if the pixel has a non-zero value,
    // include it in the calculation of the mean distance
    if(it.Get())
    {
      // calculate original physical position of pixel
      ImageType::IndexType index = it.GetIndex();
      PointType physicalPosition;
      mask->TransformIndexToPhysicalPoint(index, physicalPosition);
      
      // add Euclidian distance to total
      PointType perfectPosition = perfectTransform2->TransformPoint(perfectTransform1->GetInverseTransform()->TransformPoint(physicalPosition));
      PointType noisyPosition   = noisyTransform2->  TransformPoint(noisyTransform1->  GetInverseTransform()->TransformPoint(physicalPosition));
      meanDistance += perfectPosition.EuclideanDistanceTo(noisyPosition);
      
      ++countedPixels;
    }
    else
    {
      ++uncountedPixels;
    }
    ++it;
  }
  
  meanDistance /= countedPixels;
  
  cout << meanDistance << endl;
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("mask", po::value<string>(), "each non-zero pixel is counted in mean displacement")
      ("perfectTransform1", po::value<string>(), "first perfect transform")
      ("perfectTransform2", po::value<string>(), "second perfect transform")
      ("noisyTransform1", po::value<string>(), "first noisy transform")
      ("noisyTransform2", po::value<string>(), "second noisy transform")
  ;
  
  po::positional_options_description p;
  p.add("mask", 1)
   .add("perfectTransform1", 1)
   .add("perfectTransform2", 1)
   .add("noisyTransform1", 1)
   .add("noisyTransform2", 1)
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
  if(    vm.count("help")
     || !vm.count("mask")
     || !vm.count("perfectTransform1")
     || !vm.count("perfectTransform2")
     || !vm.count("noisyTransform1")
     || !vm.count("noisyTransform2")
    )
  {
    cerr << "Usage: "
      << argv[0] << " [--mask=]sliceMask.mha"
      << " [--perfectTransform1=]perfect/0001 [[--perfectTransform2=]perfect/0002]"
      << " [--noisyTransform1=]noisy/0001 [[--noisyTransform2=]noisy/0002]"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}


TransformType::Pointer readCheckedTransform(string path)
{
  itk::TransformBase::Pointer bpTransform = readTransform(path);
  TransformType::Pointer transform = dynamic_cast<TransformType*>( bpTransform.GetPointer() );
  assert( transform != 0 );
  return transform;
}


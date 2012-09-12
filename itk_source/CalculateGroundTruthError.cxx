// calculate a scalar measure of the displacement
// of a noisy transform from its original position

#include "boost/program_options.hpp"
#include <boost/math/constants/constants.hpp>

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkMatrixOffsetTransformBase.h"

#include "IOHelpers.hpp"

// define pi
const double pi = boost::math::constants::pi<double>();

using namespace std;
namespace po = boost::program_options;

po::variables_map parse_arguments(int argc, char *argv[]);

int main( int argc, char * argv[] )
{
  // Parse command line arguments
  po::variables_map vm = parse_arguments(argc, argv);
  
	// Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  typedef itk::MatrixOffsetTransformBase< double, 2, 2 > TransformType;
  
  // read transforms
  itk::TransformBase::Pointer bpNoisyTransform   = readTransform(vm["noisyTransform"].as<string>());
  itk::TransformBase::Pointer bpPerfectTransform = readTransform(vm["perfectTransform"].as<string>());
  TransformType *noisyTransform   = dynamic_cast<TransformType*>( bpNoisyTransform.GetPointer() );
  TransformType *perfectTransform = dynamic_cast<TransformType*>( bpPerfectTransform.GetPointer() );
  assert( noisyTransform   != 0 );
  assert( perfectTransform != 0 );
  
  // typedefs
  typedef itk::Point<double, 2> PointType;
  typedef vector<PointType> PointsType;
  
  // extract centre and radius from command line arguments
  PointType centre;
  centre[0] =  vm["mean-x"].as<double>();
  centre[1] =  vm["mean-y"].as<double>();
  double radius = vm["radius"].as<double>();
  
  // generate 8 compass points on a circle of the specified radius,
  // centred at the specified centre
  PointsType points(8);
  
  
  for(PointsType::iterator ppoint=points.begin(); ppoint != points.end(); ++ppoint)
  {
    static double angle = 0;
    (*ppoint)[0] = centre[0] + radius * cos(angle);
    (*ppoint)[1] = centre[1] + radius * sin(angle);
    angle += pi/4;
  }
  
  // calculate the mean Euclidian distance of each pair
  // between the noisy transformed point and the perfect transformed point
  double meanDistance = 0;
  for(PointsType::iterator ppoint=points.begin(); ppoint != points.end(); ++ppoint)
  {
    PointType noisyPoint   = noisyTransform->GetInverseTransform()->TransformPoint(*ppoint);
    PointType perfectPoint = perfectTransform->GetInverseTransform()->TransformPoint(*ppoint);
    cerr << "noisyPoint: " << noisyPoint << endl;
    // cerr << "perfectPoint: " << perfectPoint << endl;
    meanDistance += noisyPoint.EuclideanDistanceTo(perfectPoint);
  }
  meanDistance /= 8;
  
  std::cout << "mean distance: " << meanDistance << std::endl;
 
 
 
  
  return EXIT_SUCCESS;
}

po::variables_map parse_arguments(int argc, char *argv[])
{
  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help,h", "produce help message")
      ("noisyTransform", po::value<string>(), "input image")
      ("perfectTransform", po::value<string>(), "output image")
      ("mean-x", po::value<double>(), "x-coordinate of the point cloud centre")
      ("mean-y", po::value<double>(), "y-coordinate of the point cloud centre")
      ("radius", po::value<double>(), "radius of point cloud")
  ;
  
  po::positional_options_description p;
  p.add("noisyTransform", 1)
   .add("perfectTransform", 1)
   .add("mean-x", 1)
   .add("mean-y", 1)
   .add("radius", 1)
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
     || !vm.count("noisyTransform")
     || !vm.count("perfectTransform")
     || !vm.count("mean-x")
     || !vm.count("mean-y")
     || !vm.count("radius")
    )
  {
    cerr << "Usage: "
      << argv[0] << " [--noisyTransform=]noisy/0001 [--perfectTransform=]perfect/0001 "
      << " [--mean-x=]5.0 [--mean-y=]10.0 [--radius=]2.0"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
  
  return vm;
}

// Composes two transform sets.
// Output transform is the result of applying the first input transform, followed by the second.

#include <boost/lexical_cast.hpp>
#include "boost/program_options.hpp"

#include <assert.h>

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkCenteredRigid2DTransform.h"

#include "IOHelpers.hpp"
#include "Dirs.hpp" 

namespace po = boost::program_options;
using namespace boost;

po::variables_map parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	// Verify the number of parameters in the command line
  po::variables_map vm = parse_arguments(argc, argv);
	
	// Process command line arguments
  Dirs::SetDataSet( vm["dataSet"].as<string>() );
  Dirs::SetOutputDirName( vm["outputDir"].as<string>() );
  const string transformsName = vm["transformsName"].as<string>();
  
  // set up transform paths
  const string originalDir   = Dirs::ResultsDir() + "HiResTransforms_1_8/CenteredRigid2DTransform/";
  const string adjustmentDir = Dirs::ResultsDir() + "HiResPairs/FinalTransforms/" + transformsName;
  const string bananaDir     = Dirs::ResultsDir() + "HiResPairs/BananaTransforms/"  + transformsName;
  
	// set up unique file lists
  vector< string > basenames = getBasenames(Dirs::ImageList());
  set< string > unique_basenames_set(basenames.begin(), basenames.end());
  vector< string > unique_basenames(unique_basenames_set.begin(), unique_basenames_set.end());
  vector< string > pair_basenames;
  for(unsigned int i=0; i<unique_basenames.size()-1; i++)
  {
    pair_basenames.push_back(unique_basenames[i] + "_" + unique_basenames[i+1]);
  }
  vector< string > originalPaths   = constructPaths(originalDir, unique_basenames);
  vector< string > adjustmentPaths = constructPaths(adjustmentDir, pair_basenames);
  vector< string > bananaPaths     = constructPaths(bananaDir, unique_basenames);
  
	// clear results directory
  remove_all(bananaDir);
  create_directories(bananaDir);
  
	// Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  // itk::TransformFactory< itk::TranslationTransform< double, 2 > >::RegisterTransform();
  
	// Generate new transforms
	// TranslationTransform also has a Compose() interface, but only with other TranslationTransforms
  typedef itk::MatrixOffsetTransformBase< double, 2, 2 > ComposableTransformType;
  typedef itk::CenteredRigid2DTransform< double > CenteredRigid2DTransformType;
  
  for(int i=0; i < originalPaths.size(); ++i)
  {
    cerr << "unique_basenames[" << i << "]: " << unique_basenames[i] << endl;
    
    // create banana transform
    CenteredRigid2DTransformType::Pointer bananaTransform = CenteredRigid2DTransformType::New();
    bananaTransform->SetIdentity();
    
    // apply all the transforms from i->(i-1) down to (1->0)
    for(int j=i-1; j >= 0; j--)
    {
      // check that original transform is of the right dynamic type
      itk::TransformBase::Pointer bpAdjustmentTransform = readTransform(adjustmentPaths[j]);
      ComposableTransformType *pAdjustmentTransform = dynamic_cast<ComposableTransformType*>( bpAdjustmentTransform.GetPointer() );
      assert( pAdjustmentTransform != 0 );
      
      // compose adjustment with banana transform
      // If the argument pre is true (default false), then other is precomposed with self; that is, the resulting transformation consists of first
      // applying other to the source, followed by self. If pre is false or omitted, then other is post-composed with self; that is the resulting
      // transformation consists of first applying self to the source, followed by other. This updates the Translation based on current center.
      bananaTransform->Compose(pAdjustmentTransform);
    }
    
    // check that original transform is of the right dynamic type
    itk::TransformBase::Pointer bpOriginalTransform = readTransform(originalPaths[i]);
    ComposableTransformType *pOriginalTransform = dynamic_cast<ComposableTransformType*>( bpOriginalTransform.GetPointer() );
    assert( pOriginalTransform != 0 );
    
  	// apply the original transforms
    bananaTransform->Compose(pOriginalTransform);
    
    // save output transform
    writeTransform(bananaTransform, bananaPaths[i]);
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
      ("transformsName", po::value<string>(), "name of transform group")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("transformsName", 1)
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
  if(vm.count("help") ||
    !vm.count("dataSet") ||
    !vm.count("outputDir") ||
    !vm.count("transformsName") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--transformsName=]CenteredRigid2DTransform"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}


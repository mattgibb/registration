// Composes two transform sets.
// Output transform is the result of applying the first input transform, followed by the second.

#include <boost/lexical_cast.hpp>
#include "boost/program_options.hpp"

#include <assert.h>

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkAffineTransform.h"

#include "StackTransforms.hpp"
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
	const unsigned int iteration = vm["iteration"].as<unsigned int>();
  
  // set up diffusion and adjustment paths
  const string originalDir  = Dirs::ResultsDir() + "HiResPairs/AdjustedTransforms/"  + transformsName + "_" + lexical_cast<string>(iteration - 1);
  const string diffusionDir = Dirs::ResultsDir() + "HiResPairs/DiffusionTransforms/" + transformsName + "_" + lexical_cast<string>(iteration);
  const string adjustedDir  = Dirs::ResultsDir() + "HiResPairs/AdjustedTransforms/"  + transformsName + "_" + lexical_cast<string>(iteration);

	// set up file lists
  vector< string > basenames = getBasenames(Dirs::ImageList());
  vector< string > originalPaths   = constructPaths(originalDir,  basenames);
  vector< string > diffusionPaths  = constructPaths(diffusionDir, basenames);
  vector< string > adjustedPaths   = constructPaths(adjustedDir,  basenames);
  
	// clear results directory
  remove_all(adjustedDir);
  create_directories(adjustedDir);
  
	// Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  // itk::TransformFactory< itk::TranslationTransform< double, 2 > >::RegisterTransform();
  
	// Generate new transforms
	// TranslationTransform also has a Compose() interface, but only with other TranslationTransforms
  typedef itk::MatrixOffsetTransformBase< double, 2, 2 > ComposableTransformType;
  typedef itk::AffineTransform< double, 2 > AffineTransformType;
  
  // the first and last slices should not have adjustments
  // apply adjustments for every other slice
  for(unsigned int i=1; i < originalPaths.size() - 1; ++i)
  {
    // check that transforms are of the right dynamic type
    itk::TransformBase::Pointer bpOriginalTransform = readTransform(originalPaths[i]);
    itk::TransformBase::Pointer bpDiffusionTransform = readTransform(diffusionPaths[i]);
    ComposableTransformType *pOriginalTransform  = dynamic_cast<ComposableTransformType*>( bpOriginalTransform.GetPointer() );
    ComposableTransformType *pDiffusionTransform = dynamic_cast<ComposableTransformType*>( bpDiffusionTransform.GetPointer() );
    assert( pOriginalTransform != 0 && pDiffusionTransform != 0 );
    
    // calculate ROI transform
    itk::Vector< double, 2 > translation = StackTransforms::GetLoResTranslation("ROI") - StackTransforms::GetLoResTranslation("whole_heart");
    
    // compose transforms
    // If the argument pre is true (default false), then other is precomposed with self; that is, the resulting transformation consists of first
    // applying other to the source, followed by self. If pre is false or omitted, then other is post-composed with self; that is the resulting
    // transformation consists of first applying self to the source, followed by other. This updates the Translation based on current center.
    AffineTransformType::Pointer adjustedTransform = AffineTransformType::New();
    adjustedTransform->SetIdentity();
    adjustedTransform->Translate(-translation);
    adjustedTransform->Compose(pDiffusionTransform);
    adjustedTransform->Translate(translation);
    adjustedTransform->Compose(pOriginalTransform);
    
    // save output transform
    writeTransform(adjustedTransform, adjustedPaths[i]);
  }
  
  // write out the unaltered first and last transforms
  copy_file( *(originalPaths.begin()), *(adjustedPaths.begin()) );
  copy_file( *(--originalPaths.end()), *(--adjustedPaths.end()) );
  
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
      ("iteration", po::value<unsigned int>(), "iteration number of diffusion smoothing")
  ;
  
  po::positional_options_description p;
  p.add("dataSet", 1)
   .add("outputDir", 1)
   .add("transformsName", 1)
   .add("iteration", 1)
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
    !vm.count("transformsName") ||
    !vm.count("iteration") )
  {
    cerr << "Usage: "
      << argv[0]
      << " [--dataSet=]RatX [--outputDir=]my_dir"
      << " [--transformsName=]CenteredAffineTransform"
      << " [--iteration=]1"
      << endl << endl;
    cerr << opts << "\n";
    exit(EXIT_FAILURE);
  }
    
  return vm;
}


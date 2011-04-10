// Composes two transform sets.
// Output transform is the result of applying the first input transform, followed by the second.

#include <assert.h>

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkMatrixOffsetTransformBase.h"

#include "IOHelpers.hpp"
#include "Dirs.hpp" 


void checkUsage(int argc, char const *argv[]) {
  if( argc < 3 )
  {
    cerr << "\nUsage: " << endl;
    cerr << argv[0] << " firstInputDir secondInputDir outputDir\n\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	// Generate file lists
  vector< string > firstInputFileNames  = getFilePaths(argv[1], Dirs::SliceFile());
  vector< string > secondInputFileNames = getFilePaths(argv[2], Dirs::SliceFile());
  vector< string > outputFileNames      = getFilePaths(argv[3], Dirs::SliceFile());

	// Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  // itk::TransformFactory< itk::TranslationTransform< double, 2 > >::RegisterTransform();
  
	// Generate new transforms
	typedef itk::TransformFileReader ReaderType;
	typedef itk::TransformFileWriter WriterType;
	// TranslationTransform also has a Compose() interface, but only with other TranslationTransforms
  typedef itk::MatrixOffsetTransformBase< double, 2, 2 > ComposableTransformType;
  
  for(unsigned int i=0; i < firstInputFileNames.size(); ++i)
  {
    // Load input transforms
    ReaderType::Pointer firstReader = ReaderType::New();
    ReaderType::Pointer secondReader = ReaderType::New();
    firstReader->SetFileName(  firstInputFileNames[i].c_str()  );
    secondReader->SetFileName( secondInputFileNames[i].c_str() );
    firstReader->Update();
    secondReader->Update();
    
    // check that transforms are of the right dynamic type
    ComposableTransformType *pFirstTransform  = dynamic_cast<ComposableTransformType*>( firstReader->GetTransformList()->begin()->GetPointer() );
    ComposableTransformType *pSecondTransform = dynamic_cast<ComposableTransformType*>( secondReader->GetTransformList()->begin()->GetPointer() );
    assert( pFirstTransform != 0 && pSecondTransform != 0 );
    
    // compose transforms
    // If the argument pre is true, then other is precomposed with self; that is, the resulting transformation consists of first applying 
    // other to the source, followed by self. If pre is false or omitted, then other is post-composed with self; that is the resulting
    // transformation consists of first applying self to the source, followed by other. This updates the Translation based on current center.
    pFirstTransform->Compose(pSecondTransform);
    
    // save output transform
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outputFileNames[i].c_str() );
    writer->AddTransform( pFirstTransform );
    writer->Update();
    
  }
  
  return EXIT_SUCCESS;
}

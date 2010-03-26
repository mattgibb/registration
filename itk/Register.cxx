#include "itkImage.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageMaskSpatialObject.h"

// 3-D registration
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

// 2-D registration
#include "itkRegularStepGradientDescentOptimizer.h"

// File IO
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"

// my files
#include "StdOutIterationUpdate.hpp"
#include "FileIterationUpdate.hpp"
#include "MultiResRegistrationCommand.hpp"
#include "Stack.hpp"
#include "Framework3D.hpp"

using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 10 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " histoDir regularExpression ";
		cerr << "sortingExpression mriFile transformFile3D ";
		cerr << "outputMRI outputStack outputMask outputParameters\n\n";
		cerr << "histoDir should have no trailing slash\n";
		cerr << "regularExpression should be enclosed in single quotes\n";
		cerr << "sortingExpression should be an integer ";
		cerr << "representing the group in the regexp\n\n";
    exit(EXIT_FAILURE);
  }
  
}

vector< string > getFileNames(char const *argv[]) {
	string directory = argv[1];
	string regularExpression = argv[2];
	const unsigned int subMatch = atoi( argv[3]);
	
	typedef itk::RegularExpressionSeriesFileNames    NameGeneratorType;
  NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

  nameGenerator->SetRegularExpression( regularExpression );
  nameGenerator->SetSubMatch( subMatch );
  nameGenerator->SetDirectory( directory );
  
	return nameGenerator->GetFileNames();
}

template<typename ImageType>
void writeImage(typename ImageType::Pointer image, char const *fileName) {
	typedef itk::ImageFileWriter< ImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( image );
  
  writer->SetFileName( fileName );
	
  try {
  	writer->Update();
  }
	catch( itk::ExceptionObject & err ) {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
		exit(EXIT_FAILURE);
	}
}


int main (int argc, char const *argv[]) {
	// Verify the number of parameters in the command line
	checkUsage(argc, argv);
	
	
	// initialise stack object
	cout << "About to begin stack construction..." << endl;
	Stack stack( getFileNames(argv) );
	cout << "Just finished stack construction..." << endl;
	
	// TODO Make Framework::MRIVolumeType reference new MRI class instead, e.g. MRI::VolumeType
	typedef Framework3D::MRIVolumeType MRIVolumeType;
	
	// perform 3-D registration
	MRI mriVolume( argv[4] );
	Framework3D framework3D(stack, mriVolume);
  
  
  // Create the command observers and register them with the optimiser.
	typedef StdOutIterationUpdate< Framework3D::OptimizerType3D > StdOutObserverType3D;
	typedef FileIterationUpdate< Framework3D::OptimizerType3D > FileObserverType3D;
	typedef MultiResRegistrationCommand< Framework3D::RegistrationType3D,
	                                     Framework3D::OptimizerType3D,
	                                     Framework3D::MetricType3D > MultiResCommandType;
	StdOutObserverType3D::Pointer stdOutObserver3D = StdOutObserverType3D::New();
	FileObserverType3D::Pointer   fileObserver3D   = FileObserverType3D::New();
	MultiResCommandType::Pointer  multiResCommand  = MultiResCommandType::New();
  framework3D.optimizer3D->AddObserver( itk::IterationEvent(), stdOutObserver3D );
  framework3D.optimizer3D->AddObserver( itk::IterationEvent(), fileObserver3D   );
  framework3D.registration3D->AddObserver( itk::IterationEvent(), multiResCommand  );

	ofstream output;
	output.open( argv[9] );
	fileObserver3D->SetOfstream( &output );
	
	framework3D.registration3D->SetNumberOfLevels( 4 );
		
  // Begin registration
  try
    {
    framework3D.registration3D->StartRegistration();
    cout << "Optimizer stop condition: "
         << framework3D.registration3D->GetOptimizer()->GetStopConditionDescription() << endl << endl;

    }
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }
  
  output.close();
	
	
	
	// Get final parameters
  Framework3D::OptimizerType3D::ParametersType finalParameters3D = framework3D.registration3D->GetLastTransformParameters();
  
	// Write final transform to file
	itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();

  writer->SetInput( framework3D.transform3D );
  // writer->AddTransform( anotherTransform );

  writer->SetFileName( argv[5] );

  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    return 0;
    }
	
  Framework3D::TransformType3D::MatrixType matrix3D = framework3D.transform3D->GetRotationMatrix();
  Framework3D::TransformType3D::OffsetType offset3D = framework3D.transform3D->GetOffset();
  
  std::cout << "Matrix = " << std::endl << matrix3D << std::endl;
  std::cout << "Offset = " << std::endl << offset3D << std::endl;  
	
  typedef itk::ResampleImageFilter< MRI::MRIVolumeType, MRI::MRIVolumeType > ResampleFilterType;
  Framework3D::TransformType3D::Pointer finalTransform = Framework3D::TransformType3D::New();
  
  // TODO Check to see if using transform3D directly instead of finalTransform makes a difference
  finalTransform->SetCenter( framework3D.transform3D->GetCenter() );
  finalTransform->SetParameters( finalParameters3D );
  finalTransform->SetFixedParameters( framework3D.transform3D->GetFixedParameters() );
  
  ResampleFilterType::Pointer resampler3D = ResampleFilterType::New();
  
  Stack::VolumeType::Pointer fixedImage3D = stack.GetVolume();
  
  resampler3D->SetSize( fixedImage3D->GetLargestPossibleRegion().GetSize() );
  resampler3D->SetOutputOrigin( fixedImage3D->GetOrigin() );
  resampler3D->SetOutputSpacing( fixedImage3D->GetSpacing() );
  resampler3D->SetOutputDirection( fixedImage3D->GetDirection() );
  resampler3D->SetDefaultPixelValue( 100 );
  
  // resampler3D->SetTransform( finalTransform );
  resampler3D->SetTransform( framework3D.transform3D );
	resampler3D->SetInterpolator( framework3D.interpolator3D );
  resampler3D->SetInput( mriVolume.GetVolume() );

	writeImage< MRI::MRIVolumeType >(resampler3D->GetOutput(), argv[6] );
    
  // used for final resampling
	typedef itk::NearestNeighborInterpolateImageFunction< MRI::MRIVolumeType, double > NearestNeighborInterpolatorType3D;
  
	// extract 2-D MRI slices
	// typedef itk::ExtractImageFilter< OutputImageType, OutputSliceType > ExtractFilterType;
  // ExtractFilterType::Pointer extractor = ExtractFilterType::New();
  
	
	
	// perform 2-D registration
	typedef StdOutIterationUpdate< itk::RegularStepGradientDescentOptimizer > ObserverType2D;
	ObserverType2D::Pointer observer2D = ObserverType2D::New();
	
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[7] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), argv[8] );
		
	
  return EXIT_SUCCESS;
}
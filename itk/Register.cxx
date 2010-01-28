#include "itkImage.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

// 3-D registration
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

// 2-D registration
#include "itkRegularStepGradientDescentOptimizer.h"

// File IO
#include "itkRegularExpressionSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// my files
#include "CommandIterationUpdate.hpp"
#include "Stack.hpp"

using namespace std;

void checkUsage(int argc, char const *argv[]) {
  if( argc < 6 )
  {
    cerr << "\nUsage: " << std::endl;
    cerr << argv[0] << " histoDir regularExpression ";
    cerr << "sortingExpression mriFile outputImageFile\n\n";
		cerr << "directory should have no trailing slash\n";
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
	Stack stack( getFileNames(argv) );
	
	
	// perform 3-D registration
	typedef CommandIterationUpdate< itk::VersorRigid3DTransformOptimizer > ObserverType3D;
	ObserverType3D::Pointer observer3D = ObserverType3D::New();
	
	
  typedef unsigned short MRIPixelType;
  typedef itk::Image< MRIPixelType, 3 > MRIVolumeType;	
	typedef itk::VersorRigid3DTransform< double > TransformType3D;
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType3D;
	typedef itk::MattesMutualInformationImageToImageMetric< Stack::VolumeType, MRIVolumeType > MetricType3D;
  // used for registration
  typedef itk::LinearInterpolateImageFunction< MRIVolumeType, double > LinearInterpolatorType3D;
  // used for final resampling
	typedef itk::NearestNeighborInterpolateImageFunction< MRIVolumeType, double > NearestNeighborInterpolatorType3D;
  typedef itk::ImageRegistrationMethod< Stack::VolumeType, MRIVolumeType > RegistrationType3D;
	
	
	MetricType3D::Pointer metric3D = MetricType3D::New();
  OptimizerType3D::Pointer optimizer3D = OptimizerType3D::New();
  LinearInterpolatorType3D::Pointer interpolator3D = LinearInterpolatorType3D::New();
  RegistrationType3D::Pointer registration3D = RegistrationType3D::New();
  TransformType3D::Pointer  transform3D = TransformType3D::New();

	// Number of spatial samples should be ~20% of pixels for detailed images, see ITK Software Guide p341
	// Total pixels in MRI: 128329344
	// metric3D.UseAllPixelsOn() // Uses all the pixels in the fixed image, rather than just a sample
	metric3D->SetNumberOfSpatialSamples( 12800000 );
	// Number of bins recommended to be about 50, see ITK Software Guide p341
	metric3D->SetNumberOfHistogramBins( 50 );
	
  registration3D->SetMetric( metric3D );
  registration3D->SetOptimizer( optimizer3D );
  registration3D->SetInterpolator( interpolator3D );
  registration3D->SetTransform( transform3D );
	
	
  typedef itk::ImageFileReader< MRIVolumeType > MRIVolumeReaderType;
  MRIVolumeReaderType::Pointer MRIVolumeReader = MRIVolumeReaderType::New();
	MRIVolumeReader->SetFileName( argv[4] );
	
	
  registration3D->SetFixedImage( stack.GetVolume() );
  registration3D->SetMovingImage( MRIVolumeReader->GetOutput() );
	
  registration3D->SetFixedImageRegion( stack.GetVolume()->GetBufferedRegion() );


	// Set up transform initializer
  typedef itk::CenteredTransformInitializer< TransformType3D,
																						 Stack::VolumeType,
																						 MRIVolumeType > TransformInitializerType;
  TransformInitializerType::Pointer initializer = TransformInitializerType::New();

  initializer->SetTransform( transform3D );
  initializer->SetFixedImage(  stack.GetVolume() );
  initializer->SetMovingImage( MRIVolumeReader->GetOutput() );

  //  The use of the geometrical centers is selected by calling
  //  GeometryOn() while the use of center of mass is selected by
  //  calling MomentsOn(). Below we select the center of mass mode.
  // initializer->GeometryOn();
  initializer->MomentsOn();
  initializer->InitializeTransform();

  //  The rotation part of the transform is initialized using a
  //  Versor which is simply a unit quaternion. The
  //  VersorType can be obtained from the transform traits. The versor
  //  itself defines the type of the vector used to indicate the rotation axis.
  //  This trait can be extracted as VectorType. The following lines
  //  create a versor object and initialize its parameters by passing a
  //  rotation axis and an angle.
  typedef TransformType3D::VersorType VersorType;
  typedef VersorType::VectorType VectorType;
  
  VersorType rotation;
  VectorType axis;
  
  axis[0] = 1.0;
  axis[1] = 0.0;
  axis[2] = -1.0;
  
  const double angle = M_PI;
  
  rotation.Set(  axis, angle );
  
  transform3D->SetRotation( rotation );

	// TEMP TO TEST ROTATION
	typedef itk::ResampleImageFilter< MRIVolumeType, MRIVolumeType > ResampleFilterType;

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();

  resampler->SetTransform( transform3D );

  LinearInterpolatorType3D::Pointer interpolator = LinearInterpolatorType3D::New();
  resampler->SetInterpolator( interpolator );
  
  // resampler->SetDefaultPixelValue( 0 );
	MRIVolumeReader->Update();
	MRIVolumeType::Pointer MRIImage = MRIVolumeReader->GetOutput();
  
  resampler->SetOutputSpacing( MRIImage->GetSpacing() );
  resampler->SetOutputOrigin( MRIImage->GetOrigin() );
  resampler->SetOutputDirection( MRIImage->GetDirection() );
  resampler->SetSize( MRIImage->GetLargestPossibleRegion().GetSize() );

  resampler->SetInput( MRIImage );
	
	writeImage< MRIVolumeType >( resampler->GetOutput(), "testdir/rotated_mri.mhd");
	
	// END TEMP TO TEST ROTATION
  


  //  We now pass the parameters of the current transform as the initial
  //  parameters to be used when the registration process starts. 
  // 
  // // Software Guide : BeginCodeSnippet
  // registration->SetInitialTransformParameters( transform->GetParameters() );
  // // Software Guide : EndCodeSnippet
  // 
  // typedef OptimizerType::ScalesType       OptimizerScalesType;
  // OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
  // const double translationScale = 1.0 / 1000.0;
  // 
  // optimizerScales[0] = 1.0;
  // optimizerScales[1] = 1.0;
  // optimizerScales[2] = 1.0;
  // optimizerScales[3] = translationScale;
  // optimizerScales[4] = translationScale;
  // optimizerScales[5] = translationScale;
  // 
  // optimizer->SetScales( optimizerScales );
  // 
  // optimizer->SetMaximumStepLength( 0.2000  ); 
  // optimizer->SetMinimumStepLength( 0.0001 );
  // 
  // optimizer->SetNumberOfIterations( 200 );
  // 
  // 
  // // Create the Command observer and register it with the optimizer.
  // //
  // CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  // optimizer->AddObserver( itk::IterationEvent(), observer );
  // 
  // 
  // try
  //   {
  //   registration->StartRegistration(); 
  //   std::cout << "Optimizer stop condition: "
  //             << registration->GetOptimizer()->GetStopConditionDescription()
  //             << std::endl;
  //   } 
  // catch( itk::ExceptionObject & err ) 
  //   { 
  //   std::cerr << "ExceptionObject caught !" << std::endl; 
  //   std::cerr << err << std::endl; 
  //   return EXIT_FAILURE;
  //   } 
  // 
  // OptimizerType::ParametersType finalParameters = 
  //                   registration->GetLastTransformParameters();
  // 
  // 
  // const double versorX              = finalParameters[0];
  // const double versorY              = finalParameters[1];
  // const double versorZ              = finalParameters[2];
  // const double finalTranslationX    = finalParameters[3];
  // const double finalTranslationY    = finalParameters[4];
  // const double finalTranslationZ    = finalParameters[5];
  // 
  // const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
  // 
  // const double bestValue = optimizer->GetValue();
  // 
  // 
  // // Print out results
  // //
  // std::cout << std::endl << std::endl;
  // std::cout << "Result = " << std::endl;
  // std::cout << " versor X      = " << versorX  << std::endl;
  // std::cout << " versor Y      = " << versorY  << std::endl;
  // std::cout << " versor Z      = " << versorZ  << std::endl;
  // std::cout << " Translation X = " << finalTranslationX  << std::endl;
  // std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  // std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
  // std::cout << " Iterations    = " << numberOfIterations << std::endl;
  // std::cout << " Metric value  = " << bestValue          << std::endl;
  // 
  // 
  // //  Software Guide : BeginLatex
  // //  
  // //  Let's execute this example over some of the images available in the ftp
  // //  site 
  // //
  // //  \url{ftp://public.kitware.com/pub/itk/Data/BrainWeb}
  // //
  // //  Note that the images in the ftp site are compressed in \code{.tgz} files.
  // //  You should download these files an uncompress them in your local system.
  // //  After decompressing and extracting the files you could take a pair of
  // //  volumes, for example the pair:
  // //  
  // //  \begin{itemize}
  // //  \item \code{brainweb165a10f17.mha} 
  // //  \item \code{brainweb165a10f17Rot10Tx15.mha}
  // //  \end{itemize}
  // //
  // //  The second image is the result of intentionally rotating the first image
  // //  by $10$ degrees around the origin and shifting it $15mm$ in $X$.  The
  // //  registration takes $24$ iterations and produces:
  // //
  // //  \begin{center}
  // //  \begin{verbatim}
  // //  [-6.03744e-05, 5.91487e-06, -0.0871932, 2.64659, -17.4637, -0.00232496]
  // //  \end{verbatim}
  // //  \end{center}
  // //
  // //  That are interpreted as
  // //
  // //  \begin{itemize}
  // //  \item Versor        = $(-6.03744e-05, 5.91487e-06, -0.0871932)$
  // //  \item Translation   = $(2.64659,  -17.4637,  -0.00232496)$ millimeters
  // //  \end{itemize}
  // //  
  // //  This Versor is equivalent to a rotation of $9.98$ degrees around the $Z$
  // //  axis.
  // // 
  // //  Note that the reported translation is not the translation of $(15.0,0.0,0.0)$
  // //  that we may be naively expecting. The reason is that the
  // //  \code{VersorRigid3DTransform} is applying the rotation around the center
  // //  found by the \code{CenteredTransformInitializer} and then adding the
  // //  translation vector shown above.
  // //
  // //  It is more illustrative in this case to take a look at the actual
  // //  rotation matrix and offset resulting form the $6$ parameters.
  // //
  // //  Software Guide : EndLatex 
  // 
  // // Software Guide : BeginCodeSnippet
  // transform->SetParameters( finalParameters );
  // 
  // TransformType::MatrixType matrix = transform->GetRotationMatrix();
  // TransformType::OffsetType offset = transform->GetOffset();
  // 
  // std::cout << "Matrix = " << std::endl << matrix << std::endl;
  // std::cout << "Offset = " << std::endl << offset << std::endl;
  // // Software Guide : EndCodeSnippet
  // 
  // 
  // //  Software Guide : BeginLatex
  // //
  // //  The output of this print statements is
  // //
  // //  \begin{center}
  // //  \begin{verbatim}
  // //  Matrix =
  // //      0.984795 0.173722 2.23132e-05
  // //      -0.173722 0.984795 0.000119257
  // //      -1.25621e-06 -0.00012132 1
  // //
  // //  Offset =
  // //      [-15.0105, -0.00672343, 0.0110854]
  // //  \end{verbatim}
  // //  \end{center}
  // //
  // //  From the rotation matrix it is possible to deduce that the rotation is
  // //  happening in the X,Y plane and that the angle is on the order of
  // //  $\arcsin{(0.173722)}$ which is very close to 10 degrees, as we expected.
  // //  
  // //  Software Guide : EndLatex 
  // 
  // 
  // //  Software Guide : BeginLatex
  // //  
  // // \begin{figure}
  // // \center
  // // \includegraphics[width=0.44\textwidth]{BrainProtonDensitySliceBorder20.eps}
  // // \includegraphics[width=0.44\textwidth]{BrainProtonDensitySliceR10X13Y17.eps}
  // // \itkcaption[CenteredTransformInitializer input images]{Fixed and moving image
  // // provided as input to the registration method using
  // // CenteredTransformInitializer.}
  // // \label{fig:FixedMovingImageRegistration8}
  // // \end{figure}
  // //
  // //
  // // \begin{figure}
  // // \center
  // // \includegraphics[width=0.32\textwidth]{ImageRegistration8Output.eps}
  // // \includegraphics[width=0.32\textwidth]{ImageRegistration8DifferenceBefore.eps}
  // // \includegraphics[width=0.32\textwidth]{ImageRegistration8DifferenceAfter.eps} 
  // // \itkcaption[CenteredTransformInitializer output images]{Resampled moving
  // // image (left). Differences between fixed and moving images, before (center)
  // // and after (right) registration with the
  // // CenteredTransformInitializer.}
  // // \label{fig:ImageRegistration8Outputs}
  // // \end{figure}
  // //
  // // Figure \ref{fig:ImageRegistration8Outputs} shows the output of the
  // // registration. The center image in this figure shows the differences
  // // between the fixed image and the resampled moving image before the
  // // registration. The image on the right side presents the difference between
  // // the fixed image and the resampled moving image after the registration has
  // // been performed. Note that these images are individual slices extracted
  // // from the actual volumes. For details, look at the source code of this
  // // example, where the ExtractImageFilter is used to extract a slice from the
  // // the center of each one of the volumes. One of the main purposes of this
  // // example is to illustrate that the toolkit can perform registration on
  // // images of any dimension. The only limitations are, as usual, the amount of
  // // memory available for the images and the amount of computation time that it
  // // will take to complete the optimization process.
  // //
  // // \begin{figure}
  // // \center
  // // \includegraphics[height=0.32\textwidth]{ImageRegistration8TraceMetric.eps}
  // // \includegraphics[height=0.32\textwidth]{ImageRegistration8TraceAngle.eps}
  // // \includegraphics[height=0.32\textwidth]{ImageRegistration8TraceTranslations.eps} 
  // // \itkcaption[CenteredTransformInitializer output plots]{Plots of the metric,
  // // rotation angle, center of rotation and translations during the
  // // registration using CenteredTransformInitializer.}
  // // \label{fig:ImageRegistration8Plots}
  // // \end{figure}
  // //
  // //  Figure \ref{fig:ImageRegistration8Plots} shows the plots of the main
  // //  output parameters of the registration process. The metric values at every
  // //  iteration. The Z component of the versor is plotted as an indication of
  // //  how the rotation progress. The X,Y translation components of the
  // //  registration are plotted at every iteration too.
  // //  
  // //  Shell and Gnuplot scripts for generating the diagrams in
  // //  Figure~\ref{fig:ImageRegistration8Plots} are available in the directory
  // //
  // //  \code{InsightDocuments/SoftwareGuide/Art}
  // //
  // //  You are strongly encouraged to run the example code, since only in this
  // //  way you can gain a first hand experience with the behavior of the
  // //  registration process. Once again, this is a simple reflection of the
  // //  philosophy that we put forward in this book: 
  // //
  // //  \emph{If you can not replicate it, then it does not exist!}.
  // //
  // //  We have seen enough published papers with pretty pictures, presenting
  // //  results that in practice are impossible to replicate. That is vanity, not
  // //  science.
  // //
  // //  Software Guide : EndLatex 
  // 
  // 
  // typedef itk::ResampleImageFilter< 
  //                           MovingImageType, 
  //                           FixedImageType >    ResampleFilterType;
  // 
  // TransformType::Pointer finalTransform = TransformType::New();
  // 
  // finalTransform->SetCenter( transform->GetCenter() );
  // 
  // finalTransform->SetParameters( finalParameters );
  // finalTransform->SetFixedParameters( transform->GetFixedParameters() );
  // 
  // ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  // 
  // resampler->SetTransform( finalTransform );
  // resampler->SetInput( movingImageReader->GetOutput() );
  // 
  // FixedImageType::Pointer fixedImage = fixedImageReader->GetOutput();
  // 
  // resampler->SetSize(    fixedImage->GetLargestPossibleRegion().GetSize() );
  // resampler->SetOutputOrigin(  fixedImage->GetOrigin() );
  // resampler->SetOutputSpacing( fixedImage->GetSpacing() );
  // resampler->SetOutputDirection( fixedImage->GetDirection() );
  // resampler->SetDefaultPixelValue( 100 );
  // 
  // typedef  unsigned char  OutputPixelType;
  // 
  // typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
  // 
  // typedef itk::CastImageFilter< 
  //                       FixedImageType,
  //                       OutputImageType > CastFilterType;
  //                   
  // typedef itk::ImageFileWriter< OutputImageType >  WriterType;
  // 
  // 
  // WriterType::Pointer      writer =  WriterType::New();
  // CastFilterType::Pointer  caster =  CastFilterType::New();
  // 
  // 
  // writer->SetFileName( argv[3] );
  // 
  // 
  // caster->SetInput( resampler->GetOutput() );
  // writer->SetInput( caster->GetOutput()   );
  // writer->Update();
  // 
  // 
  // typedef itk::SubtractImageFilter< 
  //                                 FixedImageType, 
  //                                 FixedImageType, 
  //                                 FixedImageType > DifferenceFilterType;
  // 
  // DifferenceFilterType::Pointer difference = DifferenceFilterType::New();
  // 
  // 
  // typedef itk::RescaleIntensityImageFilter< 
  //                                 FixedImageType, 
  //                                 OutputImageType >   RescalerType;
  // 
  // RescalerType::Pointer intensityRescaler = RescalerType::New();
  // 
  // intensityRescaler->SetInput( difference->GetOutput() );
  // intensityRescaler->SetOutputMinimum(   0 );
  // intensityRescaler->SetOutputMaximum( 255 );
  // 
  // difference->SetInput1( fixedImageReader->GetOutput() );
  // difference->SetInput2( resampler->GetOutput() );
  // 
  // resampler->SetDefaultPixelValue( 1 );
  // 
  // WriterType::Pointer writer2 = WriterType::New();
  // writer2->SetInput( intensityRescaler->GetOutput() );  
  // 
  // 
  // // Compute the difference image between the 
  // // fixed and resampled moving image.
  // if( argc > 5 )
  //   {
  //   writer2->SetFileName( argv[5] );
  //   writer2->Update();
  //   }
  // 
  // 
  // typedef itk::IdentityTransform< double, Dimension > IdentityTransformType;
  // IdentityTransformType::Pointer identity = IdentityTransformType::New();
  // 
  // // Compute the difference image between the 
  // // fixed and moving image before registration.
  // if( argc > 4 )
  //   {
  //   resampler->SetTransform( identity );
  //   writer2->SetFileName( argv[4] );
  //   writer2->Update();
  //   }
  // 
  // //
  // //  Here we extract slices from the input volume, and the difference volumes
  // //  produced before and after the registration.  These slices are presented as
  // //  figures in the Software Guide.
  // //
  // //
  // 
  // typedef itk::Image< OutputPixelType, 2 > OutputSliceType;
  // 
  // typedef itk::ExtractImageFilter< 
  //                         OutputImageType, 
  //                         OutputSliceType > ExtractFilterType;
  // 
  // ExtractFilterType::Pointer extractor = ExtractFilterType::New();
  // 
  // 
  // FixedImageType::RegionType inputRegion =
  //                              fixedImage->GetLargestPossibleRegion();
  // 
  // FixedImageType::SizeType  size  = inputRegion.GetSize();
  // FixedImageType::IndexType start = inputRegion.GetIndex();
  // 
  // // Select one slice as output
  // size[2]  =  0;
  // start[2] = 90;
  // 
  // FixedImageType::RegionType desiredRegion;
  // desiredRegion.SetSize(  size  );
  // desiredRegion.SetIndex( start );
  // 
  // extractor->SetExtractionRegion( desiredRegion );
  // 
  // typedef itk::ImageFileWriter< OutputSliceType > SliceWriterType;
  // SliceWriterType::Pointer sliceWriter = SliceWriterType::New();
  // 
  // sliceWriter->SetInput( extractor->GetOutput() );
  //  
  // if( argc > 6 )
  //   {
  //   extractor->SetInput( caster->GetOutput() );
  //   resampler->SetTransform( identity );
  //   sliceWriter->SetFileName( argv[6] );  
  //   sliceWriter->Update();
  //   }
  //  
  // if( argc > 7 )
  //   {
  //   extractor->SetInput( intensityRescaler->GetOutput() );
  //   resampler->SetTransform( identity );
  //   sliceWriter->SetFileName( argv[7] );  
  //   sliceWriter->Update();
  //   }
  // 
  // if( argc > 8 )
  //   {
  //   resampler->SetTransform( finalTransform );
  //   sliceWriter->SetFileName( argv[8] );  
  //   sliceWriter->Update();
  //   }
  // 
  // if( argc > 9 )
  //   {
  //   extractor->SetInput( caster->GetOutput() );
  //   resampler->SetTransform( finalTransform );
  //   sliceWriter->SetFileName( argv[9] );  
  //   sliceWriter->Update();
  //   }
  // 	
	
	// add observer to optimiser
	
	
	// extract 2-D MRI slices
	
	
	// perform 2-D registration
	typedef CommandIterationUpdate< itk::RegularStepGradientDescentOptimizer > ObserverType2D;
	ObserverType2D::Pointer observer2D = ObserverType2D::New();

	
	
	// write volume and mask to disk
	writeImage< Stack::VolumeType >( stack.GetVolume(), argv[5] );
	writeImage< Stack::MaskVolumeType >( stack.GetMaskVolume(), "testdir/testmask.mhd");
		
	
  return EXIT_SUCCESS;
}
#include "itkNumericTraits.h"
#include "itkArray.h"
#include "itkImage.h"
// #include "itkImage.h"
// #include "itkImageFileReader.h"
// #include "itkImageFileWriter.h"
// #include "itkRegularExpressionSeriesFileNames.h"
// #include "itkChangeInformationImageFilter.h"
// #include "itkResampleImageFilter.h"
// #include "itkTileImageFilter.h"
// #include "itkTranslationTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkVersorRigid3DTransform.h"
// #include "itkLinearInterpolateImageFunction.h"
// #include "itkRegularStepGradientDescentOptimizer.h"
// #include "itkImageRegistrationMethod.h"

using namespace std;

void throw_error() {
  ::itk::ExceptionObject e_(__FILE__, __LINE__, "Hey!",ITK_LOCATION);
  throw e_;
}

int main(int argc, char* argv[]) {
  try{
    throw_error();
  }
  
  catch (itk::ExceptionObject e) {
    cout << "Caught!\n";
    cout << e;
  }
}

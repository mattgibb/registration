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

int main(int argc, char* argv[]) {
  typedef itk::Image< short, 2 > ImageType;
  ImageType::Pointer image = ImageType::New();
  cout << "image: " << image << endl;
}

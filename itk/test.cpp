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
	typedef itk::VersorRigid3DTransform< double > TransformType;
  TransformType::Pointer t = TransformType::New();
  t->SetIdentity();
  cout << "t->GetMatrix(): " << endl << t->GetMatrix() << endl;
  cout << "t->GetRotationMatrix(): " << endl << t->GetRotationMatrix() << endl;
  cout << "t->GetParameters(): " << endl << t->GetParameters() << endl;
  cout << "t->GetVersor(): " << endl << t->GetVersor() << endl;
}

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
// #include "itkLinearInterpolateImageFunction.h"
// #include "itkRegularStepGradientDescentOptimizer.h"
// #include "itkImageRegistrationMethod.h"

using namespace std;

class Test
{
public:
	vector< int > ints;
	
	Test(vector< int > fileNames) {
		ints = fileNames;
	}
	
	typedef itk::CenteredRigid2DTransform< double > TransformType;
	// TransformType::Pointer transform2D = TransformType::New();
	int test;
	
	int getLastElement() {
		return ints.back();
	}
		
};

int main() {
	int x=0, y=1;
	vector< int > vec;
	vec.push_back(x);
	vec.push_back(y);
	
	
	Test t(vec);
	int z;
	z = t.getLastElement();
	cout << "t.getLastElement = " << z;
}

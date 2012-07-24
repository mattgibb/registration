#ifndef __itkStructureTensorImageFilter_txx
#define __itkStructureTensorImageFilter_txx

#include <itkGradientImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkUnaryFunctorImageFilter.h>
#include "itkStructureTensorImageFilter.h"

namespace itk
{

template< class TInputImage >
void itkStructureTensorImageFilter< TInputImage >
::GenerateData()
{
	typedef GradientImageFilter<TInputImage> GradientType;
	//we use GradientImageFilter instead of GradientRecursiveGaussianImageFilter
	//because we expect the image to be already smoothed
	GradientType::Pointer grad=GradientType::New();
	grad->SetInput(this->GetInput());
	//grad->Update(); //useful for debugging
  
	typedef Image<CVector, TInputImage::ImageDimension> CovariantImageType;
    
	typedef itk::UnaryFunctorImageFilter< CovariantImageType, OutputImageType, CovariantVectorToTensorFunctor > FilterType;
	FilterType::Pointer filter=FilterType::New();
	filter->SetInput(grad->GetOutput());
	//filter->Update(); //useful for debugging

	typedef RecursiveGaussianImageFilter<OutputImageType, OutputImageType> SmoothingType;
	SmoothingType::Pointer smoothing=SmoothingType::New();
	smoothing->SetInput(filter->GetOutput());
	smoothing->SetSigma(m_Sigma);
	smoothing->Update();
	this->GraftOutput(smoothing->GetOutput());
}

}// end namespace

#endif //__itkStructureTensorImageFilter_txx
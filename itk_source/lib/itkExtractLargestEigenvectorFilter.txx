#ifndef __itkExtractLargestEigenvectorFilter_txx
#define __itkExtractLargestEigenvectorFilter_txx

#include <itkUnaryFunctorImageFilter.h>
#include "itkExtractLargestEigenvectorFilter.h"

namespace itk
{

template< class TensorImage >
void ExtractLargestEigenvectorFilter< TensorImage >
::GenerateData()
{
	typedef itk::UnaryFunctorImageFilter< TensorImage , VectorImageType, ExtractLargestEigenvectorFunctor > FilterType;
	typename FilterType::Pointer filter=FilterType::New();
	filter->SetInput(this->GetInput());
  filter->Update();
	this->GraftOutput(filter->GetOutput());
}

}// end namespace

#endif //__itkExtractLargestEigenvectorFilter_txx
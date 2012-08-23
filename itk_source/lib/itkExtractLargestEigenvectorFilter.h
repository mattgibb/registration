// extracts the largest eigenvector at each pixel from a tensor image

#ifndef __itkExtractLargestEigenvectorFilter_h
#define __itkExtractLargestEigenvectorFilter_h

#include <itkImageToImageFilter.h>

namespace itk
{
/** \class ExtractLargestEigenvectorFilter
 * \brief Constructs a structure tensor for each voxel.
 *
 * \ingroup ImageFilters
 */
template< class TensorImage >
class ExtractLargestEigenvectorFilter:public ImageToImageFilter< TensorImage, Image< CovariantVector<float, TensorImage::ImageDimension>, TensorImage::ImageDimension> >
{
public:
	/** Standard class typedefs. */
  typedef typename TensorImage::PixelType TensorType;
	typedef ExtractLargestEigenvectorFilter             Self;
	typedef ImageToImageFilter< TensorImage, Image< FixedArray<float, TensorImage::ImageDimension>, TensorImage::ImageDimension> > Superclass;
	typedef SmartPointer< Self >        Pointer;
  typedef CovariantVector<float, TensorImage::ImageDimension > VectorType;
  typedef Image< VectorType, TensorImage::ImageDimension > VectorImageType;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);
  
	/** Run-time type information (and related methods). */
	itkTypeMacro(ExtractLargestEigenvectorFilter, ImageToImageFilter);
  
protected:
	ExtractLargestEigenvectorFilter(){}
	~ExtractLargestEigenvectorFilter(){}
  
	virtual void GenerateData();
  
	class ExtractLargestEigenvectorFunctor
	{
	public:
		VectorType operator()( TensorType in )
		{
			VectorType result;
      typename TensorType::EigenValuesArrayType values;
      typename TensorType::EigenVectorsMatrixType vectors;
      in.ComputeEigenAnalysis(values, vectors);
      
			for (unsigned i=0; i<TensorImage::ImageDimension; i++)
        result[i]=vectors(TensorImage::ImageDimension-1,i);
			return result;
		}
	};

private:
	ExtractLargestEigenvectorFilter(const Self &); //purposely not implemented
	void operator=(const Self &);  //purposely not implemented
};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkExtractLargestEigenvectorFilter.txx"
#endif

#endif // __itkExtractLargestEigenvectorFilter_h

#ifndef __CenteredTransformPCAInitializer_h
#define __CenteredTransformPCAInitializer_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkImageMomentsCalculator.h"

#include <iostream>

namespace itk
{
/** \class CenteredTransformPCAInitializer
 * \brief CenteredTransformPCAInitializer is a helper class intended to
 * initialize the center of rotation, translation and angle of centered
 * transforms so the principal components of two images are aligned
 *
 * This class is connected to the fixed image, moving image and transform
 * involved in the registration.
 * 
 */
template< class TTransform,
          class TFixedImage,
          class TMovingImage >
class CenteredTransformPCAInitializer:public Object
{
public:
  /** Standard class typedefs. */
  typedef CenteredTransformPCAInitializer Self;
  typedef Object                          Superclass;
  typedef SmartPointer< Self >            Pointer;
  typedef SmartPointer< const Self >      ConstPointer;

  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CenteredTransformPCAInitializer, Object);

  /** Type of the transform to initialize */
  typedef TTransform                      TransformType;
  typedef typename TransformType::Pointer TransformPointer;

  /** Dimension of parameters. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int,
                      TransformType::InputSpaceDimension);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int,
                      TransformType::OutputSpaceDimension);

  /** Image Types to use in the initialization of the transform */
  typedef   TFixedImage  FixedImageType;
  typedef   TMovingImage MovingImageType;

  typedef   typename FixedImageType::ConstPointer  FixedImagePointer;
  typedef   typename MovingImageType::ConstPointer MovingImagePointer;

  /** Moment calculators */
  typedef ImageMomentsCalculator< FixedImageType >  FixedImageCalculatorType;
  typedef ImageMomentsCalculator< MovingImageType > MovingImageCalculatorType;

  typedef typename FixedImageCalculatorType::Pointer
  FixedImageCalculatorPointer;
  typedef typename MovingImageCalculatorType::Pointer
  MovingImageCalculatorPointer;

  /** Offset type. */
  typedef typename TransformType::OffsetType OffsetType;

  /** Point type. */
  typedef typename TransformType::InputPointType InputPointType;

  /** Vector type. */
  typedef typename TransformType::OutputVectorType OutputVectorType;

  /** Angle type. */
  typedef typename TransformType::ScalarType ScalarType;

  /** Set the transform to be initialized */
  itkSetObjectMacro(Transform,   TransformType);

  /** Set the fixed image used in the registration process */
  itkSetConstObjectMacro(FixedImage,  FixedImageType);

  /** Set the moving image used in the registration process */
  itkSetConstObjectMacro(MovingImage, MovingImageType);

  /** Initialize the transform using data from the images */
  virtual void InitializeTransform();

  /** Get() access to the moments calculators */
  itkGetConstObjectMacro(FixedCalculator,  FixedImageCalculatorType);
  itkGetConstObjectMacro(MovingCalculator, MovingImageCalculatorType);
protected:
  CenteredTransformPCAInitializer();
  ~CenteredTransformPCAInitializer(){}

  void PrintSelf(std::ostream & os, Indent indent) const;

  itkGetObjectMacro(Transform, TransformType);
private:
  CenteredTransformPCAInitializer(const Self &); //purposely not implemented
  void operator=(const Self &);               //purposely not implemented

  TransformPointer m_Transform;

  FixedImagePointer m_FixedImage;

  MovingImagePointer m_MovingImage;

  FixedImageCalculatorPointer  m_FixedCalculator;
  MovingImageCalculatorPointer m_MovingCalculator;
}; //class CenteredTransformPCAInitializer
}  // namespace registration

#ifndef ITK_MANUAL_INSTANTIATION
#include "CenteredTransformPCAInitializer.hxx"
#endif

#endif /* __CenteredTransformPCAInitializer_h */

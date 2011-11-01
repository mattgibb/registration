#ifndef __CenteredTransformPCAInitializer_hxx
#define __CenteredTransformPCAInitializer_hxx

#include "CenteredTransformPCAInitializer.h"

namespace itk
{
template< class TTransform, class TFixedImage, class TMovingImage >
CenteredTransformPCAInitializer< TTransform, TFixedImage, TMovingImage >
::CenteredTransformPCAInitializer()
{
  m_FixedCalculator  = FixedImageCalculatorType::New();
  m_MovingCalculator = MovingImageCalculatorType::New();
}

template< class TTransform, class TFixedImage, class TMovingImage >
void
CenteredTransformPCAInitializer< TTransform, TFixedImage, TMovingImage >
::InitializeTransform()
{
  // Sanity check
  if ( !m_FixedImage )
    {
    itkExceptionMacro("Fixed Image has not been set");
    return;
    }
  if ( !m_MovingImage )
    {
    itkExceptionMacro("Moving Image has not been set");
    return;
    }
  if ( !m_Transform )
    {
    itkExceptionMacro("Transform has not been set");
    return;
    }

  // If images come from filters, then update those filters.
  if ( m_FixedImage->GetSource() )
    {
    m_FixedImage->GetSource()->Update();
    }
  if ( m_MovingImage->GetSource() )
    {
    m_MovingImage->GetSource()->Update();
    }

  InputPointType   rotationCenter;
  OutputVectorType translationVector;
  ScalarType       angle;

  m_FixedCalculator->SetImage(m_FixedImage);
  m_FixedCalculator->Compute();
  m_MovingCalculator->SetImage(m_MovingImage);
  m_MovingCalculator->Compute();
  cerr << "m_FixedImage: " << m_FixedImage << endl;
  cerr << "m_MovingImage: " << m_MovingImage << endl;
  
  // calculate centre of rotation and translation
  typename FixedImageCalculatorType::VectorType fixedCenter =
    m_FixedCalculator->GetCenterOfGravity();

  typename MovingImageCalculatorType::VectorType movingCenter =
    m_MovingCalculator->GetCenterOfGravity();
  cerr << "fixedCenter: " << fixedCenter << endl;
  cerr << "movingCenter: " << movingCenter << endl;
  
  for ( unsigned int i = 0; i < InputSpaceDimension; i++ )
    {
    rotationCenter[i]    = fixedCenter[i];
    translationVector[i] = movingCenter[i] - fixedCenter[i];
    }
  
  // calculate transform angle
  // moving angle - fixed angle, -π/2 < angle ≤ π/2
  vnl_matrix< double > rotationMatrix =
    m_FixedCalculator->GetPrincipalAxes().GetTranspose()
    * m_MovingCalculator->GetPrincipalAxes().GetVnlMatrix();
  cerr << "rotationMatrix:\n" << rotationMatrix << endl;
  cerr << "rotationMatrix.get(1,0): " << rotationMatrix.get(1,0) << endl;
  angle = asin( rotationMatrix.get(1,0) );
  cerr << "angle: " << angle << endl;
  
  // initialise transform
  m_Transform->SetAngle(angle);
  m_Transform->SetCenter(rotationCenter);
  m_Transform->SetTranslation(translationVector);
}

template< class TTransform, class TFixedImage, class TMovingImage >
void
CenteredTransformPCAInitializer< TTransform, TFixedImage, TMovingImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Transform   = " << std::endl;
  if ( m_Transform )
    {
    os << indent << m_Transform  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "FixedImage   = " << std::endl;
  if ( m_FixedImage )
    {
    os << indent << m_FixedImage  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "MovingImage   = " << std::endl;
  if ( m_MovingImage )
    {
    os << indent << m_MovingImage  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "MovingMomentCalculator   = " << std::endl;
  if ( m_MovingCalculator )
    {
    os << indent << m_MovingCalculator  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "FixedMomentCalculator   = " << std::endl;
  if ( m_FixedCalculator )
    {
    os << indent << m_FixedCalculator  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }
}
}  // namespace registration

#endif /* __CenteredTransformPCAInitializer_hxx */

/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkGradientDifferenceImageToImageMetric_h
#define itkGradientDifferenceImageToImageMetric_h

#include "itkImageToImageMetric.h"

#include "itkSobelOperator.h"
#include "itkNeighborhoodOperatorImageFilter.h"
#include "itkPoint.h"
#include "itkCastImageFilter.h"
#include "itkResampleImageFilter.h"

namespace itk
{
/** \class GradientDifferenceImageToImageMetric
 * \brief Computes similarity between two objects to be registered
 *
 * This Class is templated over the type of the Images to be compared and
 * over the type of transformation and Interpolator to be used.
 *
 * This metric computes the sum of squared differences between pixels in
 * the derivatives of the moving and fixed images after passing the squared
 * difference through a function of type \f$ \frac{1}{1+x} \f$.
 *
 * \warning THIS IMAGE METRIC IS CURRENTLY UNDER DEBUGGING. USE AT YOUR OWN RISK.
 *
 * Spatial correspondence between both images is established through a
 * Transform. Pixel values are taken from the Moving image. Their positions
 * are mapped to the Fixed image and result in general in non-grid position
 * on it. Values at these non-grid position of the Fixed image are
 * interpolated using a user-selected Interpolator.
 *
 * Implementation of this class is based on \cite hipwell2003.
 *
 * \ingroup RegistrationMetrics
 * \ingroup ITKRegistrationCommon
 */
template <typename TFixedImage, typename TMovingImage>
class ITK_TEMPLATE_EXPORT GradientDifferenceImageToImageMetric : public ImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(GradientDifferenceImageToImageMetric);

  /** Standard class type aliases. */
  using Self = GradientDifferenceImageToImageMetric;
  using Superclass = ImageToImageMetric<TFixedImage, TMovingImage>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(GradientDifferenceImageToImageMetric);

  /** Types transferred from the base class */
  using typename Superclass::RealType;
  using typename Superclass::TransformType;
  using typename Superclass::TransformPointer;
  using typename Superclass::TransformParametersType;
  using typename Superclass::TransformJacobianType;

  using typename Superclass::MeasureType;
  using typename Superclass::DerivativeType;
  using typename Superclass::FixedImageType;
  using typename Superclass::MovingImageType;
  using typename Superclass::FixedImageConstPointer;
  using typename Superclass::MovingImageConstPointer;

  using FixedImagePixelType = typename TFixedImage::PixelType;
  using MovedImagePixelType = typename TMovingImage::PixelType;

  static constexpr unsigned int FixedImageDimension = TFixedImage::ImageDimension;
  /** Types for transforming the moving image */
  using TransformedMovingImageType = itk::Image<FixedImagePixelType, Self::FixedImageDimension>;

  using TransformMovingImageFilterType = itk::ResampleImageFilter<MovingImageType, TransformedMovingImageType>;

  /** Sobel filters to compute the gradients of the Fixed Image */

  using FixedGradientImageType = itk::Image<RealType, Self::FixedImageDimension>;

  using CastFixedImageFilterType = itk::CastImageFilter<FixedImageType, FixedGradientImageType>;
  using CastFixedImageFilterPointer = typename CastFixedImageFilterType::Pointer;

  using FixedGradientPixelType = typename FixedGradientImageType::PixelType;

  /** Sobel filters to compute the gradients of the Moved Image */

  static constexpr unsigned int MovedImageDimension = MovingImageType::ImageDimension;

  using MovedGradientImageType = itk::Image<RealType, Self::MovedImageDimension>;

  using CastMovedImageFilterType = itk::CastImageFilter<TransformedMovingImageType, MovedGradientImageType>;
  using CastMovedImageFilterPointer = typename CastMovedImageFilterType::Pointer;

  using MovedGradientPixelType = typename MovedGradientImageType::PixelType;

  /** Get the derivatives of the similarity measure. */
  void
  GetDerivative(const TransformParametersType & parameters, DerivativeType & derivative) const override;

  /** Get the value of the similarity measure for single valued optimizers. */
  MeasureType
  GetValue(const TransformParametersType & parameters) const override;

  /** Get value and derivatives of the similarity measure for multiple valued optimizers. */
  void
  GetValueAndDerivative(const TransformParametersType & parameters,
                        MeasureType &                   Value,
                        DerivativeType &                Derivative) const override;

  /** Initialize the Metric by making sure that all the components are present and plugged together correctly. */
  void
  Initialize() override;

  /** Set/Get the value of Delta used for computing derivatives by finite
   * differences in the GetDerivative() method */
  /** @ITKStartGrouping */
  itkSetMacro(DerivativeDelta, double);
  itkGetConstReferenceMacro(DerivativeDelta, double);
  /** @ITKEndGrouping */
protected:
  GradientDifferenceImageToImageMetric();
  ~GradientDifferenceImageToImageMetric() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** Compute the range of the moved image gradients. */
  void
  ComputeMovedGradientRange() const;

  /** Compute the variance and range of the moving image gradients. */
  void
  ComputeVariance() const;

  /** Compute the similarity measure using a specified subtraction factor. */
  MeasureType
  ComputeMeasure(const TransformParametersType & parameters, const double * subtractionFactor) const;

  using FixedSobelFilter = NeighborhoodOperatorImageFilter<FixedGradientImageType, FixedGradientImageType>;

  using MovedSobelFilter = NeighborhoodOperatorImageFilter<MovedGradientImageType, MovedGradientImageType>;

private:
  /** The variance of the moving image gradients. */
  mutable MovedGradientPixelType m_Variance[FixedImageDimension]{};

  /** The range of the moving image gradients. */
  mutable MovedGradientPixelType m_MinMovedGradient[MovedImageDimension]{};
  mutable MovedGradientPixelType m_MaxMovedGradient[MovedImageDimension]{};

  /** The range of the fixed image gradients. */
  mutable FixedGradientPixelType m_MinFixedGradient[FixedImageDimension]{};
  mutable FixedGradientPixelType m_MaxFixedGradient[FixedImageDimension]{};

  /** The filter for transforming the moving image. */
  typename TransformMovingImageFilterType::Pointer m_TransformMovingImageFilter{};

  /** The Sobel gradients of the fixed image */
  CastFixedImageFilterPointer m_CastFixedImageFilter{};

  SobelOperator<FixedGradientPixelType, Self::FixedImageDimension> m_FixedSobelOperators[FixedImageDimension]{};

  typename FixedSobelFilter::Pointer m_FixedSobelFilters[Self::FixedImageDimension]{};

  ZeroFluxNeumannBoundaryCondition<MovedGradientImageType> m_MovedBoundCond{};
  ZeroFluxNeumannBoundaryCondition<FixedGradientImageType> m_FixedBoundCond{};

  /** The Sobel gradients of the moving image */
  CastMovedImageFilterPointer m_CastMovedImageFilter{};

  SobelOperator<MovedGradientPixelType, Self::MovedImageDimension> m_MovedSobelOperators[MovedImageDimension]{};

  typename MovedSobelFilter::Pointer m_MovedSobelFilters[Self::MovedImageDimension]{};

  double m_DerivativeDelta{};
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGradientDifferenceImageToImageMetric.hxx"
#endif

#endif

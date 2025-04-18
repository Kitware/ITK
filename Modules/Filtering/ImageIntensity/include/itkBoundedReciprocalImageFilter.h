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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef itkBoundedReciprocalImageFilter_h
#define itkBoundedReciprocalImageFilter_h

#include "itkUnaryGeneratorImageFilter.h"

namespace itk
{
/** \class BoundedReciprocalImageFilter
 *
 * \brief Computes 1/(1+x) for each pixel in the image
 *
 * The filter expects both the input and output images to have the same
 * number of dimensions, and both of a scalar image type.
 *
 * \ingroup ITKImageIntensity
 */
namespace Functor
{
template <typename TInput, typename TOutput>
class BoundedReciprocal
{
public:
  bool
  operator==(const BoundedReciprocal &) const
  {
    return true;
  }

  ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(BoundedReciprocal);

  inline TOutput
  operator()(const TInput & A) const
  {
    return static_cast<TOutput>(1.0 / (1.0 + static_cast<double>(A)));
  }
};
} // namespace Functor

template <typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT BoundedReciprocalImageFilter : public UnaryGeneratorImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BoundedReciprocalImageFilter);

  /** Standard class type aliases. */
  using Self = BoundedReciprocalImageFilter;
  using Superclass = UnaryGeneratorImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using FunctorType = Functor::BoundedReciprocal<typename TInputImage::PixelType, typename TOutputImage::PixelType>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(BoundedReciprocalImageFilter);

  itkConceptMacro(InputConvertibleToDoubleCheck, (Concept::Convertible<typename TInputImage::PixelType, double>));
  itkConceptMacro(DoubleConvertibleToOutputCheck, (Concept::Convertible<double, typename TOutputImage::PixelType>));

protected:
  BoundedReciprocalImageFilter()
  {
#if !defined(ITK_WRAPPING_PARSER)
    Superclass::SetFunctor(FunctorType());
#endif
  }

  ~BoundedReciprocalImageFilter() override = default;
};
} // end namespace itk

#endif

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
#ifndef itkBoxSigmaImageFilter_h
#define itkBoxSigmaImageFilter_h

#include "itkBoxImageFilter.h"

namespace itk
{
/**
 * \class BoxSigmaImageFilter
 * \brief Implements a fast rectangular sigma filter using the
 * accumulator approach
 *
 * This code was contributed in the Insight Journal paper:
 * "Efficient implementation of kernel filtering"
 * by Beare R., Lehmann G
 * https://doi.org/10.54294/igq8fn
 *
 * \author Gaetan Lehmann
 * \ingroup ITKSmoothing
 */

template <typename TInputImage, typename TOutputImage = TInputImage>
class ITK_TEMPLATE_EXPORT BoxSigmaImageFilter : public BoxImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BoxSigmaImageFilter);

  /** Standard class type aliases. */
  using Self = BoxSigmaImageFilter;
  using Superclass = BoxImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Standard New method. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(BoxSigmaImageFilter);

  /** Image related type alias. */
  using InputImageType = TInputImage;
  using OutputImageType = TOutputImage;
  using RegionType = typename TInputImage::RegionType;
  using SizeType = typename TInputImage::SizeType;
  using IndexType = typename TInputImage::IndexType;
  using PixelType = typename TInputImage::PixelType;
  using OffsetType = typename TInputImage::OffsetType;
  using typename Superclass::OutputImageRegionType;
  using OutputPixelType = typename TOutputImage::PixelType;

  /** Image related type alias. */
  static constexpr unsigned int OutputImageDimension = TOutputImage::ImageDimension;
  static constexpr unsigned int InputImageDimension = TInputImage::ImageDimension;

  itkConceptMacro(SameDimension, (Concept::SameDimension<Self::InputImageDimension, Self::OutputImageDimension>));

protected:
  BoxSigmaImageFilter();
  ~BoxSigmaImageFilter() override = default;

  /** Multi-thread version GenerateData. */
  void
  DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;

}; // end of class
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBoxSigmaImageFilter.hxx"
#endif

#endif

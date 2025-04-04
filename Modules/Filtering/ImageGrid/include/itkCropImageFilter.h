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
#ifndef itkCropImageFilter_h
#define itkCropImageFilter_h

#include "itkExtractImageFilter.h"

namespace itk
{
/**
 * \class CropImageFilter
 * \brief Decrease the image size by cropping the image by an itk::Size at
 * both the upper and lower bounds of the largest possible region.
 *
 * CropImageFilter changes the image boundary of an image by removing
 * pixels outside the target region. The target region is not specified in
 * advance, but calculated in BeforeThreadedGenerateData().
 *
 * This filter uses ExtractImageFilter to perform the cropping.
 *
 * \ingroup GeometricTransform
 * \ingroup ITKImageGrid
 *
 * \sphinx
 * \sphinxexample{Filtering/ImageGrid/CropImageBySpecifyingRegion2,Crop Image By Specifying Region}
 * \endsphinx
 */
template <typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT CropImageFilter : public ExtractImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(CropImageFilter);

  /** Standard class type aliases. */
  using Self = CropImageFilter;
  using Superclass = ExtractImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(CropImageFilter);

  /** Typedef to describe the output and input image region types. */
  using typename Superclass::OutputImageRegionType;
  using typename Superclass::InputImageRegionType;

  /** Typedef to describe the type of pixel. */
  using typename Superclass::OutputImagePixelType;
  using typename Superclass::InputImagePixelType;

  /** Typedef to describe the output and input image index and size types. */
  using typename Superclass::OutputImageIndexType;
  using typename Superclass::InputImageIndexType;
  using typename Superclass::OutputImageSizeType;
  using typename Superclass::InputImageSizeType;
  using SizeType = InputImageSizeType;

  /** ImageDimension constants. */
  static constexpr unsigned int InputImageDimension = Superclass::InputImageDimension;
  static constexpr unsigned int OutputImageDimension = Superclass::OutputImageDimension;

  /** Set/Get the cropping sizes for the upper and lower boundaries. */
  /** @ITKStartGrouping */
  itkSetMacro(UpperBoundaryCropSize, SizeType);
  itkGetConstMacro(UpperBoundaryCropSize, SizeType);
  itkSetMacro(LowerBoundaryCropSize, SizeType);
  itkGetConstMacro(LowerBoundaryCropSize, SizeType);
  /** @ITKEndGrouping */
  void
  SetBoundaryCropSize(const SizeType & s)
  {
    this->SetUpperBoundaryCropSize(s);
    this->SetLowerBoundaryCropSize(s);
  }

  itkConceptMacro(InputConvertibleToOutputCheck, (Concept::Convertible<InputImagePixelType, OutputImagePixelType>));
  itkConceptMacro(SameDimensionCheck, (Concept::SameDimension<InputImageDimension, OutputImageDimension>));

protected:
  CropImageFilter()
  {
    this->SetDirectionCollapseToSubmatrix();
    m_UpperBoundaryCropSize.Fill(0);
    m_LowerBoundaryCropSize.Fill(0);
  }

  ~CropImageFilter() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  void
  GenerateOutputInformation() override;

  void
  VerifyInputInformation() const override;

private:
  SizeType m_UpperBoundaryCropSize{};
  SizeType m_LowerBoundaryCropSize{};
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkCropImageFilter.hxx"
#endif

#endif

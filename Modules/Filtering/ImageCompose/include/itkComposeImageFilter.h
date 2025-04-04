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
#ifndef itkComposeImageFilter_h
#define itkComposeImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "itkImageScanlineIterator.h"
#include <vector>

namespace itk
{
/**
 * \class ComposeImageFilter
 * \brief ComposeImageFilter combine several scalar images into a multicomponent image
 *
 * ComposeImageFilter combine several scalar images into an itk::Image of
 * vector pixel (itk::Vector, itk::RGBPixel, ...), of std::complex pixel,
 * or in an itk::VectorImage.
 *
 * \par Inputs and Usage
   \code
      filter->SetInput( 0, image0 );
      filter->SetInput( 1, image1 );
      ...
      filter->Update();
      itk::VectorImage< PixelType, dimension >::Pointer = filter->GetOutput();
   \endcode
 * All input images are expected to have the same template parameters and have
 * the same size and origin.
 *
 * \sa VectorImage
 * \sa VectorIndexSelectionCastImageFilter
 * \ingroup ITKImageCompose
 *
 * \sphinx
 * \sphinxexample{Filtering/ImageCompose/CreateVectorImageFromScalarImages,Create Vector Image From Scalar Images}
 * \sphinxexample{Filtering/ImageCompose/ComposeVectorFromThreeScalarImages,Compose Vector From Three Scalar Images}
 * \sphinxexample{Filtering/ImageCompose/ConvertRealAndImaginaryToComplexImage,Convert Real And Imaginary Images To
 Complex Image}
 * \endsphinx
 */
template <typename TInputImage,
          typename TOutputImage = VectorImage<typename TInputImage::PixelType, TInputImage::ImageDimension>>
class ITK_TEMPLATE_EXPORT ComposeImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ComposeImageFilter);

  using Self = ComposeImageFilter;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
  itkNewMacro(Self);
  itkOverrideGetNameOfClassMacro(ComposeImageFilter);

  static constexpr unsigned int Dimension = TInputImage::ImageDimension;

  using InputImageType = TInputImage;
  using OutputImageType = TOutputImage;
  using InputPixelType = typename InputImageType::PixelType;
  using OutputPixelType = typename OutputImageType::PixelType;
  using RegionType = typename InputImageType::RegionType;

  void
  SetInput1(const InputImageType * image1);
  void
  SetInput2(const InputImageType * image2);
  void
  SetInput3(const InputImageType * image3);

  itkConceptMacro(InputCovertibleToOutputCheck,
                  (Concept::Convertible<InputPixelType, typename NumericTraits<OutputPixelType>::ValueType>));

protected:
  ComposeImageFilter();

  void
  GenerateOutputInformation() override;

  void
  BeforeThreadedGenerateData() override;

  void
  DynamicThreadedGenerateData(const RegionType & outputRegionForThread) override;


private:
  // we have to specialize the code for complex, because it provides no operator[]
  // method
  using InputIteratorType = ImageScanlineConstIterator<InputImageType>;
  using InputIteratorContainerType = std::vector<InputIteratorType>;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkComposeImageFilter.hxx"
#endif

#endif

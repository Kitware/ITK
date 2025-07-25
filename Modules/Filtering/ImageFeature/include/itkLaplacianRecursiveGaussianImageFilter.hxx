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
#ifndef itkLaplacianRecursiveGaussianImageFilter_hxx
#define itkLaplacianRecursiveGaussianImageFilter_hxx

#include "itkImageRegionIteratorWithIndex.h"
#include "itkProgressAccumulator.h"
#include "itkCastImageFilter.h"
#include "itkBinaryGeneratorImageFilter.h"

namespace itk
{

template <typename TInputImage, typename TOutputImage>
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::LaplacianRecursiveGaussianImageFilter()

{
  for (unsigned int i = 0; i < NumberOfSmoothingFilters; ++i)
  {
    m_SmoothingFilters[i] = GaussianFilterType::New();
    m_SmoothingFilters[i]->SetOrder(GaussianOrderEnum::ZeroOrder);
    m_SmoothingFilters[i]->SetNormalizeAcrossScale(m_NormalizeAcrossScale);
    m_SmoothingFilters[i]->ReleaseDataFlagOn();
    m_SmoothingFilters[i]->InPlaceOn();
  }

  m_DerivativeFilter = DerivativeFilterType::New();
  m_DerivativeFilter->SetOrder(GaussianOrderEnum::SecondOrder);
  m_DerivativeFilter->SetNormalizeAcrossScale(m_NormalizeAcrossScale);
  m_DerivativeFilter->ReleaseDataFlagOn();
  m_DerivativeFilter->InPlaceOff();

  m_DerivativeFilter->SetInput(this->GetInput());

  m_SmoothingFilters[0]->SetInput(m_DerivativeFilter->GetOutput());

  if (NumberOfSmoothingFilters > 1)
  {
    for (unsigned int i = 1; i < NumberOfSmoothingFilters; ++i)
    {
      m_SmoothingFilters[i]->SetInput(m_SmoothingFilters[i - 1]->GetOutput());
    }
  }

  this->SetSigma(1.0);
}

template <typename TInputImage, typename TOutputImage>
void
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::SetSigma(RealType sigma)
{
  for (unsigned int i = 0; i < NumberOfSmoothingFilters; ++i)
  {
    m_SmoothingFilters[i]->SetSigma(sigma);
  }
  m_DerivativeFilter->SetSigma(sigma);

  this->Modified();
}

template <typename TInputImage, typename TOutputImage>
auto
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::GetSigma() const -> RealType
{
  return m_DerivativeFilter->GetSigma();
}

template <typename TInputImage, typename TOutputImage>
void
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::SetNormalizeAcrossScale(bool normalize)
{
  m_NormalizeAcrossScale = normalize;

  for (unsigned int i = 0; i < NumberOfSmoothingFilters; ++i)
  {
    m_SmoothingFilters[i]->SetNormalizeAcrossScale(normalize);
  }
  m_DerivativeFilter->SetNormalizeAcrossScale(normalize);

  this->Modified();
}

template <typename TInputImage, typename TOutputImage>
void
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::EnlargeOutputRequestedRegion(DataObject * output)
{
  auto * out = dynamic_cast<TOutputImage *>(output);

  if (out)
  {
    out->SetRequestedRegion(out->GetLargestPossibleRegion());
  }
}

template <typename TInputImage, typename TOutputImage>
void
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::GenerateData()
{
  itkDebugMacro("LaplacianRecursiveGaussianImageFilter generating data ");

  // Set the number of threads on all the filters
  for (unsigned int i = 0; i < ImageDimension - 1; ++i)
  {
    m_SmoothingFilters[i]->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());
  }
  m_DerivativeFilter->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());

  // Create a process accumulator for tracking the progress of minipipeline
  auto progress = ProgressAccumulator::New();
  progress->SetMiniPipelineFilter(this);

  // dim^2 recursive gaussians + dim add filters + cast filter
  const unsigned int numberOfFilters = (ImageDimension * ImageDimension) + ImageDimension + 1;

  // register (most) filters with the progress accumulator
  for (unsigned int i = 0; i < NumberOfSmoothingFilters; ++i)
  {
    progress->RegisterInternalFilter(m_SmoothingFilters[i], 1.0 / numberOfFilters);
  }
  progress->RegisterInternalFilter(m_DerivativeFilter, 1.0 / numberOfFilters);

  const typename TInputImage::ConstPointer inputImage(this->GetInput());

  // initialize output image
  //
  // NOTE: We intentionally don't allocate the output image here,
  // because the cast image filter will either run inplace, or allocate
  // the output there. The requested region has already been set in
  // ImageToImageFilter::GenerateInputImageFilter.
  const typename TOutputImage::Pointer outputImage(this->GetOutput());
  // outputImage->Allocate(); let the CasterImageFilter allocate the image

  //  Auxiliary image for accumulating the second-order derivatives
  using CumulativeImageType = Image<InternalRealType, Self::ImageDimension>;
  using CumulativeImagePointer = typename CumulativeImageType::Pointer;

  // The CastImageFilter is used because it is multithreaded and
  // it may perform no operation if the two images types are the same
  using CastFilterType = itk::CastImageFilter<CumulativeImageType, OutputImageType>;
  auto caster = CastFilterType::New();
  caster->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());

  // If the last filter is running in-place then this bulk data is not
  // needed, release it to save memory
  if (caster->CanRunInPlace())
  {
    outputImage->ReleaseData();
  }


  CumulativeImagePointer cumulativeImage = CumulativeImageType::New();
  cumulativeImage->SetRegions(outputImage->GetRequestedRegion());
  cumulativeImage->CopyInformation(inputImage);
  cumulativeImage->AllocateInitialized();

  m_DerivativeFilter->SetInput(inputImage);

  // allocate the add and scale image filter just for the scope of
  // this function!
  using AddFilterType = itk::BinaryGeneratorImageFilter<CumulativeImageType, RealImageType, CumulativeImageType>;
  auto addFilter = AddFilterType::New();
  addFilter->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());

  // register with progress accumulator
  progress->RegisterInternalFilter(addFilter, 1.0 / numberOfFilters);


  for (unsigned int dim = 0; dim < ImageDimension; ++dim)
  {
    unsigned int i = 0;
    unsigned int j = 0;
    while (i < NumberOfSmoothingFilters)
    {
      if (i == dim)
      {
        ++j;
      }
      m_SmoothingFilters[i]->SetDirection(j);
      ++i;
      ++j;
    }
    m_DerivativeFilter->SetDirection(dim);

    const GaussianFilterPointer lastFilter = m_SmoothingFilters[ImageDimension - 2];

    // scale the new value by the inverse of the spacing squared
    const RealType spacing2 = itk::Math::sqr(inputImage->GetSpacing()[dim]);
    addFilter->SetFunctor([spacing2](const InternalRealType & a, const InternalRealType & b) {
      return static_cast<InternalRealType>(a + b * (1.0 / spacing2));
    });

    // Cumulate the results on the output image
    addFilter->SetInput1(cumulativeImage);
    addFilter->SetInput2(lastFilter->GetOutput());
    addFilter->InPlaceOn();
    addFilter->Update();

    cumulativeImage = addFilter->GetOutput();
    cumulativeImage->DisconnectPipeline();
  }

  // Because the output of last filter in the mini-pipeline is not
  // pipelined the data must be manually released
  if constexpr (ImageDimension > 1)
  {
    m_SmoothingFilters[ImageDimension - 2]->GetOutput()->ReleaseData();
  }
  else
  {
    m_DerivativeFilter->GetOutput()->ReleaseData();
  }

  // Finally convert the cumulated image to the output with a caster
  caster->SetInput(cumulativeImage);

  // register with progress accumulator
  progress->RegisterInternalFilter(caster, 1.0 / numberOfFilters);

  // graft the our output to the casted output to share the
  // output bulk-data, meta-information and regions, then update the
  // requested image
  caster->GraftOutput(outputImage);
  caster->Update();
  this->GraftOutput(caster->GetOutput());
}

template <typename TInputImage, typename TOutputImage>
void
LaplacianRecursiveGaussianImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "NormalizeAcrossScale: " << m_NormalizeAcrossScale << std::endl;
}
} // end namespace itk

#endif

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
#ifndef itkThresholdLabelerImageFilter_h
#define itkThresholdLabelerImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkConceptChecking.h"

namespace itk
{
/**
 * \class ThresholdLabelerImageFilter
 *
 * \brief Label an input image according to a set of thresholds.
 *
 * This filter produces an output image whose pixels are labeled
 * progressively according to the classes identified by a set of thresholds.
 * Values equal to a threshold is considered to be in the lower class.
 *
 * This filter is templated over the input image type
 * and the output image type.
 *
 * The filter expect both images to have the same number of dimensions.
 *
 * \ingroup IntensityImageFilters  MultiThreaded
 * \ingroup ITKThresholding
 */
namespace Functor
{
template <typename TInput, typename TOutput>
class ITK_TEMPLATE_EXPORT ThresholdLabeler
{
public:
  ThresholdLabeler()
    : m_LabelOffset(NumericTraits<TOutput>::OneValue())
  {}
  ~ThresholdLabeler() = default;

  using RealThresholdType = typename NumericTraits<TInput>::RealType;
  using RealThresholdVector = std::vector<RealThresholdType>;

  /** Set the vector of thresholds. */
  void
  SetThresholds(const RealThresholdVector & thresholds)
  {
    m_Thresholds = thresholds;
  }

  /** Set the offset which labels have to start from. */
  void
  SetLabelOffset(const TOutput & labelOffset)
  {
    m_LabelOffset = labelOffset;
  }


  bool
  operator==(const ThresholdLabeler & other) const
  {
    return m_Thresholds == other.m_Thresholds && m_LabelOffset == other.m_LabelOffset;
  }

  ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(ThresholdLabeler);

  inline TOutput
  operator()(const TInput & A) const
  {
    // When there are N thresholds, they divide values into N+1 buckets, which we number
    // 0, ..., N.  Each bucket represents a half-open interval of values (A, B].  The
    // variables low, mid, and high refer to buckets.  The inclusive range [low, high]
    // are the buckets that are not yet ruled out.  We repeatedly bisect this range
    // using the variable `mid`.  In the case of ties, this method returns the lowest
    // bucket index for which `A` is less than or equal to the bucket's upper limit.
    size_t low = 0;
    size_t high = m_Thresholds.size();
    while (low < high)
    {
      const size_t mid = (low + high) / 2;
      if (A <= m_Thresholds[mid])
      {
        high = mid;
      }
      else
      {
        low = mid + 1;
      }
    }
    // The computed bucket index is relative to m_LabelOffset.
    return static_cast<TOutput>(low) + m_LabelOffset;
  }

private:
  RealThresholdVector m_Thresholds;
  TOutput             m_LabelOffset;
};
} // namespace Functor

template <typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT ThresholdLabelerImageFilter
  : public UnaryFunctorImageFilter<
      TInputImage,
      TOutputImage,
      Functor::ThresholdLabeler<typename TInputImage::PixelType, typename TOutputImage::PixelType>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ThresholdLabelerImageFilter);

  /** Standard class type aliases. */
  using Self = ThresholdLabelerImageFilter;
  using Superclass = UnaryFunctorImageFilter<
    TInputImage,
    TOutputImage,
    Functor::ThresholdLabeler<typename TInputImage::PixelType, typename TOutputImage::PixelType>>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ThresholdLabelerImageFilter);

  /** Pixel types. */
  using InputPixelType = typename TInputImage::PixelType;
  using OutputPixelType = typename TOutputImage::PixelType;

  /** Threshold vector types. */
  using ThresholdVector = std::vector<InputPixelType>;
  using RealThresholdType = typename NumericTraits<InputPixelType>::RealType;
  using RealThresholdVector = std::vector<RealThresholdType>;

  /** The input and output pixel types must support comparison operators. */
  /** @ITKStartGrouping */
  itkConceptMacro(PixelTypeComparable, (Concept::Comparable<InputPixelType>));
  itkConceptMacro(OutputPixelTypeComparable, (Concept::Comparable<OutputPixelType>));
  itkConceptMacro(OutputPixelTypeOStreamWritable, (Concept::OStreamWritable<OutputPixelType>));
  /** @ITKEndGrouping */
  /** Set the vector of thresholds. */
  void
  SetThresholds(const ThresholdVector & thresholds)
  {
    m_Thresholds = thresholds;
    m_RealThresholds.clear();
    auto itr = m_Thresholds.begin();
    while (itr != m_Thresholds.end())
    {
      m_RealThresholds.push_back(static_cast<RealThresholdType>(*itr));
      ++itr;
    }
    this->Modified();
  }

  /** Get the vector of thresholds. */
  const ThresholdVector &
  GetThresholds() const
  {
    return m_Thresholds;
  }

  /** Set the vector of real type thresholds. */
  void
  SetRealThresholds(const RealThresholdVector & thresholds)
  {
    m_RealThresholds = thresholds;
    m_Thresholds.clear();
    auto itr = m_RealThresholds.begin();
    while (itr != m_RealThresholds.end())
    {
      m_Thresholds.push_back(static_cast<InputPixelType>(*itr));
      ++itr;
    }
    this->Modified();
  }

  /** Get the vector of real thresholds. */
  const RealThresholdVector &
  GetRealThresholds() const
  {
    return m_RealThresholds;
  }

  /** Set the offset which labels have to start from. */
  /** @ITKStartGrouping */
  itkSetClampMacro(LabelOffset, OutputPixelType, OutputPixelType{}, NumericTraits<OutputPixelType>::max());
  itkGetConstMacro(LabelOffset, OutputPixelType);
  /** @ITKEndGrouping */
protected:
  ThresholdLabelerImageFilter();
  ~ThresholdLabelerImageFilter() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** This method is used to set the state of the filter before
   * multi-threading. */
  void
  BeforeThreadedGenerateData() override;

private:
  ThresholdVector     m_Thresholds{};
  RealThresholdVector m_RealThresholds{};
  OutputPixelType     m_LabelOffset{};
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkThresholdLabelerImageFilter.hxx"
#endif

#endif

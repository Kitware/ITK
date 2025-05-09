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
#ifndef itkNormalizedMutualInformationHistogramImageToImageMetric_hxx
#define itkNormalizedMutualInformationHistogramImageToImageMetric_hxx

#include "itkHistogram.h"

namespace itk
{
template <typename TFixedImage, typename TMovingImage>
auto
NormalizedMutualInformationHistogramImageToImageMetric<TFixedImage, TMovingImage>::EvaluateMeasure(
  HistogramType & histogram) const -> MeasureType
{
  MeasureType entropyX{};
  MeasureType entropyY{};
  MeasureType jointEntropy{};

  using HistogramFrequencyRealType = typename NumericTraits<HistogramFrequencyType>::RealType;

  auto totalFreq = static_cast<HistogramFrequencyRealType>(histogram.GetTotalFrequency());

  for (unsigned int i = 0; i < this->GetHistogramSize()[0]; ++i)
  {
    auto freq = static_cast<HistogramFrequencyRealType>(histogram.GetFrequency(i, 0));

    if (freq > 0)
    {
      entropyX += freq * std::log(freq);
    }
  }

  entropyX = -entropyX / static_cast<MeasureType>(totalFreq) + std::log(totalFreq);

  for (unsigned int i = 0; i < this->GetHistogramSize()[1]; ++i)
  {
    auto freq = static_cast<HistogramFrequencyRealType>(histogram.GetFrequency(i, 1));

    if (freq > 0)
    {
      entropyY += freq * std::log(freq);
    }
  }

  entropyY = -entropyY / static_cast<MeasureType>(totalFreq) + std::log(totalFreq);

  HistogramIteratorType       it = histogram.Begin();
  const HistogramIteratorType end = histogram.End();
  while (it != end)
  {
    auto freq = static_cast<HistogramFrequencyRealType>(it.GetFrequency());

    if (freq > 0)
    {
      jointEntropy += freq * std::log(freq);
    }
    ++it;
  }

  jointEntropy = -jointEntropy / static_cast<MeasureType>(totalFreq) + std::log(totalFreq);

  return (entropyX + entropyY) / jointEntropy;
}
} // End namespace itk

#endif // itkNormalizedMutualInformationHistogramImageToImageMetric_hxx

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
#ifndef itkScalarImageToCooccurrenceListSampleFilter_hxx
#define itkScalarImageToCooccurrenceListSampleFilter_hxx

namespace itk::Statistics
{
template <typename TImage>
ScalarImageToCooccurrenceListSampleFilter<TImage>::ScalarImageToCooccurrenceListSampleFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->ProcessObject::SetNumberOfRequiredOutputs(1);

  this->ProcessObject::SetNthOutput(0, this->MakeOutput(0));
}

template <typename TImage>
void
ScalarImageToCooccurrenceListSampleFilter<TImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

template <typename TImage>
void
ScalarImageToCooccurrenceListSampleFilter<TImage>::SetInput(const ImageType * image)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<ImageType *>(image));
}

template <typename TImage>
const TImage *
ScalarImageToCooccurrenceListSampleFilter<TImage>::GetInput() const
{
  return itkDynamicCastInDebugMode<const ImageType *>(this->GetPrimaryInput());
}

template <typename TImage>
auto
ScalarImageToCooccurrenceListSampleFilter<TImage>::GetOutput() const -> const SampleType *
{
  const auto * output = static_cast<const SampleType *>(this->ProcessObject::GetOutput(0));

  return output;
}

template <typename TImage>
typename ScalarImageToCooccurrenceListSampleFilter<TImage>::DataObjectPointer
ScalarImageToCooccurrenceListSampleFilter<TImage>::MakeOutput(DataObjectPointerArraySizeType)
{
  return SampleType::New().GetPointer();
}

template <typename TImage>
void
ScalarImageToCooccurrenceListSampleFilter<TImage>::GenerateData()
{
  constexpr auto radius = MakeFilled<typename ShapedNeighborhoodIteratorType::RadiusType>(1);

  using FaceCalculatorType = itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<ImageType>;

  FaceCalculatorType faceCalculator;

  using ShapeNeighborhoodIterator = typename ShapedNeighborhoodIteratorType::ConstIterator;

  using PixelType = typename ImageType::PixelType;

  typename SampleType::MeasurementVectorType coords;

  const ImageType * input = this->GetInput();

  auto * output = static_cast<SampleType *>(this->ProcessObject::GetOutput(0));

  // constant for a cooccurrence matrix.
  constexpr unsigned int measurementVectorSize = 2;

  output->SetMeasurementVectorSize(measurementVectorSize);

  const typename FaceCalculatorType::FaceListType faceList = faceCalculator(input, input->GetRequestedRegion(), radius);

  constexpr OffsetType center_offset{};

  for (const auto & face : faceList)
  {
    ShapedNeighborhoodIteratorType it(radius, input, face);

    auto iter = m_OffsetTable.begin();
    while (iter != m_OffsetTable.end())
    {
      it.ActivateOffset(*iter);
      ++iter;
    }

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      const PixelType center_pixel_intensity = it.GetPixel(center_offset);

      ShapeNeighborhoodIterator ci = it.Begin();
      while (ci != it.End())
      {

        // Check if the point is inside and add the measurement vector
        // only if its inside
        bool            isInside = false;
        const PixelType pixel_intensity = it.GetPixel(ci.GetNeighborhoodIndex(), isInside);
        if (isInside)
        {

          // We have the intensity values for the center pixel and one of it's
          // neighbours.
          // We can now place these in the SampleList
          coords[0] = center_pixel_intensity;
          coords[1] = pixel_intensity;

          output->PushBack(coords);
          // std::cout << "Pushing: " << coords[0] << '\t' << coords[1] <<
          // std::endl;
        }

        ++ci;
      }
    }
  }
}

template <typename TImage>
void
ScalarImageToCooccurrenceListSampleFilter<TImage>::UseNeighbor(const OffsetType & offset)
{
  // Don't add the center pixel
  bool isTheCenterPixel = true;

  for (unsigned int i = 0; i < ImageDimension; ++i)
  {
    if (offset[i] != 0)
    {
      isTheCenterPixel = false;
      break;
    }
  }

  if (!isTheCenterPixel)
  {
    m_OffsetTable.push_back(offset);
  }
}
} // namespace itk::Statistics

#endif

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
#ifndef itkSimilarityIndexImageFilter_hxx
#define itkSimilarityIndexImageFilter_hxx

#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"
#include "itkMath.h"

namespace itk
{
template <typename TInputImage1, typename TInputImage2>
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::SimilarityIndexImageFilter()
  : m_SimilarityIndex(RealType{})
  , m_CountOfImage1(1)
  , m_CountOfImage2(1)
  , m_CountOfIntersection(1)
{
  // this filter requires two input images
  this->SetNumberOfRequiredInputs(2);
  this->DynamicMultiThreadingOff();
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::SetInput2(const TInputImage2 * image)
{
  this->SetNthInput(1, const_cast<TInputImage2 *>(image));
}

template <typename TInputImage1, typename TInputImage2>
auto
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::GetInput2() -> const InputImage2Type *
{
  return itkDynamicCastInDebugMode<const TInputImage2 *>(this->ProcessObject::GetInput(1));
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  // this filter requires:
  // - the largest possible region of the first image
  // - the corresponding region of the second image
  if (this->GetInput1())
  {
    const InputImage1Pointer image1 = const_cast<InputImage1Type *>(this->GetInput1());
    image1->SetRequestedRegionToLargestPossibleRegion();

    if (this->GetInput2())
    {
      const InputImage2Pointer image2 = const_cast<InputImage2Type *>(this->GetInput2());
      image2->SetRequestedRegion(this->GetInput1()->GetRequestedRegion());
    }
  }
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::EnlargeOutputRequestedRegion(DataObject * data)
{
  Superclass::EnlargeOutputRequestedRegion(data);
  data->SetRequestedRegionToLargestPossibleRegion();
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::AllocateOutputs()
{
  // Pass the first input through as the output
  const InputImage1Pointer image = const_cast<TInputImage1 *>(this->GetInput1());

  this->GraftOutput(image);
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::BeforeThreadedGenerateData()
{
  const ThreadIdType numberOfWorkUnits = this->GetNumberOfWorkUnits();

  // Resize the thread temporaries
  m_CountOfImage1.SetSize(numberOfWorkUnits);
  m_CountOfImage2.SetSize(numberOfWorkUnits);
  m_CountOfIntersection.SetSize(numberOfWorkUnits);

  // Initialize the temporaries
  m_CountOfImage1.Fill(SizeValueType{});
  m_CountOfImage2.Fill(SizeValueType{});
  m_CountOfIntersection.Fill(SizeValueType{});
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::AfterThreadedGenerateData()
{
  const ThreadIdType numberOfWorkUnits = this->GetNumberOfWorkUnits();

  SizeValueType countImage1 = 0;
  SizeValueType countImage2 = 0;
  SizeValueType countIntersect = 0;

  // Accumulate counts over all threads
  for (ThreadIdType i = 0; i < numberOfWorkUnits; ++i)
  {
    countImage1 += m_CountOfImage1[i];
    countImage2 += m_CountOfImage2[i];
    countIntersect += m_CountOfIntersection[i];
  }

  // compute overlap
  if (!countImage1 && !countImage2)
  {
    m_SimilarityIndex = RealType{};
    return;
  }

  m_SimilarityIndex = 2.0 * static_cast<RealType>(countIntersect) /
                      (static_cast<RealType>(countImage1) + static_cast<RealType>(countImage2));
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::ThreadedGenerateData(const RegionType & outputRegionForThread,
                                                                             ThreadIdType       threadId)
{
  ImageRegionConstIterator<TInputImage1> it1(this->GetInput1(), outputRegionForThread);
  ImageRegionConstIterator<TInputImage2> it2(this->GetInput2(), outputRegionForThread);

  // support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // do the work
  while (!it1.IsAtEnd())
  {
    bool nonzero = false;
    if (it1.Get() != InputImage1PixelType{})
    {
      m_CountOfImage1[threadId]++;
      nonzero = true;
    }
    if (Math::NotExactlyEquals(it2.Get(), InputImage2PixelType{}))
    {
      m_CountOfImage2[threadId]++;
      if (nonzero)
      {
        m_CountOfIntersection[threadId]++;
      }
    }
    ++it1;
    ++it2;

    progress.CompletedPixel();
  }
}

template <typename TInputImage1, typename TInputImage2>
void
SimilarityIndexImageFilter<TInputImage1, TInputImage2>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SimilarityIndex: " << m_SimilarityIndex << std::endl;
}
} // end namespace itk
#endif

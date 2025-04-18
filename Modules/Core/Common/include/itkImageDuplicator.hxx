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
#ifndef itkImageDuplicator_hxx
#define itkImageDuplicator_hxx

#include "itkImageAlgorithm.h"

namespace itk
{

template <typename TInputImage>
void
ImageDuplicator<TInputImage>::Update()
{
  if (!m_InputImage)
  {
    itkExceptionMacro("Input image has not been connected");
  }

  // Update only if the input image has been modified
  const ModifiedTimeType t1 = m_InputImage->GetPipelineMTime();
  const ModifiedTimeType t2 = m_InputImage->GetMTime();
  const ModifiedTimeType t = (t1 > t2 ? t1 : t2);

  if (t == m_InternalImageTime)
  {
    return; // No need to update
  }

  // Cache the timestamp
  m_InternalImageTime = t;

  // Allocate the image
  m_DuplicateImage = ImageType::New();
  m_DuplicateImage->CopyInformation(m_InputImage);
  m_DuplicateImage->SetRequestedRegion(m_InputImage->GetRequestedRegion());
  m_DuplicateImage->SetBufferedRegion(m_InputImage->GetBufferedRegion());
  m_DuplicateImage->Allocate();
  const typename ImageType::RegionType region = m_InputImage->GetBufferedRegion();
  ImageAlgorithm::Copy(m_InputImage.GetPointer(), m_DuplicateImage.GetPointer(), region, region);
}

template <typename TInputImage>
void
ImageDuplicator<TInputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfObjectMacro(InputImage);
  itkPrintSelfObjectMacro(DuplicateImage);

  os << indent
     << "InternalImageTime: " << static_cast<typename NumericTraits<ModifiedTimeType>::PrintType>(m_InternalImageTime)
     << std::endl;
}
} // end namespace itk

#endif

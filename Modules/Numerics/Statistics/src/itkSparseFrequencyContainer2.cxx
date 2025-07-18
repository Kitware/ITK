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
#include "itkSparseFrequencyContainer2.h"

namespace itk::Statistics
{
SparseFrequencyContainer2::SparseFrequencyContainer2()
  : m_TotalFrequency(TotalAbsoluteFrequencyType{})
{}

void
SparseFrequencyContainer2::Initialize(SizeValueType)
{
  this->SetToZero();
}

void
SparseFrequencyContainer2::SetToZero()
{
  auto iter = m_FrequencyContainer.begin();
  auto end = m_FrequencyContainer.end();
  while (iter != end)
  {
    iter->second = AbsoluteFrequencyType{};
    ++iter;
  }
  m_TotalFrequency = TotalAbsoluteFrequencyType{};
}

bool
SparseFrequencyContainer2::SetFrequency(const InstanceIdentifier id, const AbsoluteFrequencyType value)
{
  // No need to test for bounds because in a map container the
  // element is allocated if the key doesn't exist yet
  const AbsoluteFrequencyType frequency = this->GetFrequency(id);

  m_FrequencyContainer[id] = value;
  m_TotalFrequency += (value - frequency);
  return true;
}

SparseFrequencyContainer2::AbsoluteFrequencyType
SparseFrequencyContainer2::GetFrequency(const InstanceIdentifier id) const
{
  auto iter = m_FrequencyContainer.find(id);

  if (iter != m_FrequencyContainer.end())
  {
    return iter->second;
  }

  return 0;
}

bool
SparseFrequencyContainer2::IncreaseFrequency(const InstanceIdentifier id, const AbsoluteFrequencyType value)
{
  // No need to test for bounds because in a map container the
  // element is allocated if the key doesn't exist yet
  const AbsoluteFrequencyType frequency = this->GetFrequency(id);

  m_FrequencyContainer[id] = frequency + value;
  m_TotalFrequency += value;
  return true;
}

void
SparseFrequencyContainer2::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // namespace itk::Statistics

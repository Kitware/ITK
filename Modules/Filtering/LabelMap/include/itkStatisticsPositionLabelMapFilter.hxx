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
#ifndef itkStatisticsPositionLabelMapFilter_hxx
#define itkStatisticsPositionLabelMapFilter_hxx

#include "itkLabelMapUtilities.h"
#include "itkStatisticsLabelObjectAccessors.h"


namespace itk
{

template <typename TImage>
StatisticsPositionLabelMapFilter<TImage>::StatisticsPositionLabelMapFilter()
{
  this->m_Attribute = LabelObjectType::CENTER_OF_GRAVITY;
}


template <typename TImage>
void
StatisticsPositionLabelMapFilter<TImage>::ThreadedProcessLabelObject(LabelObjectType * labelObject)
{
  switch (this->m_Attribute)
  {
    case LabelObjectType::MAXIMUM_INDEX:
    {
      using AccessorType = typename Functor::MaximumIndexLabelObjectAccessor<LabelObjectType>;
      const AccessorType accessor;
      this->TemplatedThreadedProcessLabelObject(accessor, false, labelObject);
      break;
    }
    case LabelObjectType::MINIMUM_INDEX:
    {
      using AccessorType = typename Functor::MinimumIndexLabelObjectAccessor<LabelObjectType>;
      const AccessorType accessor;
      this->TemplatedThreadedProcessLabelObject(accessor, false, labelObject);
      break;
    }
    case LabelObjectType::CENTER_OF_GRAVITY:
    {
      using AccessorType = typename Functor::CenterOfGravityLabelObjectAccessor<LabelObjectType>;
      const AccessorType accessor;
      this->TemplatedThreadedProcessLabelObject(accessor, true, labelObject);
      break;
    }
    default:
      Superclass::ThreadedProcessLabelObject(labelObject);
      break;
  }
}

} // end namespace itk
#endif

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
#ifndef itkInputDataObjectConstIterator_h
#define itkInputDataObjectConstIterator_h

#include "itkDataObjectConstIterator.h"

namespace itk
{
/** \class InputDataObjectConstIterator
 * \brief A forward iterator over inputs of a ProcessObject
 * \author Gaetan Lehmann. Biologie du Developpement et de la Reproduction, INRA de Jouy-en-Josas, France.
 * \ingroup ITKCommon
 */
class InputDataObjectConstIterator : public DataObjectConstIterator
{
public:
  InputDataObjectConstIterator(const ProcessObject * process)
  {
    m_Begin = process->m_Inputs.begin();
    m_End = process->m_Inputs.end();
    m_Iterator = m_Begin;
  }

  [[nodiscard]] const DataObject *
  GetInput() const
  {
    return m_Iterator->second;
  }
};

} // namespace itk
#endif

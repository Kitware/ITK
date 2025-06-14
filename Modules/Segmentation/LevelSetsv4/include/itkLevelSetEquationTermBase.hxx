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

#ifndef itkLevelSetEquationTermBase_hxx
#define itkLevelSetEquationTermBase_hxx

#include "itkNumericTraits.h"
#include "itkMath.h"

namespace itk
{
// ----------------------------------------------------------------------------
template <typename TInputImage, typename TLevelSetContainer>
LevelSetEquationTermBase<TInputImage, TLevelSetContainer>::LevelSetEquationTermBase()
  : m_CurrentLevelSetId(LevelSetIdentifierType())
  , m_Coefficient(NumericTraits<LevelSetOutputRealType>::OneValue())
  , m_CFLContribution(LevelSetOutputRealType{})
  , m_TermName("")
{}

// ----------------------------------------------------------------------------
template <typename TInputImage, typename TLevelSetContainer>
auto
LevelSetEquationTermBase<TInputImage, TLevelSetContainer>::GetRequiredData() const -> const RequiredDataType &
{
  return this->m_RequiredData;
}

// ----------------------------------------------------------------------------
template <typename TInputImage, typename TLevelSetContainer>
void
LevelSetEquationTermBase<TInputImage, TLevelSetContainer>::SetLevelSetContainer(LevelSetContainerType * iContainer)
{
  if (iContainer)
  {
    this->m_LevelSetContainer = iContainer;
    this->m_Heaviside = iContainer->GetHeaviside();
    this->Modified();
  }
  else
  {
    itkGenericExceptionMacro("iContainer is nullptr");
  }
}

// ----------------------------------------------------------------------------
template <typename TInputImage, typename TLevelSetContainer>
auto
LevelSetEquationTermBase<TInputImage, TLevelSetContainer>::Evaluate(const LevelSetInputIndexType & iP)
  -> LevelSetOutputRealType
{
  if (itk::Math::abs(this->m_Coefficient) > NumericTraits<LevelSetOutputRealType>::epsilon())
  {
    return this->m_Coefficient * this->Value(iP);
  }

  return LevelSetOutputRealType{};
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
template <typename TInputImage, typename TLevelSetContainer>
auto
LevelSetEquationTermBase<TInputImage, TLevelSetContainer>::Evaluate(const LevelSetInputIndexType & iP,
                                                                    const LevelSetDataType &       iData)
  -> LevelSetOutputRealType
{
  if (itk::Math::abs(this->m_Coefficient) > NumericTraits<LevelSetOutputRealType>::epsilon())
  {
    return this->m_Coefficient * this->Value(iP, iData);
  }

  return LevelSetOutputRealType{};
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
template <typename TInputImage, typename TLevelSetContainer>
void
LevelSetEquationTermBase<TInputImage, TLevelSetContainer>::SetUp()
{
  this->m_CFLContribution = LevelSetOutputRealType{};

  if (this->m_CurrentLevelSetPointer.IsNull())
  {
    if (this->m_LevelSetContainer.IsNull())
    {
      itkGenericExceptionMacro("m_LevelSetContainer is nullptr");
    }
    this->m_CurrentLevelSetPointer = this->m_LevelSetContainer->GetLevelSet(this->m_CurrentLevelSetId);

    if (this->m_CurrentLevelSetPointer.IsNull())
    {
      itkWarningMacro("m_CurrentLevelSetId does not exist in the level set container");
    }
  }

  if (!this->m_Heaviside.IsNotNull())
  {
    itkWarningMacro("m_Heaviside is nullptr");
  }
}
// ----------------------------------------------------------------------------

} // namespace itk

#endif // itkLevelSetEquationTermBase_hxx

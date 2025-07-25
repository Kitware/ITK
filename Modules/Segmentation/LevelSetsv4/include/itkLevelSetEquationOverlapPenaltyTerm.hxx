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

#ifndef itkLevelSetEquationOverlapPenaltyTerm_hxx
#define itkLevelSetEquationOverlapPenaltyTerm_hxx


namespace itk
{

template <typename TInput, typename TLevelSetContainer>
LevelSetEquationOverlapPenaltyTerm<TInput, TLevelSetContainer>::LevelSetEquationOverlapPenaltyTerm()
  : m_DomainMapImageFilter(nullptr)
  , m_CacheImage(nullptr)
{
  this->m_TermName = "Overlap term";
  this->m_RequiredData.insert("");
}

template <typename TInput, typename TLevelSetContainer>
void
LevelSetEquationOverlapPenaltyTerm<TInput, TLevelSetContainer>::InitializeParameters()
{
  this->SetUp();
}


template <typename TInput, typename TLevelSetContainer>
auto
LevelSetEquationOverlapPenaltyTerm<TInput, TLevelSetContainer>::Value(const LevelSetInputIndexType & index)
  -> LevelSetOutputRealType
{
  LevelSetOutputRealType value{};
  this->ComputeSumTerm(index, value);
  return -value;
}

template <typename TInput, typename TLevelSetContainer>
auto
LevelSetEquationOverlapPenaltyTerm<TInput, TLevelSetContainer>::Value(const LevelSetInputIndexType & index,
                                                                      const LevelSetDataType &       itkNotUsed(data))
  -> LevelSetOutputRealType
{
  LevelSetOutputRealType value{};
  this->ComputeSumTerm(index, value);
  return -value;
}

template <typename TInput, typename TLevelSetContainer>
void
LevelSetEquationOverlapPenaltyTerm<TInput, TLevelSetContainer>::ComputeSumTerm(const LevelSetInputIndexType & index,
                                                                               LevelSetOutputRealType &       sum)
{
  CompensatedSummationType compensatedSummation;
  compensatedSummation.ResetToZero();

  if (this->m_LevelSetContainer->HasDomainMap())
  {
    if (this->m_DomainMapImageFilter == nullptr)
    {
      this->m_DomainMapImageFilter = this->m_LevelSetContainer->GetModifiableDomainMapFilter();
      this->m_CacheImage = this->m_DomainMapImageFilter->GetOutput();
    }
    const LevelSetIdentifierType idx = this->m_CacheImage->GetPixel(index);

    using DomainMapType = typename DomainMapImageFilterType::DomainMapType;
    const DomainMapType domainMap = this->m_DomainMapImageFilter->GetDomainMap();
    auto                levelSetMapItr = domainMap.find(idx);

    if (levelSetMapItr != domainMap.end())
    {
      const IdListType * idList = levelSetMapItr->second.GetIdList();

      LevelSetIdentifierType kk;
      LevelSetPointer        levelSet;
      LevelSetOutputRealType value;

      auto idListIt = idList->begin();
      while (idListIt != idList->end())
      {
        //! \todo Fix me for string identifiers
        kk = *idListIt - 1;
        if (kk != this->m_CurrentLevelSetId)
        {
          levelSet = this->m_LevelSetContainer->GetLevelSet(kk);
          value = levelSet->Evaluate(index);
          compensatedSummation += -this->m_Heaviside->Evaluate(-value);
        }
        ++idListIt;
      }
    }
  }
  else
  {
    LevelSetIdentifierType kk;
    LevelSetPointer        levelSet;
    LevelSetOutputRealType value;

    typename LevelSetContainerType::Iterator lsIt = this->m_LevelSetContainer->Begin();

    while (lsIt != this->m_LevelSetContainer->End())
    {
      kk = lsIt->GetIdentifier();
      if (kk != this->m_CurrentLevelSetId)
      {
        levelSet = this->m_LevelSetContainer->GetLevelSet(kk);
        value = levelSet->Evaluate(index);
        compensatedSummation += -this->m_Heaviside->Evaluate(-value);
      }
      ++lsIt;
    }
  }
  sum += compensatedSummation.GetSum();
}

} // namespace itk
#endif

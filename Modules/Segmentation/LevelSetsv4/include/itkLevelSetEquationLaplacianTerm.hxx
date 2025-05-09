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

#ifndef itkLevelSetEquationLaplacianTerm_hxx
#define itkLevelSetEquationLaplacianTerm_hxx


namespace itk
{
template <typename TInput, typename TLevelSetContainer>
LevelSetEquationLaplacianTerm<TInput, TLevelSetContainer>::LevelSetEquationLaplacianTerm()
{
  this->m_TermName = "Laplacian term";
  this->m_RequiredData.insert("Laplacian");
}

template <typename TInput, typename TLevelSetContainer>
void
LevelSetEquationLaplacianTerm<TInput, TLevelSetContainer>::InitializeParameters()
{
  this->SetUp();
}

template <typename TInput, typename TLevelSetContainer>
auto
LevelSetEquationLaplacianTerm<TInput, TLevelSetContainer>::LaplacianSpeed(
  const LevelSetInputIndexType & itkNotUsed(iP)) const -> LevelSetOutputRealType
{
  return NumericTraits<LevelSetOutputRealType>::OneValue();
}

template <typename TInput, typename TLevelSetContainer>
auto
LevelSetEquationLaplacianTerm<TInput, TLevelSetContainer>::Value(const LevelSetInputIndexType & iP)
  -> LevelSetOutputRealType
{
  LevelSetOutputRealType laplacian = this->m_CurrentLevelSetPointer->EvaluateLaplacian(iP);

  laplacian *= this->LaplacianSpeed(iP);

  return laplacian;
}

template <typename TInput, typename TLevelSetContainer>
auto
LevelSetEquationLaplacianTerm<TInput, TLevelSetContainer>::Value(const LevelSetInputIndexType & iP,
                                                                 const LevelSetDataType &       iData)
  -> LevelSetOutputRealType
{
  // Laplacian should be computed by this point.
  itkAssertInDebugAndIgnoreInReleaseMacro(iData.Laplacian.m_Computed);

  LevelSetOutputRealType laplacian = iData.Laplacian.m_Value;

  laplacian *= this->LaplacianSpeed(iP);

  return laplacian;
}

} // namespace itk

#endif

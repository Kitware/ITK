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

#ifndef itkLevelSetEquationCurvatureTerm_hxx
#define itkLevelSetEquationCurvatureTerm_hxx


namespace itk
{
template <typename TInput, typename TLevelSetContainer, typename TCurvatureImage>
LevelSetEquationCurvatureTerm<TInput, TLevelSetContainer, TCurvatureImage>::LevelSetEquationCurvatureTerm()
{
  for (unsigned int i = 0; i < ImageDimension; ++i)
  {
    this->m_NeighborhoodScales[i] = 1.0;
  }
  this->m_TermName = "Curvature term";
  this->m_RequiredData.insert("MeanCurvature");
  this->m_UseCurvatureImage = false;
}

template <typename TInput, typename TLevelSetContainer, typename TCurvatureImage>
void
LevelSetEquationCurvatureTerm<TInput, TLevelSetContainer, TCurvatureImage>::SetCurvatureImage(
  CurvatureImageType * iImage)
{
  m_CurvatureImage = iImage;
  m_UseCurvatureImage = true;
  this->Modified();
}

template <typename TInput, typename TLevelSetContainer, typename TCurvatureImage>
auto
LevelSetEquationCurvatureTerm<TInput, TLevelSetContainer, TCurvatureImage>::Value(const LevelSetInputIndexType & iP,
                                                                                  const LevelSetDataType &       iData)
  -> LevelSetOutputRealType
{
  // MeanCurvature has should be computed by this point.
  itkAssertInDebugAndIgnoreInReleaseMacro(iData.MeanCurvature.m_Computed);

  if (!m_UseCurvatureImage)
  {
    return iData.MeanCurvature.m_Value;
  }

  return m_CurvatureImage->GetPixel(iP) * iData.MeanCurvature.m_Value;
}

template <typename TInput, typename TLevelSetContainer, typename TCurvatureImage>
void
LevelSetEquationCurvatureTerm<TInput, TLevelSetContainer, TCurvatureImage>::InitializeParameters()
{
  this->SetUp();

  if (m_UseCurvatureImage)
  {
    if (m_CurvatureImage.IsNull())
    {
      itkGenericExceptionMacro("m_UseCurvatureImage is true and m_CurvatureImage is null");
    }
  }
}

template <typename TInput, typename TLevelSetContainer, typename TCurvatureImage>
auto
LevelSetEquationCurvatureTerm<TInput, TLevelSetContainer, TCurvatureImage>::Value(const LevelSetInputIndexType & iP)
  -> LevelSetOutputRealType
{
  if (!m_UseCurvatureImage)
  {
    return this->m_CurrentLevelSetPointer->EvaluateMeanCurvature(iP);
  }

  return m_CurvatureImage->GetPixel(iP) * this->m_CurrentLevelSetPointer->EvaluateMeanCurvature(iP);
}

} // namespace itk
#endif

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
#ifndef itkDiscreteMinimumCurvatureQuadEdgeMeshFilter_h
#define itkDiscreteMinimumCurvatureQuadEdgeMeshFilter_h

#include "itkDiscretePrincipalCurvaturesQuadEdgeMeshFilter.h"

namespace itk
{
/**
 * \class DiscreteMinimumCurvatureQuadEdgeMeshFilter
 *
 * \brief FIXME    add documentation here
 *
 * \ingroup ITKQuadEdgeMeshFiltering
 */
template <typename TInputMesh, typename TOutputMesh = TInputMesh>
class DiscreteMinimumCurvatureQuadEdgeMeshFilter
  : public DiscretePrincipalCurvaturesQuadEdgeMeshFilter<TInputMesh, TOutputMesh>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(DiscreteMinimumCurvatureQuadEdgeMeshFilter);

  using Self = DiscreteMinimumCurvatureQuadEdgeMeshFilter;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = DiscretePrincipalCurvaturesQuadEdgeMeshFilter<TInputMesh, TOutputMesh>;

  using typename Superclass::InputMeshType;
  using typename Superclass::InputMeshPointer;

  using typename Superclass::OutputMeshType;
  using typename Superclass::OutputMeshPointer;
  using typename Superclass::OutputPointsContainerPointer;
  using typename Superclass::OutputPointsContainerIterator;
  using typename Superclass::OutputPointType;
  using typename Superclass::OutputVectorType;
  using typename Superclass::OutputCoordType;
  using typename Superclass::OutputPointIdentifier;
  using typename Superclass::OutputCellIdentifier;
  using typename Superclass::OutputQEType;
  using typename Superclass::OutputMeshTraits;
  using typename Superclass::OutputCurvatureType;

  using typename Superclass::TriangleType;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(DiscreteMinimumCurvatureQuadEdgeMeshFilter);

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro(Self);

  itkConceptMacro(OutputIsFloatingPointCheck, (Concept::IsFloatingPoint<OutputCurvatureType>));

protected:
  DiscreteMinimumCurvatureQuadEdgeMeshFilter() = default;
  ~DiscreteMinimumCurvatureQuadEdgeMeshFilter() override = default;

  OutputCurvatureType
  EstimateCurvature(const OutputPointType & iP) override
  {
    this->ComputeMeanAndGaussianCurvatures(iP);
    return this->m_Mean - std::sqrt(this->ComputeDelta());
  }
};
} // namespace itk

#endif

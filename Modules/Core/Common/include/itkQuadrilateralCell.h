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
#ifndef itkQuadrilateralCell_h
#define itkQuadrilateralCell_h

#include "itkLineCell.h"
#include "itkQuadrilateralCellTopology.h"
#include "itkMakeFilled.h"

#include <array>

namespace itk
{
/** \class QuadrilateralCell
 *  \brief Represents a quadrilateral for a Mesh.
 *
 * \ingroup MeshObjects
 * \ingroup ITKCommon
 */

template <typename TCellInterface>
class ITK_TEMPLATE_EXPORT QuadrilateralCell
  : public TCellInterface
  , private QuadrilateralCellTopology
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(QuadrilateralCell);

  /** Standard class type aliases. */
  /** @ITKStartGrouping */
  itkCellCommonTypedefs(QuadrilateralCell);
  itkCellInheritedTypedefs(TCellInterface);
  /** @ITKEndGrouping */
  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(QuadrilateralCell);

  /** The type of boundary for this triangle's vertices. */
  using VertexType = VertexCell<TCellInterface>;
  using VertexAutoPointer = typename VertexType::SelfAutoPointer;

  /** The type of boundary for this triangle's edges. */
  using EdgeType = LineCell<TCellInterface>;
  using EdgeAutoPointer = typename EdgeType::SelfAutoPointer;

  /** Quadrilateral-specific topology numbers. */
  static constexpr unsigned int NumberOfPoints = 4;
  static constexpr unsigned int NumberOfVertices = 4;
  static constexpr unsigned int NumberOfEdges = 4;
  static constexpr unsigned int CellDimension = 2;
  static constexpr unsigned int NumberOfDerivatives = 8;

  /** Implement the standard CellInterface. */
  /** @ITKStartGrouping */
  [[nodiscard]] CellGeometryEnum
  GetType() const override
  {
    return CellGeometryEnum::QUADRILATERAL_CELL;
  }
  void
  MakeCopy(CellAutoPointer &) const override;
  /** @ITKEndGrouping */
  [[nodiscard]] unsigned int
  GetDimension() const override;

  [[nodiscard]] unsigned int
  GetNumberOfPoints() const override;

  CellFeatureCount
  GetNumberOfBoundaryFeatures(int dimension) const override;

  bool
  GetBoundaryFeature(int dimension, CellFeatureIdentifier, CellAutoPointer &) override;
  void
  SetPointIds(PointIdConstIterator first) override;

  void
  SetPointIds(PointIdConstIterator first, PointIdConstIterator last) override;

  void
  SetPointId(int localId, PointIdentifier) override;
  PointIdIterator
  PointIdsBegin() override;

  PointIdConstIterator
  PointIdsBegin() const override;

  PointIdIterator
  PointIdsEnd() override;

  PointIdConstIterator
  PointIdsEnd() const override;

  /** Quadrilateral-specific interface. */
  virtual CellFeatureCount
  GetNumberOfVertices() const;

  virtual CellFeatureCount
  GetNumberOfEdges() const;

  virtual bool
  GetVertex(CellFeatureIdentifier, VertexAutoPointer &);
  virtual bool
  GetEdge(CellFeatureIdentifier, EdgeAutoPointer &);

  /** Evaluate the position inside the cell */
  bool
  EvaluatePosition(CoordinateType *  x,
                   PointsContainer * points,
                   CoordinateType *  closestPoint,
                   CoordinateType[CellDimension],
                   double *                  dist2,
                   InterpolationWeightType * weight) override;

  /** Visitor interface */
  itkCellVisitMacro(CellGeometryEnum::QUADRILATERAL_CELL);

  /** Constructor and destructor */
  /** @ITKStartGrouping */
  QuadrilateralCell() = default;
  ~QuadrilateralCell() override = default;
  /** @ITKEndGrouping */
protected:
  /** Store the number of points needed for a quadrilateral. */
  std::array<PointIdentifier, NumberOfPoints> m_PointIds{ MakeFilled<std::array<PointIdentifier, NumberOfPoints>>(
    NumericTraits<PointIdentifier>::max()) };

  void
  InterpolationDerivs(const CoordinateType pointCoords[CellDimension], CoordinateType derivs[NumberOfDerivatives]);
  void
  InterpolationFunctions(const CoordinateType    pointCoords[CellDimension],
                         InterpolationWeightType weights[NumberOfPoints]);
  void
  EvaluateLocation(int &                     itkNotUsed(subId),
                   const PointsContainer *   points,
                   const CoordinateType      pointCoords[PointDimension],
                   CoordinateType            x[PointDimension],
                   InterpolationWeightType * weights);
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkQuadrilateralCell.hxx"
#endif

#endif

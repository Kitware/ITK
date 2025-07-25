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
#ifndef itkTetrahedronCell_hxx
#define itkTetrahedronCell_hxx
#include "vnl/algo/vnl_determinant.h"

#include <algorithm> // For copy_n.

namespace itk
{

template <typename TCellInterface>
void
TetrahedronCell<TCellInterface>::MakeCopy(CellAutoPointer & cellPointer) const
{
  cellPointer.TakeOwnership(new Self);
  cellPointer->SetPointIds(this->GetPointIds());
}

template <typename TCellInterface>
unsigned int
TetrahedronCell<TCellInterface>::GetDimension() const
{
  return Self::CellDimension;
}

template <typename TCellInterface>
unsigned int
TetrahedronCell<TCellInterface>::GetNumberOfPoints() const
{
  return Self::NumberOfPoints;
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::GetNumberOfBoundaryFeatures(int dimension) const -> CellFeatureCount
{
  switch (dimension)
  {
    case 0:
      return GetNumberOfVertices();
    case 1:
      return GetNumberOfEdges();
    case 2:
      return GetNumberOfFaces();
    default:
      return 0;
  }
}

template <typename TCellInterface>
bool
TetrahedronCell<TCellInterface>::EvaluatePosition(CoordinateType *          x,
                                                  PointsContainer *         points,
                                                  CoordinateType *          closestPoint,
                                                  CoordinateType            pcoord[],
                                                  double *                  minDist2,
                                                  InterpolationWeightType * weights)
{

  CoordinateType pcoords[3] = { 0.0, 0.0, 0.0 };

  if (!points)
  {
    return false;
  }

  PointType pt1 = points->GetElement(m_PointIds[0]);
  PointType pt2 = points->GetElement(m_PointIds[1]);
  PointType pt3 = points->GetElement(m_PointIds[2]);
  PointType pt4 = points->GetElement(m_PointIds[3]);

  double rhs[PointDimension];
  double c1[PointDimension];
  double c2[PointDimension];
  double c3[PointDimension];
  for (unsigned int i = 0; i < PointDimension; ++i)
  {
    rhs[i] = x[i] - pt4[i];
    c1[i] = pt1[i] - pt4[i];
    c2[i] = pt2[i] - pt4[i];
    c3[i] = pt3[i] - pt4[i];
  }

  // Create a vnl_matrix so that the determinant can be computed
  // for any PointDimension
  vnl_matrix_fixed<CoordinateType, 3, PointDimension> mat;
  for (unsigned int i = 0; i < PointDimension; ++i)
  {
    mat.put(0, i, c1[i]);
    mat.put(1, i, c2[i]);
    mat.put(2, i, c3[i]);
  }
  const double det = vnl_determinant(mat);
  if (det == 0.0)
  {
    return false;
  }

  for (unsigned int i = 0; i < PointDimension; ++i)
  {
    mat.put(0, i, rhs[i]);
    mat.put(1, i, c2[i]);
    mat.put(2, i, c3[i]);
  }

  pcoords[0] = vnl_determinant(mat) / det;

  for (unsigned int i = 0; i < PointDimension; ++i)
  {
    mat.put(0, i, c1[i]);
    mat.put(1, i, rhs[i]);
    mat.put(2, i, c3[i]);
  }

  pcoords[1] = vnl_determinant(mat) / det;

  for (unsigned int i = 0; i < PointDimension; ++i)
  {
    mat.put(0, i, c1[i]);
    mat.put(1, i, c2[i]);
    mat.put(2, i, rhs[i]);
  }

  pcoords[2] = vnl_determinant(mat) / det;

  const double p4 = 1.0 - pcoords[0] - pcoords[1] - pcoords[2];

  if (weights)
  {
    weights[0] = p4;
    weights[1] = pcoords[0];
    weights[2] = pcoords[1];
    weights[3] = pcoords[2];
  }

  if (pcoord)
  {
    pcoord[0] = pcoords[0];
    pcoord[1] = pcoords[1];
    pcoord[2] = pcoords[2];
  }

  if (pcoords[0] >= -0.001 && pcoords[0] <= 1.001 && pcoords[1] >= -0.001 && pcoords[1] <= 1.001 &&
      pcoords[2] >= -0.001 && pcoords[2] <= 1.001 && p4 >= -0.001 && p4 <= 1.001)
  {
    if (closestPoint)
    {
      for (unsigned int ii = 0; ii < PointDimension; ++ii)
      {
        closestPoint[ii] = x[ii];
      }
      if (minDist2)
      {
        *minDist2 = 0.0; // inside tetra
      }
    }
    return true;
  }
  else
  { // could easily be sped up using parametric localization - next release
    CoordinateType closest[PointDimension];
    CoordinateType pc[3];

    if (closestPoint)
    {
      FaceAutoPointer triangle;
      *minDist2 = NumericTraits<double>::max();
      for (int i = 0; i < 4; ++i)
      {
        this->GetFace(i, triangle);
        double dist2 = NAN;
        triangle->EvaluatePosition(x, points, closest, pc, &dist2, nullptr);

        if (dist2 < *minDist2)
        {
          for (unsigned int dim = 0; dim < PointDimension; ++dim)
          {
            closestPoint[dim] = closest[dim];
          }
          *minDist2 = dist2;
        }
      }
    }
    // Just fall through to default return false;
  }
  return false;
}

template <typename TCellInterface>
bool
TetrahedronCell<TCellInterface>::GetBoundaryFeature(int                   dimension,
                                                    CellFeatureIdentifier featureId,
                                                    CellAutoPointer &     cellPointer)
{
  switch (dimension)
  {
    case 0:
    {
      VertexAutoPointer vertexPointer;
      if (this->GetVertex(featureId, vertexPointer))
      {
        TransferAutoPointer(cellPointer, vertexPointer);
        return true;
      }
      break;
    }
    case 1:
    {
      EdgeAutoPointer edgePointer;
      if (this->GetEdge(featureId, edgePointer))
      {
        TransferAutoPointer(cellPointer, edgePointer);
        return true;
      }
      break;
    }
    case 2:
    {
      FaceAutoPointer facePointer;
      if (this->GetFace(featureId, facePointer))
      {
        TransferAutoPointer(cellPointer, facePointer);
        return true;
      }
      break;
    }
    default:
      break; // just fall through
  }
  cellPointer.Reset();
  return false;
}

template <typename TCellInterface>
void
TetrahedronCell<TCellInterface>::SetPointIds(PointIdConstIterator first)
{
  std::copy_n(first, Self::NumberOfPoints, m_PointIds.begin());
}

template <typename TCellInterface>
void
TetrahedronCell<TCellInterface>::SetPointIds(PointIdConstIterator first, PointIdConstIterator last)
{
  int                  localId = 0;
  PointIdConstIterator ii(first);

  while (ii != last)
  {
    m_PointIds[localId++] = *ii++;
  }
}

template <typename TCellInterface>
void
TetrahedronCell<TCellInterface>::SetPointId(int localId, PointIdentifier ptId)
{
  m_PointIds[localId] = ptId;
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::PointIdsBegin() -> PointIdIterator
{
  return &m_PointIds[0];
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::PointIdsBegin() const -> PointIdConstIterator
{
  return &m_PointIds[0];
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::PointIdsEnd() -> PointIdIterator
{
  return &m_PointIds[Self::NumberOfPoints - 1] + 1;
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::PointIdsEnd() const -> PointIdConstIterator
{
  return &m_PointIds[Self::NumberOfPoints - 1] + 1;
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::GetNumberOfVertices() const -> CellFeatureCount
{
  return Self::NumberOfVertices;
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::GetNumberOfEdges() const -> CellFeatureCount
{
  return Self::NumberOfEdges;
}

template <typename TCellInterface>
auto
TetrahedronCell<TCellInterface>::GetNumberOfFaces() const -> CellFeatureCount
{
  return Self::NumberOfFaces;
}

template <typename TCellInterface>
bool
TetrahedronCell<TCellInterface>::GetVertex(CellFeatureIdentifier vertexId, VertexAutoPointer & vertexPointer)
{
  auto * vert = new VertexType;

  vert->SetPointId(0, m_PointIds[vertexId]);
  vertexPointer.TakeOwnership(vert);
  return true;
}

template <typename TCellInterface>
bool
TetrahedronCell<TCellInterface>::GetEdge(CellFeatureIdentifier edgeId, EdgeAutoPointer & edgePointer)
{
  auto * edge = new EdgeType;

  for (unsigned int i = 0; i < EdgeType::NumberOfPoints; ++i)
  {
    edge->SetPointId(i, m_PointIds[m_Edges[edgeId][i]]);
  }
  edgePointer.TakeOwnership(edge);
  return true;
}

template <typename TCellInterface>
bool
TetrahedronCell<TCellInterface>::GetFace(CellFeatureIdentifier faceId, FaceAutoPointer & facePointer)
{
  auto * face = new FaceType;

  for (unsigned int i = 0; i < FaceType::NumberOfPoints; ++i)
  {
    face->SetPointId(i, m_PointIds[m_Faces[faceId][i]]);
  }
  facePointer.TakeOwnership(face);
  return true;
}
} // end namespace itk

#endif

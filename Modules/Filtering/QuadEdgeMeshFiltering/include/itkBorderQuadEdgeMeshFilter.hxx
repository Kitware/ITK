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
#ifndef itkBorderQuadEdgeMeshFilter_hxx
#define itkBorderQuadEdgeMeshFilter_hxx


namespace itk
{
// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::BorderQuadEdgeMeshFilter()
  : m_TransformType(BorderTransformEnum::SQUARE_BORDER_TRANSFORM)
  , m_BorderPick(BorderPickEnum::LONGEST)
  , m_Radius(0.0)
{}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
auto
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::GetBoundaryPtMap() -> MapPointIdentifier
{
  return this->m_BoundaryPtMap;
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
auto
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::GetBorder() -> InputVectorPointType
{
  return this->m_Border;
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
void
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::ComputeBoundary()
{
  InputQEType * bdryEdge = nullptr;

  switch (m_BorderPick)
  {
    case BorderPickEnum::LONGEST:
      bdryEdge = ComputeLongestBorder();
      break;
    case BorderPickEnum::LARGEST:
      bdryEdge = ComputeLargestBorder();
      break;
    default:
      itkWarningMacro("Unknown Border to be picked...");
      break;
  }

  InputPointIdentifier    i = 0;
  InputIteratorGeom       it = bdryEdge->BeginGeomLnext();
  const InputIteratorGeom end = bdryEdge->EndGeomLnext();

  while (it != end)
  {
    this->m_BoundaryPtMap[it.Value()->GetOrigin()] = i;
    ++it;
    ++i;
  }

  this->m_Border.resize(i);
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
void
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::GenerateData()
{
  this->ComputeTransform();
}

// ----------------------------------------------------------------------------
// *** under testing ***
#if !defined(ITK_WRAPPING_PARSER)
template <typename TInputMesh, typename TOutputMesh>
auto
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::ComputeLongestBorder() -> InputQEType *
{
  const BoundaryRepresentativeEdgesPointer boundaryRepresentativeEdges = BoundaryRepresentativeEdgesType::New();

  const InputMeshConstPointer input = this->GetInput();

  InputEdgeListPointerType list;
  list.TakeOwnership(boundaryRepresentativeEdges->Evaluate(*input));

  if (!list || list->empty())
  {
    itkGenericExceptionMacro("This filter requires at least one boundary");
  }

  InputCoordinateType max_length(0.0);
  auto                oborder_it = list->begin();
  for (auto b_it = list->begin(); b_it != list->end(); ++b_it)
  {
    InputCoordinateType length(0.0);

    for (InputIteratorGeom e_it = (*b_it)->BeginGeomLnext(); e_it != (*b_it)->EndGeomLnext(); ++e_it)
    {
      InputQEType * t_edge = e_it.Value();

      const InputPointIdentifier id_org = t_edge->GetOrigin();
      const InputPointIdentifier id_dest = t_edge->GetDestination();

      const InputPointType org = input->GetPoint(id_org);
      const InputPointType dest = input->GetPoint(id_dest);

      length += org.EuclideanDistanceTo(dest);
    }
    if (length > max_length)
    {
      max_length = length;
      oborder_it = b_it;
    }
  }

  InputQEType * output = *oborder_it;

  return output;
}
#endif

// ----------------------------------------------------------------------------
#if !defined(ITK_WRAPPING_PARSER)
template <typename TInputMesh, typename TOutputMesh>
auto
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::ComputeLargestBorder() -> InputQEType *
{
  const BoundaryRepresentativeEdgesPointer boundaryRepresentativeEdges = BoundaryRepresentativeEdgesType::New();

  const InputMeshConstPointer input = this->GetInput();

  InputEdgeListPointerType list;
  list.TakeOwnership(boundaryRepresentativeEdges->Evaluate(*input));

  if (!list || list->empty())
  {
    itkGenericExceptionMacro("This filter requires at least one boundary");
  }

  SizeValueType max_id = 0L;
  SizeValueType k = 0L;

  auto oborder_it = list->begin();

  for (auto b_it = list->begin(); b_it != list->end(); ++b_it)
  {
    k = 0;

    for (InputIteratorGeom e_it = (*b_it)->BeginGeomLnext(); e_it != (*b_it)->EndGeomLnext(); ++e_it)
    {
      ++k;
    }

    if (k > max_id)
    {
      max_id = k;
      oborder_it = b_it;
    }
  }

  InputQEType * output = *oborder_it;

  return output;
}
#endif

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
void
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::DiskTransform()
{
  const InputMeshConstPointer input = this->GetInput();

  auto NbBoundaryPt = static_cast<InputPointIdentifier>(this->m_BoundaryPtMap.size());

  const InputCoordinateType r = this->RadiusMaxSquare();

  const InputCoordinateType two_r = 2.0 * r;
  const InputCoordinateType inv_two_r = 1.0 / two_r;

  InputPointIdentifier id = this->m_BoundaryPtMap.begin()->first;
  InputPointType       pt1 = input->GetPoint(id);

  id = (--m_BoundaryPtMap.end())->first;
  InputPointType pt2 = input->GetPoint(id);

  InputCoordinateType dist = pt1.SquaredEuclideanDistanceTo(pt2);

  std::vector<InputCoordinateType> tetas(NbBoundaryPt, 0.0);
  tetas[0] = static_cast<InputCoordinateType>(std::acos((two_r - dist) * inv_two_r));

  auto BoundaryPtIterator = this->m_BoundaryPtMap.begin();

  ++BoundaryPtIterator;

  OutputPointIdentifier j = 1;

  while (BoundaryPtIterator != this->m_BoundaryPtMap.end())
  {
    pt1 = pt2;

    id = BoundaryPtIterator->first;
    pt2 = input->GetPoint(id);

    dist = pt1.SquaredEuclideanDistanceTo(pt2);

    tetas[j] = tetas[j - 1] + std::acos((two_r - dist) * inv_two_r);

    ++j;
    ++BoundaryPtIterator;
  }

  const InputCoordinateType a = (2.0 * itk::Math::pi) / tetas[NbBoundaryPt - 1];
  if (this->m_Radius == 0.0)
  {
    this->m_Radius = std::pow(std::sqrt(r), a);
  }

  for (auto BoundaryPtMapIterator = this->m_BoundaryPtMap.begin(); BoundaryPtMapIterator != this->m_BoundaryPtMap.end();
       ++BoundaryPtMapIterator)
  {
    id = BoundaryPtMapIterator->first;
    j = BoundaryPtMapIterator->second;

    pt1[0] = this->m_Radius * static_cast<InputCoordinateType>(std::cos(a * tetas[j]));
    pt1[1] = this->m_Radius * static_cast<InputCoordinateType>(std::sin(a * tetas[j]));
    pt1[2] = 0.0;

    this->m_Border[j] = pt1;
  }
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
auto
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::RadiusMaxSquare() -> InputCoordinateType
{
  const InputMeshConstPointer input = this->GetInput();

  const InputPointType center = this->GetMeshBarycentre();

  InputCoordinateType oRmax(0.);
  for (auto BoundaryPtIterator = this->m_BoundaryPtMap.begin(); BoundaryPtIterator != this->m_BoundaryPtMap.end();
       ++BoundaryPtIterator)
  {
    const auto r =
      static_cast<InputCoordinateType>(center.SquaredEuclideanDistanceTo(input->GetPoint(BoundaryPtIterator->first)));
    if (r > oRmax)
    {
      oRmax = r;
    }
  }

  oRmax *= 2.25;

  return oRmax;
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
auto
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::GetMeshBarycentre() -> InputPointType
{
  const InputMeshConstPointer input = this->GetInput();

  InputPointType oCenter{};

  const InputPointsContainer * points = input->GetPoints();

  InputPointsContainerConstIterator PointIterator = points->Begin();
  while (PointIterator != points->End())
  {
    InputPointType pt = PointIterator.Value();

    for (unsigned int i = 0; i < PointDimension; ++i)
    {
      oCenter[i] += pt[i];
    }
    ++PointIterator;
  }

  const InputCoordinateType invNbOfPoints = 1.0 / static_cast<InputCoordinateType>(input->GetNumberOfPoints());
  for (unsigned int i = 0; i < PointDimension; ++i)
  {
    oCenter[i] *= invNbOfPoints;
  }

  return oCenter;
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
void
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::ComputeTransform()
{
  this->ComputeBoundary();

  switch (this->m_TransformType)
  {
    default:
    case BorderTransformEnum::SQUARE_BORDER_TRANSFORM:
    {
      this->ArcLengthSquareTransform();
      break;
    }
    case BorderTransformEnum::DISK_BORDER_TRANSFORM:
    {
      this->DiskTransform();
      break;
    }
  }
}

// ----------------------------------------------------------------------------
template <typename TInputMesh, typename TOutputMesh>
void
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::ArcLengthSquareTransform()
{
  const BoundaryRepresentativeEdgesPointer boundaryRepresentativeEdges = BoundaryRepresentativeEdgesType::New();

  const InputMeshConstPointer input = this->GetInput();

  InputEdgeListPointerType list;
  list.TakeOwnership(boundaryRepresentativeEdges->Evaluate(*input));

  InputQEType * bdryEdge = *(list->begin());

  auto NbBoundaryPt = static_cast<InputPointIdentifier>(this->m_BoundaryPtMap.size());

  std::vector<InputCoordinateType> Length(NbBoundaryPt + 1, 0.0);

  InputCoordinateType TotalLength(0.0);
  {
    InputPointIdentifier i(0);
    for (InputIteratorGeom it = bdryEdge->BeginGeomLnext(); it != bdryEdge->EndGeomLnext(); ++it, ++i)
    {
      const InputPointIdentifier org = it.Value()->GetOrigin();
      const InputPointIdentifier dest = it.Value()->GetDestination();

      const InputPointType PtOrg = input->GetPoint(org);
      const InputPointType PtDest = input->GetPoint(dest);

      const InputCoordinateType distance = PtOrg.EuclideanDistanceTo(PtDest);
      TotalLength += distance;
      Length[i] = TotalLength;
    }
  }
  if (this->m_Radius == 0.0)
  {
    this->m_Radius = 1000.;
  }

  const InputCoordinateType EdgeLength = 2.0 * this->m_Radius;
  const InputCoordinateType ratio = 4.0 * EdgeLength / TotalLength;

  for (InputPointIdentifier i = 0; i < NbBoundaryPt + 1; ++i)
  {
    Length[i] *= ratio;
  }

  InputPointType pt;
  pt[0] = -this->m_Radius;
  pt[1] = this->m_Radius;
  pt[2] = 0.0;

  this->m_Border[0] = pt;

  InputPointIdentifier i = 1;
  while (Length[i] < EdgeLength)
  {
    pt[0] = -this->m_Radius + Length[i];
    this->m_Border[i++] = pt;
  }

  pt[0] = this->m_Radius;
  pt[1] = this->m_Radius;
  this->m_Border[i++] = pt;

  while (Length[i] < (2.0 * EdgeLength))
  {
    pt[1] = this->m_Radius - (Length[i] - EdgeLength);
    this->m_Border[i++] = pt;
  }

  pt[0] = this->m_Radius;
  pt[1] = -this->m_Radius;
  this->m_Border[i++] = pt;

  while (Length[i] < (3.0 * EdgeLength))
  {
    pt[0] = this->m_Radius - (Length[i] - 2.0 * EdgeLength);
    this->m_Border[i++] = pt;
  }

  pt[0] = -this->m_Radius;
  pt[1] = -this->m_Radius;
  this->m_Border[i++] = pt;

  while (i < NbBoundaryPt)
  {
    pt[1] = -this->m_Radius + (Length[i] - 3.0 * EdgeLength);
    this->m_Border[i++] = pt;
  }
}

template <typename TInputMesh, typename TOutputMesh>
void
BorderQuadEdgeMeshFilter<TInputMesh, TOutputMesh>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "TransformType: " << static_cast<char>(m_TransformType) << std::endl;
  os << indent << "BorderPick: " << static_cast<char>(m_BorderPick) << std::endl;
  os << indent << "Radius: " << m_Radius << std::endl;
}
} // namespace itk

#endif

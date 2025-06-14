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
#ifndef itkSpatialObject_hxx
#define itkSpatialObject_hxx

#include "itkNumericTraits.h"
#include <algorithm>
#include <string>
#include "itkMath.h"
#include "itkImageBase.h"

namespace itk
{

template <unsigned int TDimension>
SpatialObject<TDimension>::~SpatialObject()
{
  this->RemoveAllChildren(0);
}


template <unsigned int TDimension>
void
SpatialObject<TDimension>::Clear()
{
  const typename BoundingBoxType::PointType pnt{};
  m_FamilyBoundingBoxInObjectSpace->SetMinimum(pnt);
  m_FamilyBoundingBoxInObjectSpace->SetMaximum(pnt);
  m_FamilyBoundingBoxInWorldSpace->SetMinimum(pnt);
  m_FamilyBoundingBoxInWorldSpace->SetMaximum(pnt);

  m_MyBoundingBoxInObjectSpace->SetMinimum(pnt);
  m_MyBoundingBoxInObjectSpace->SetMaximum(pnt);
  m_MyBoundingBoxInWorldSpace->SetMinimum(pnt);
  m_MyBoundingBoxInWorldSpace->SetMaximum(pnt);

  m_ObjectToParentTransform->SetIdentity();
  m_ObjectToParentTransformInverse->SetIdentity();

  ProtectedComputeObjectToWorldTransform();

  m_DefaultInsideValue = 1.0;
  m_DefaultOutsideValue = 0.0;

  m_Property.Clear();

  this->Modified();
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetId(int id)
{
  if (id != m_Id)
  {
    m_Id = id;
    for (const auto & child : m_ChildrenList)
    {
      child->SetParentId(id);
    }
    this->Modified();
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::DerivativeAtInObjectSpace(const PointType &            point,
                                                     short unsigned int           order,
                                                     DerivativeVectorType &       value,
                                                     unsigned int                 depth,
                                                     const std::string &          name,
                                                     const DerivativeOffsetType & offset)
{
  if (!IsEvaluableAtInObjectSpace(point, depth, name))
  {
    itkExceptionMacro("This spatial object is not evaluable at the point");
  }

  if (order == 0)
  {
    double r = NAN;

    ValueAtInObjectSpace(point, r, depth, name);
    value.Fill(r);
  }
  else
  {
    typename DerivativeVectorType::Iterator it = value.Begin();
    DerivativeVectorType                    v1;
    auto                                    it_v1 = v1.cbegin();
    DerivativeVectorType                    v2;
    auto                                    it_v2 = v2.cbegin();

    DerivativeOffsetType offsetDiv2;
    for (unsigned short i = 0; i < TDimension; ++i)
    {
      offsetDiv2[i] = offset[i] / 2.0;
    }
    for (unsigned short i = 0; i < TDimension; i++, it++, it_v1++, it_v2++)
    {
      PointType p1 = point;
      PointType p2 = point;

      p1[i] -= offset[i];
      p2[i] += offset[i];

      // note DerivativeAtInObjectSpace might throw.
      DerivativeAtInObjectSpace(p1, order - 1, v1, depth, name, offsetDiv2);
      DerivativeAtInObjectSpace(p2, order - 1, v2, depth, name, offsetDiv2);

      (*it) = ((*it_v2) - (*it_v1)) / 2;
    }
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::DerivativeAtInWorldSpace(const PointType &            point,
                                                    short unsigned int           order,
                                                    DerivativeVectorType &       value,
                                                    unsigned int                 depth,
                                                    const std::string &          name,
                                                    const DerivativeOffsetType & offset)
{
  const PointType pnt = m_ObjectToWorldTransformInverse->TransformType::TransformPoint(point);
  this->DerivativeAtInObjectSpace(pnt, order, value, depth, name, offset);
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsInsideInObjectSpace(const PointType &   point,
                                                 unsigned int        depth,
                                                 const std::string & name) const
{
  if (name.empty() || (this->GetTypeName().find(name) != std::string::npos))
  {
    if (this->IsInsideInObjectSpace(point))
    {
      return true;
    }
  }

  if (depth > 0)
  {
    return Self::IsInsideChildrenInObjectSpace(point, depth - 1, name);
  }

  return false;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsInsideInObjectSpace(const PointType & itkNotUsed(point)) const
{
  // This overload is virtual, and should be overridden.
  return false;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsInsideInWorldSpace(const PointType &   point,
                                                unsigned int        depth,
                                                const std::string & name) const
{
  const PointType pnt = m_ObjectToWorldTransformInverse->TransformType::TransformPoint(point);
  return IsInsideInObjectSpace(pnt, depth, name);
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsInsideInWorldSpace(const PointType & point) const
{
  const PointType pnt = m_ObjectToWorldTransformInverse->TransformType::TransformPoint(point);
  return IsInsideInObjectSpace(pnt);
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsInsideChildrenInObjectSpace(const PointType &   point,
                                                         unsigned int        depth,
                                                         const std::string & name) const
{
  for (const auto & child : m_ChildrenList)
  {
    const PointType pnt = child->GetObjectToParentTransformInverse()->TransformPoint(point);
    if (child->IsInsideInObjectSpace(pnt, depth, name))
    {
      return true;
    }
  }

  return false;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsEvaluableAtInObjectSpace(const PointType &   point,
                                                      unsigned int        depth,
                                                      const std::string & name) const
{
  if (IsInsideInObjectSpace(point, 0, name))
  {
    return true;
  }

  if (depth > 0)
  {
    return IsEvaluableAtChildrenInObjectSpace(point, depth - 1, name);
  }
  else
  {
    return false;
  }
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsEvaluableAtInWorldSpace(const PointType &   point,
                                                     unsigned int        depth,
                                                     const std::string & name) const
{
  const PointType pnt = m_ObjectToWorldTransformInverse->TransformType::TransformPoint(point);
  return this->IsEvaluableAtInObjectSpace(pnt, depth, name);
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::IsEvaluableAtChildrenInObjectSpace(const PointType &   point,
                                                              unsigned int        depth,
                                                              const std::string & name) const
{
  for (const auto & child : m_ChildrenList)
  {
    const PointType pnt = child->GetObjectToParentTransformInverse()->TransformPoint(point);
    if (child->IsEvaluableAtInObjectSpace(pnt, depth, name))
    {
      return true;
    }
  }

  return false;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::ValueAtInObjectSpace(const PointType &   point,
                                                double &            value,
                                                unsigned int        depth,
                                                const std::string & name) const
{
  if (IsEvaluableAtInObjectSpace(point, 0, name))
  {
    if (IsInsideInObjectSpace(point, 0, name))
    {
      value = m_DefaultInsideValue;
      return true;
    }

    value = m_DefaultOutsideValue;
    return true;
  }
  else
  {
    if (depth > 0)
    {
      if (ValueAtChildrenInObjectSpace(point, value, depth - 1, name))
      {
        return true;
      }
    }
  }

  value = m_DefaultOutsideValue;
  return false;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::ValueAtInWorldSpace(const PointType &   point,
                                               double &            value,
                                               unsigned int        depth,
                                               const std::string & name) const
{
  const PointType pnt = m_ObjectToWorldTransformInverse->TransformType::TransformPoint(point);
  return this->ValueAtInObjectSpace(pnt, value, depth, name);
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::ValueAtChildrenInObjectSpace(const PointType &   point,
                                                        double &            value,
                                                        unsigned int        depth,
                                                        const std::string & name) const
{
  for (const auto & child : m_ChildrenList)
  {
    const PointType pnt = child->GetObjectToParentTransformInverse()->TransformPoint(point);
    if (child->IsEvaluableAtInObjectSpace(pnt, depth, name))
    {
      child->ValueAtInObjectSpace(pnt, value, depth, name);
      return true;
    }
  }

  value = m_DefaultOutsideValue;
  return false;
}

template <unsigned int TDimension>
typename LightObject::Pointer
SpatialObject<TDimension>::InternalClone() const
{
  typename LightObject::Pointer loPtr = CreateAnother();

  const typename Self::Pointer rval = dynamic_cast<Self *>(loPtr.GetPointer());
  if (rval.IsNull())
  {
    itkExceptionMacro("downcast to type " << this->GetNameOfClass() << " failed.");
  }

  rval->SetTypeName(this->GetTypeName());
  rval->SetId(this->GetId());
  rval->SetParentId(this->GetParentId());
  rval->SetObjectToParentTransform(this->GetObjectToParentTransform());
  rval->SetProperty(this->GetProperty());
  rval->SetDefaultInsideValue(this->GetDefaultInsideValue());
  rval->SetDefaultOutsideValue(this->GetDefaultOutsideValue());

  return loPtr;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Id: " << m_Id << std::endl;
  os << indent << "TypeName: " << m_TypeName << std::endl;
  os << indent << "ParentId: " << m_ParentId << std::endl;
  os << indent << "Parent: " << m_Parent << std::endl;
  os << indent << "LargestPossibleRegion: " << m_LargestPossibleRegion << std::endl;
  os << indent << "RequestedRegion: " << m_RequestedRegion << std::endl;
  os << indent << "BufferedRegion: " << m_BufferedRegion << std::endl;

  itkPrintSelfObjectMacro(MyBoundingBoxInObjectSpace);
  itkPrintSelfObjectMacro(MyBoundingBoxInWorldSpace);
  itkPrintSelfObjectMacro(FamilyBoundingBoxInObjectSpace);
  itkPrintSelfObjectMacro(FamilyBoundingBoxInWorldSpace);

  itkPrintSelfObjectMacro(ObjectToWorldTransform);
  itkPrintSelfObjectMacro(ObjectToWorldTransformInverse);
  itkPrintSelfObjectMacro(ObjectToParentTransform);
  itkPrintSelfObjectMacro(ObjectToParentTransformInverse);

  os << indent << "Property: ";
  m_Property.Print(os);

  os << indent << "ChildrenList: " << std::endl;
  for (const auto & elem : m_ChildrenList)
  {
    os << indent.GetNextIndent() << "[" << &elem - &*(m_ChildrenList.begin()) << "]: " << *elem << std::endl;
  }

  os << indent << "DefaultInsideValue: " << m_DefaultInsideValue << std::endl;
  os << indent << "DefaultOutsideValue: " << m_DefaultOutsideValue << std::endl;
}

template <unsigned int TDimension>
auto
SpatialObject<TDimension>::GetFamilyBoundingBoxInWorldSpace() const -> const BoundingBoxType *
{
  // Next Transform the corners of the bounding box
  using PointsContainer = typename BoundingBoxType::PointsContainer;
  const auto corners = m_FamilyBoundingBoxInObjectSpace->ComputeCorners();
  auto       transformedCorners = PointsContainer::New();
  transformedCorners->Reserve(static_cast<typename PointsContainer::ElementIdentifier>(corners.size()));

  std::transform(corners.cbegin(), corners.cend(), transformedCorners->begin(), [this](const auto & point) {
    return m_ObjectToWorldTransform->TransformPoint(point);
  });

  m_FamilyBoundingBoxInWorldSpace->SetPoints(transformedCorners);
  m_FamilyBoundingBoxInWorldSpace->ComputeBoundingBox();

  return m_FamilyBoundingBoxInWorldSpace;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::AddChild(Self * pointer)
{
  const auto pos = std::find(m_ChildrenList.begin(), m_ChildrenList.end(), pointer);
  if (pos == m_ChildrenList.end())
  {
    m_ChildrenList.push_back(pointer);

    if (pointer->GetId() == -1)
    {
      pointer->SetId(this->GetNextAvailableId());
    }

    pointer->SetParent(this);

    this->Modified();
  }
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::RemoveChild(Self * pointer)
{
  const auto pos = std::find(m_ChildrenList.begin(), m_ChildrenList.end(), pointer);
  if (pos != m_ChildrenList.end())
  {
    m_ChildrenList.erase(pos);

    if (pointer->GetParent() == this && pointer->GetParentId() == this->GetId())
    {
      pointer->SetParent(nullptr);
    }

    this->Modified();

    return true;
  }

  return false;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::RemoveAllChildren(unsigned int depth)
{
  auto it = m_ChildrenList.begin();
  while (it != m_ChildrenList.end())
  {
    auto itPtr = *it;
    it = m_ChildrenList.erase(it);
    itPtr->SetParent(nullptr);
    if (depth > 0)
    {
      itPtr->RemoveAllChildren(depth - 1);
    }
  }

  this->Modified();
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetObjectToParentTransform(const TransformType * transform)
{
  if (!transform->GetInverse(m_ObjectToParentTransformInverse))
  {
    itkExceptionMacro("Transform must be invertible.");
  }

  m_ObjectToParentTransform->SetFixedParameters(transform->GetFixedParameters());
  m_ObjectToParentTransform->SetParameters(transform->GetParameters());

  ProtectedComputeObjectToWorldTransform();
}

template <unsigned int TDimension>
auto
SpatialObject<TDimension>::GetObjectToParentTransformInverse() const -> const TransformType *
{
  if (m_ObjectToParentTransform->GetMTime() > m_ObjectToParentTransformInverse->GetMTime())
  {
    m_ObjectToParentTransform->GetInverse(m_ObjectToParentTransformInverse);
  }
  return m_ObjectToParentTransformInverse.GetPointer();
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::ProtectedComputeObjectToWorldTransform()
{
  m_ObjectToWorldTransform->SetFixedParameters(this->GetObjectToParentTransform()->GetFixedParameters());
  m_ObjectToWorldTransform->SetParameters(this->GetObjectToParentTransform()->GetParameters());
  if (this->HasParent())
  {
    m_ObjectToWorldTransform->Compose(this->GetParent()->m_ObjectToWorldTransform, false);
  }

  if (!m_ObjectToWorldTransform->GetInverse(m_ObjectToWorldTransformInverse))
  {
    itkExceptionMacro("Transform must be invertible.");
  }

  // Propagate the changes to the children
  for (const auto & child : m_ChildrenList)
  {
    child->Update();
  }

  this->Modified();
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetObjectToWorldTransform(const TransformType * transform)
{
  if (!transform->GetInverse(m_ObjectToWorldTransformInverse))
  {
    itkExceptionMacro("Transform must be invertible.");
  }

  m_ObjectToWorldTransform->SetFixedParameters(transform->GetFixedParameters());
  m_ObjectToWorldTransform->SetParameters(transform->GetParameters());

  ComputeObjectToParentTransform();
  ProtectedComputeObjectToWorldTransform();
}

template <unsigned int TDimension>
auto
SpatialObject<TDimension>::GetObjectToWorldTransformInverse() const -> const TransformType *
{
  if (m_ObjectToWorldTransform->GetMTime() > m_ObjectToWorldTransformInverse->GetMTime())
  {
    m_ObjectToWorldTransform->GetInverse(m_ObjectToWorldTransformInverse);
  }
  return m_ObjectToWorldTransformInverse.GetPointer();
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::ComputeObjectToParentTransform()
{
  m_ObjectToParentTransform->SetFixedParameters(m_ObjectToWorldTransform->GetFixedParameters());
  m_ObjectToParentTransform->SetParameters(m_ObjectToWorldTransform->GetParameters());

  if (this->HasParent())
  {
    auto inverse = TransformType::New();
    if (this->GetParent()->m_ObjectToWorldTransform->GetInverse(inverse))
    {
      m_ObjectToParentTransform->Compose(inverse, true);
    }
    else
    {
      itkExceptionMacro("Parent's ObjectToWorldTransform not invertible.");
    }
  }

  if (!m_ObjectToParentTransform->GetInverse(m_ObjectToParentTransformInverse))
  {
    itkExceptionMacro("ObjectToParentTransform not invertible.");
  }
  ProtectedComputeObjectToWorldTransform();
}

template <unsigned int TDimension>
ModifiedTimeType
SpatialObject<TDimension>::GetMTime() const
{
  ModifiedTimeType latestTime = Object::GetMTime();

  for (const auto & child : m_ChildrenList)
  {
    const ModifiedTimeType localTime = child->GetMTime();

    if (localTime > latestTime)
    {
      latestTime = localTime;
    }
  }

  return latestTime;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::ComputeMyBoundingBox()
{
  const typename BoundingBoxType::PointType pnt{};
  if (m_MyBoundingBoxInObjectSpace->GetMinimum() != pnt || m_MyBoundingBoxInObjectSpace->GetMaximum() != pnt)
  {
    m_MyBoundingBoxInObjectSpace->SetMinimum(pnt);
    m_MyBoundingBoxInObjectSpace->SetMaximum(pnt);

    this->Modified();
  }
}

template <unsigned int TDimension>
auto
SpatialObject<TDimension>::GetMyBoundingBoxInWorldSpace() const -> const BoundingBoxType *
{
  // Next Transform the corners of the bounding box
  using PointsContainer = typename BoundingBoxType::PointsContainer;
  const auto corners = m_MyBoundingBoxInObjectSpace->ComputeCorners();
  auto       transformedCorners = PointsContainer::New();
  transformedCorners->Reserve(static_cast<typename PointsContainer::ElementIdentifier>(corners.size()));

  std::transform(corners.cbegin(), corners.cend(), transformedCorners->begin(), [this](const auto & point) {
    return m_ObjectToWorldTransform->TransformPoint(point);
  });

  m_MyBoundingBoxInWorldSpace->SetPoints(transformedCorners);
  m_MyBoundingBoxInWorldSpace->ComputeBoundingBox();

  return m_MyBoundingBoxInWorldSpace;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::ComputeFamilyBoundingBox(unsigned int depth, const std::string & name) const
{
  itkDebugMacro("Computing Bounding Box");

  const typename BoundingBoxType::PointType zeroPnt{};
  m_FamilyBoundingBoxInObjectSpace->SetMinimum(zeroPnt);
  m_FamilyBoundingBoxInObjectSpace->SetMaximum(zeroPnt);
  bool bbDefined = false;

  if (this->GetTypeName().find(name) != std::string::npos)
  {
    PointType pointMin = m_MyBoundingBoxInObjectSpace->GetMinimum();
    PointType pointMax = m_MyBoundingBoxInObjectSpace->GetMaximum();
    for (unsigned int i = 0; i < ObjectDimension; ++i)
    {
      if (Math::NotExactlyEquals(pointMin[i], 0) || Math::NotExactlyEquals(pointMax[i], 0))
      {
        bbDefined = true;
        m_FamilyBoundingBoxInObjectSpace->SetMinimum(pointMin);
        m_FamilyBoundingBoxInObjectSpace->SetMaximum(pointMax);
        break;
      }
    }
  }

  if (depth > 0)
  {
    PointType pnt;
    PointType tPnt;
    for (const auto & child : m_ChildrenList)
    {
      child->ComputeFamilyBoundingBox(depth - 1, name);

      if (!bbDefined)
      {
        pnt = child->GetFamilyBoundingBoxInObjectSpace()->GetMinimum();
        tPnt = child->GetObjectToParentTransform()->TransformPoint(pnt);
        m_FamilyBoundingBoxInObjectSpace->SetMinimum(tPnt);
        pnt = child->GetFamilyBoundingBoxInObjectSpace()->GetMaximum();
        tPnt = child->GetObjectToParentTransform()->TransformPoint(pnt);
        m_FamilyBoundingBoxInObjectSpace->SetMaximum(tPnt);
        bbDefined = true;
      }
      else
      {
        pnt = child->GetFamilyBoundingBoxInObjectSpace()->GetMinimum();
        tPnt = child->GetObjectToParentTransform()->TransformPoint(pnt);
        m_FamilyBoundingBoxInObjectSpace->ConsiderPoint(tPnt);
        pnt = child->GetFamilyBoundingBoxInObjectSpace()->GetMaximum();
        tPnt = child->GetObjectToParentTransform()->TransformPoint(pnt);
        m_FamilyBoundingBoxInObjectSpace->ConsiderPoint(tPnt);
      }
    }
  }

  return bbDefined;
}

template <unsigned int TDimension>
auto
SpatialObject<TDimension>::GetChildren(unsigned int depth, const std::string & name) const -> ChildrenListType *
{
  auto * childrenSO = new ChildrenListType;

  for (const auto & child : m_ChildrenList)
  {
    if (child->GetTypeName().find(name) != std::string::npos)
    {
      childrenSO->push_back(child);
    }
  }

  if (depth > 0)
  {
    for (const auto & child : m_ChildrenList)
    {
      child->AddChildrenToList(childrenSO, depth - 1, name);
    }
  }

  return childrenSO;
}

template <unsigned int TDimension>
auto
SpatialObject<TDimension>::GetConstChildren(unsigned int depth, const std::string & name) const
  -> ChildrenConstListType *
{
  auto * childrenSO = new ChildrenConstListType;

  for (const auto & child : m_ChildrenList)
  {
    if (child->GetTypeName().find(name) != std::string::npos)
    {
      childrenSO->push_back(child);
    }
  }

  if (depth > 0)
  {
    for (const auto & child : m_ChildrenList)
    {
      child->AddChildrenToConstList(childrenSO, depth - 1, name);
    }
  }

  return childrenSO;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::AddChildrenToList(ChildrenListType *  childrenList,
                                             unsigned int        depth,
                                             const std::string & name) const
{
  for (const auto & child : m_ChildrenList)
  {
    if (child->GetTypeName().find(name) != std::string::npos)
    {
      childrenList->push_back(child);
    }
  }

  if (depth > 0)
  {
    for (const auto & child : m_ChildrenList)
    {
      child->AddChildrenToList(childrenList, depth - 1, name);
    }
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::AddChildrenToConstList(ChildrenConstListType * childrenCList,
                                                  unsigned int            depth,
                                                  const std::string &     name) const
{
  for (const auto & child : m_ChildrenList)
  {
    if (child->GetTypeName().find(name) != std::string::npos)
    {
      childrenCList->push_back(child);
    }
  }

  if (depth > 0)
  {
    for (const auto & child : m_ChildrenList)
    {
      child->AddChildrenToConstList(childrenCList, depth - 1, name);
    }
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetChildren(ChildrenListType & children)
{
  this->RemoveAllChildren(0);

  // Add children
  for (const auto & child : children)
  {
    this->AddChild(child);
  }
}

template <unsigned int TDimension>
unsigned int
SpatialObject<TDimension>::GetNumberOfChildren(unsigned int depth, const std::string & name) const
{
  unsigned int ccount = 0;
  for (const auto & child : m_ChildrenList)
  {
    if (child->GetTypeName().find(name) != std::string::npos)
    {
      ++ccount;
    }
  }

  if (depth > 0)
  {
    for (const auto & child : m_ChildrenList)
    {
      ccount += child->GetNumberOfChildren(depth - 1, name);
    }
  }

  return ccount;
}

template <unsigned int TDimension>
SpatialObject<TDimension> *
SpatialObject<TDimension>::GetObjectById(int id)
{
  if (id == this->GetId())
  {
    return this;
  }

  for (const auto & child : m_ChildrenList)
  {
    SpatialObject<TDimension> * tmp = child->GetObjectById(id);
    if (tmp != nullptr)
    {
      return tmp;
    }
  }

  return nullptr;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::FixParentChildHierarchyUsingParentIds()
{
  ChildrenListType * children = this->GetChildren(MaximumDepth);

  auto it = children->begin();
  auto itEnd = children->end();

  bool ret = true;
  while (it != itEnd)
  {
    const int parentId = (*it)->GetParentId();
    if (parentId >= 0)
    {
      SpatialObject<TDimension> * parentObject = this->GetObjectById(parentId);
      if (parentObject == nullptr)
      {
        ret = false;
      }
      else
      {
        parentObject->AddChild(*it);
      }
    }
    ++it;
  }

  delete children;

  return ret;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::CheckIdValidity() const
{
  if (this->GetId() == -1)
  {
    return false;
  }

  ChildrenListType * children = this->GetChildren();

  typename ObjectListType::iterator       it = children->begin();
  const typename ObjectListType::iterator itEnd = children->end();
  typename ObjectListType::iterator       it2;

  while (it != itEnd)
  {
    const int id = (*it)->GetId();
    it2 = ++it;
    while (it2 != itEnd)
    {
      const int id2 = (*it2)->GetId();
      if (id == id2 || id2 == -1)
      {
        delete children;
        return false;
      }
      ++it2;
    }
  }

  delete children;
  return true;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::FixIdValidity()
{
  if (this->GetId() == -1)
  {
    this->SetId(this->GetNextAvailableId());
  }

  ChildrenListType * children = this->GetChildren(MaximumDepth);

  auto                              it = children->begin();
  auto                              itEnd = children->end();
  typename ObjectListType::iterator it2;

  while (it != itEnd)
  {
    const int id = (*it)->GetId();
    it2 = ++it;
    while (it2 != itEnd)
    {
      const int id2 = (*it2)->GetId();
      if (id == id2 || id2 == -1)
      {
        const int idNew = this->GetNextAvailableId();
        (*it2)->SetId(idNew);
        ChildrenListType * children2 = (*it2)->GetChildren(0);
        auto               childIt2 = children2->begin();
        auto               childIt2End = children2->end();
        while (childIt2 != childIt2End)
        {
          (*childIt2)->SetParentId(idNew);
          ++childIt2;
        }
        delete children2;
      }
      ++it2;
    }
  }

  delete children;
}

template <unsigned int TDimension>
int
SpatialObject<TDimension>::GetNextAvailableId() const
{
  int maxId = this->GetId();

  for (const auto & child : m_ChildrenList)
  {
    const int id = child->GetNextAvailableId() - 1;
    if (id > maxId)
    {
      maxId = id;
    }
  }

  return maxId + 1;
}

template <unsigned int TDimension>
SpatialObject<TDimension> *
SpatialObject<TDimension>::GetParent()
{
  return m_Parent;
}

template <unsigned int TDimension>
const SpatialObject<TDimension> *
SpatialObject<TDimension>::GetParent() const
{
  return m_Parent;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetParent(Self * parent)
{
  if (parent != m_Parent)
  {
    Self *                oldParent = m_Parent;
    const TransformType * oldObjectWorldTransform = this->m_ObjectToWorldTransform;

    m_Parent = parent;
    if (parent != nullptr)
    {
      m_ParentId = parent->GetId();

      m_Parent->AddChild(this); // Verifies hasn't been added already

      this->SetObjectToWorldTransform(oldObjectWorldTransform);
      this->ComputeObjectToParentTransform();
    }
    else
    {
      m_ParentId = -1;
      this->SetObjectToParentTransform(oldObjectWorldTransform);
      this->Update();
    }

    if (oldParent != nullptr)
    {
      oldParent->RemoveChild(this); // Verifies hasn't been removed already
    }
  }
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::HasParent() const
{
  if (m_Parent == nullptr)
  {
    return false;
  }
  return true;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetLargestPossibleRegion(const RegionType & region)
{
  if (m_LargestPossibleRegion != region)
  {
    m_LargestPossibleRegion = region;
    this->Modified();
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::UpdateOutputInformation()
{
  const auto source = this->GetSource();

  if (source)
  {
    source->UpdateOutputInformation();
  }
  // If we don't have a source, then let's make our Image
  // span our buffer
  else
  {
    m_LargestPossibleRegion = m_BufferedRegion;
  }

  // Now we should know what our largest possible region is. If our
  // requested region was not set yet, (or has been set to something
  // invalid - with no data in it ) then set it to the largest possible
  // region.
  if (m_RequestedRegion.GetNumberOfPixels() == 0)
  {
    this->SetRequestedRegionToLargestPossibleRegion();
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedRegion = m_LargestPossibleRegion;
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  const IndexType & requestedRegionIndex = m_RequestedRegion.GetIndex();
  const IndexType & bufferedRegionIndex = m_BufferedRegion.GetIndex();

  const SizeType & requestedRegionSize = m_RequestedRegion.GetSize();
  const SizeType & bufferedRegionSize = m_BufferedRegion.GetSize();

  for (unsigned int i = 0; i < ObjectDimension; ++i)
  {
    if ((requestedRegionIndex[i] < bufferedRegionIndex[i]) ||
        ((requestedRegionIndex[i] + static_cast<OffsetValueType>(requestedRegionSize[i])) >
         (bufferedRegionIndex[i] + static_cast<OffsetValueType>(bufferedRegionSize[i]))))
    {
      return true;
    }
  }
  return false;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetBufferedRegion(const RegionType & region)
{
  if (m_BufferedRegion != region)
  {
    m_BufferedRegion = region;
    this->Modified();
  }
}

template <unsigned int TDimension>
bool
SpatialObject<TDimension>::VerifyRequestedRegion()
{
  bool retval = true;

  // Is the requested region within the LargestPossibleRegion?
  // Note that the test is indeed against the largest possible region
  // rather than the buffered region; see DataObject::VerifyRequestedRegion.
  const IndexType & requestedRegionIndex = m_RequestedRegion.GetIndex();
  const IndexType & largestPossibleRegionIndex = m_LargestPossibleRegion.GetIndex();

  const SizeType & requestedRegionSize = m_RequestedRegion.GetSize();
  const SizeType & largestPossibleRegionSize = m_LargestPossibleRegion.GetSize();

  for (unsigned int i = 0; i < ObjectDimension; ++i)
  {
    if ((requestedRegionIndex[i] < largestPossibleRegionIndex[i]) ||
        ((requestedRegionIndex[i] + static_cast<OffsetValueType>(requestedRegionSize[i])) >
         (largestPossibleRegionIndex[i] + static_cast<OffsetValueType>(largestPossibleRegionSize[i]))))
    {
      retval = false;
    }
  }

  return retval;
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetRequestedRegion(const RegionType & region)
{
  if (m_RequestedRegion != region)
  {
    m_RequestedRegion = region;
    this->Modified();
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::SetRequestedRegion(const DataObject * data)
{
  const auto * soData = dynamic_cast<const SpatialObject *>(data);
  const auto * imgData = dynamic_cast<const ImageBase<TDimension> *>(data);

  if (soData != nullptr)
  {
    m_RequestedRegion = soData->GetRequestedRegion();
  }
  else if (imgData != nullptr)
  {
    m_RequestedRegion = imgData->GetRequestedRegion();
  }
  else
  {
    itkExceptionMacro("SpatialObject::SetRequestedRegion(const DataObject *) cannot cast "
                      << typeid(data).name() << " to " << typeid(SpatialObject *).name());
  }
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::Update()
{
  Superclass::Update();

  this->ComputeMyBoundingBox();

  m_FamilyBoundingBoxInObjectSpace->SetMinimum(m_MyBoundingBoxInObjectSpace->GetMinimum());
  m_FamilyBoundingBoxInObjectSpace->SetMaximum(m_MyBoundingBoxInObjectSpace->GetMaximum());

  this->ProtectedComputeObjectToWorldTransform();
}

template <unsigned int TDimension>
std::string
SpatialObject<TDimension>::GetClassNameAndDimension() const
{
  std::ostringstream n;

  n << GetNameOfClass() << '_' << TDimension;

  return n.str();
}

template <unsigned int TDimension>
void
SpatialObject<TDimension>::CopyInformation(const DataObject * data)
{
  // Standard call to the superclass' method
  Superclass::CopyInformation(data);

  // Attempt to cast data to an ImageBase
  const auto * soData = dynamic_cast<const SpatialObject<TDimension> *>(data);

  if (soData == nullptr)
  {
    // pointer could not be cast back down
    itkExceptionMacro("itk::SpatialObject::CopyInformation() cannot cast "
                      << typeid(data).name() << " to " << typeid(SpatialObject<TDimension> *).name());
  }

  // Copy the meta data for this data type
  m_LargestPossibleRegion = soData->GetLargestPossibleRegion();

  // check if we are the same type
  const auto * source = dynamic_cast<const Self *>(data);
  if (!source)
  {
    std::cerr << "CopyInformation: objects are not of the same type" << std::endl;
    return;
  }

  // copy the properties
  this->SetProperty(source->GetProperty());

  // copy the ivars
  this->SetObjectToWorldTransform(source->m_ObjectToWorldTransform);
  this->SetDefaultInsideValue(source->GetDefaultInsideValue());
  this->SetDefaultOutsideValue(source->GetDefaultOutsideValue());

  // Do not copy id, parent, or child info
  // this->SetParent( source->GetParent() );
}

} // end of namespace itk

#endif // __SpatialObject_hxx

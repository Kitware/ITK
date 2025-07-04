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
#ifndef itkWatershedSegmentTree_h
#define itkWatershedSegmentTree_h

#include "itkProcessObject.h"
#include <deque>
#include <functional>

namespace itk::watershed
{
/**
 * \class SegmentTree
 * A data structure for storing segment merge information used in filters of
 * the watershed segmentation algorithm.  See itk::WatershedImageFilter for an
 * overview.
 *
 * \par
 * This class is the implementation of the "merge tree" referred to in the
 * documentation for itk::WatershedImageFilter and other watershed segmentation
 * component classes.  It holds a list of merges among image segments at
 * various saliency levels. The list is actually a representation of a binary
 * tree, whose nodes are segments and edges are saliencies.
 *
 * \ingroup WatershedSegmentation
 * \sa itk::WatershedImageFilter
 *
 * \ingroup ITKWatersheds
 */
template <typename TScalar>
class ITK_TEMPLATE_EXPORT SegmentTree : public DataObject
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(SegmentTree);

  /** Define itk Smart Pointers for this object */
  /** @ITKStartGrouping */
  using Self = SegmentTree;
  using Superclass = DataObject;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  itkNewMacro(Self);
  itkOverrideGetNameOfClassMacro(SegmentTree);
  using ScalarType = TScalar;

  /** @ITKEndGrouping */
  /** Elements of the list (nodes of the tree).  A record of a merge
   * between two segments (IdentifierType labels) at a particular
   * saliency.   */
  struct merge_t
  {
    IdentifierType from;
    IdentifierType to;
    ScalarType     saliency;
  };

  /** Define the container type used in this list data structure */
  using DequeType = std::deque<merge_t>;
  using Iterator = typename DequeType::iterator;
  using ConstIterator = typename DequeType::const_iterator;
  using ValueType = typename DequeType::value_type;

  /** Boolean comparison functor for use in sorting functions.  */
  struct merge_comp
  {
    merge_comp() = default;
    bool
    operator()(const merge_t & a, const merge_t & b)
    {
      return b.saliency < a.saliency;
    }
  };

  /** Boolean comparison functor for use in sorting functions.   */
  struct sort_comp
  {
    bool
    operator()(const merge_t & a, const merge_t & b)
    {
      return a.saliency < b.Saliency;
    }
  };

  /** Returns the size of the list.   */
  typename DequeType::size_type
  Size() const
  {
    return m_Deque.size();
  }

  /** Returns TRUE if the SegmentTree is empty, FALSE if the SegmentTree is not
   * empty.    */
  bool
  Empty() const
  {
    return m_Deque.empty();
  }

  /** Returns a const reference to the front of the list (node with the least
   * saliency value).    */
  const merge_t &
  Front() const
  {
    return m_Deque.front();
  }

  /** Returns a const reference to the back of the list (node with the greatest
   * saliency value).   */
  const merge_t &
  Back() const
  {
    return m_Deque.back();
  }

  /** Returns a reference to the front of the list   */
  merge_t &
  Front()
  {
    return m_Deque.front();
  }

  /** Returns a reference to the back of the list   */
  merge_t &
  Back()
  {
    return m_Deque.back();
  }

  /** Inserts a node at the front of the list.   */
  void
  PushFront(const ValueType & t)
  {
    m_Deque.push_front(t);
  }

  /** Inserts a node at the back of the list   */
  void
  PushBack(const ValueType & t)
  {
    m_Deque.push_back(t);
  }

  /** Erases the node at the front of the list.   */
  void
  PopFront()
  {
    m_Deque.pop_front();
  }

  /** Erases the node at the back of the list.   */
  void
  PopBack()
  {
    m_Deque.pop_back();
  }

  /** Returns an iterator pointing to the first element in the list.   */
  Iterator
  Begin()
  {
    return m_Deque.begin();
  }

  /** Returns a const iterator pointing to the first element in the list. */
  ConstIterator
  Begin() const
  {
    return m_Deque.begin();
  }

  /** Returns an iterator pointing one element past the last element in the
   * list.    */
  Iterator
  End()
  {
    return m_Deque.end();
  }

  /** Returns a const iterator pointing one element past the last element in the
   * list.    */
  ConstIterator
  End() const
  {
    return m_Deque.end();
  }

  /** Clears the Deque completely */
  void
  Clear()
  {
    m_Deque.clear();
  }

  /** Standard DataObject routine to initialize.  Returns the segment
   * tree to a default state, deallocating memory. */
  void
  Initialize() override;

protected:
  SegmentTree() = default;
  ~SegmentTree() override = default;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  DequeType m_Deque{};
};
} // namespace itk::watershed

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkWatershedSegmentTree.hxx"
#endif

#endif

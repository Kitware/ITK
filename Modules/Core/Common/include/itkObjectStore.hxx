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
#ifndef itkObjectStore_hxx
#define itkObjectStore_hxx

#include "itkNumericTraits.h"
#include "itkPrintHelper.h"


namespace itk
{
template <typename TObjectType>
ObjectStore<TObjectType>::~ObjectStore()
{
  this->Clear();
}

template <typename TObjectType>
void
ObjectStore<TObjectType>::Reserve(SizeValueType n)
{
  // No need to grow? Do nothing.
  if (n <= m_Size)
  {
    return;
  }

  // Need to grow.  Allocate a new block of memory and copy that block's
  // pointers into the m_FreeList.  Updates m_Size appropriately.
  const MemoryBlock new_block(n - m_Size);
  m_Store.push_back(new_block);

  m_FreeList.reserve(n);
  for (ObjectType * ptr = new_block.Begin; ptr < new_block.Begin + new_block.Size; ++ptr)
  {
    m_FreeList.push_back(ptr);
  }
  m_Size += (n - m_Size);
}

template <typename TObjectType>
auto
ObjectStore<TObjectType>::Borrow() -> ObjectType *
{
  if (m_FreeList.empty()) // must allocate more memory
  {
    this->Reserve(m_Size + this->GetGrowthSize());
  }
  ObjectType * p = m_FreeList.back();
  m_FreeList.pop_back();
  return p;

  // To do: This method will need to decrement counters in the memory blocks so
  // that blocks know when they can be deleted.
}

template <typename TObjectType>
void
ObjectStore<TObjectType>::Return(ObjectType * p)
{
  // For speed, does no checking.
  m_FreeList.push_back(p);

  //  if ( m_FreeList.size() != m_Size )
  //    {  m_FreeList.push_back(p); }
  // else object has been used incorrectly

  // To do:
  // Eventually this method will have to increment counters in the memory
  // blocks so that blocks know when they can be deleted.  This will also allow
  // for checking to see if p actually belongs to this store.
  //
  // Problems could easily result if a pointer is Returned more than
  // once. Only time-wise efficient way to deal with this would be a flag in
  // the allocated memory block for each object, sort of how malloc deals with
  // it?
}

template <typename TObjectType>
SizeValueType
ObjectStore<TObjectType>::GetGrowthSize()
{
  if ((m_GrowthStrategy == GrowthStrategyEnum::EXPONENTIAL_GROWTH) && (m_Size != 0))
  {
    return m_Size;
  }
  return m_LinearGrowthSize;
}

template <typename TObjectType>
void
ObjectStore<TObjectType>::Squeeze()
{
  // Not implemented yet.
}

template <typename TObjectType>
void
ObjectStore<TObjectType>::Clear()
{
  // Clear the free list.
  m_FreeList.clear();

  // Empty the MemoryBlock list and deallocate all memory blocks.
  while (!m_Store.empty())
  {
    m_Store.back().Delete();
    m_Store.pop_back();
  }
  m_Size = 0;
}

template <typename TObjectType>
void
ObjectStore<TObjectType>::PrintSelf(std::ostream & os, Indent indent) const
{
  using namespace print_helper;

  Superclass::PrintSelf(os, indent);

  os << indent << "GrowthStrategy: " << m_GrowthStrategy << std::endl;
  os << indent << "Size: " << static_cast<typename NumericTraits<SizeValueType>::PrintType>(m_Size) << std::endl;
  os << indent
     << "LinearGrowthSize: " << static_cast<typename NumericTraits<SizeValueType>::PrintType>(m_LinearGrowthSize)
     << std::endl;
  os << indent << "FreeList: " << m_FreeList << std::endl;
}
} // end namespace itk

#endif

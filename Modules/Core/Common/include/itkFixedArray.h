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
#ifndef itkFixedArray_h
#define itkFixedArray_h

#include "itkMacro.h"
#include "itkMakeFilled.h"
#include <algorithm>
#include <array>

namespace itk
{

/** \class FixedArray
 *  \brief Simulate a standard C array with copy semantics.
 *
 * Simulates a standard C array, except that copy semantics are used instead
 * of reference semantics.  Also, arrays of different sizes cannot be
 * assigned to one another, and size information is known for function
 * returns.
 *
 * \tparam TValue Element type stored at each location in the array.
 * \tparam VLength    = Length of the array.
 *
 * The length of the array is fixed at compile time. If you wish to
 * specify the length of the array at run-time, use the class itk::Array.
 * If you wish to change to change the length of the array at run-time,
 * you're best off using std::vector<>.
 *
 * \ingroup DataRepresentation
 * \ingroup ITKCommon
 *
 * \sphinx
 * \sphinxexample{Core/Common/CreateAFixedArray,Create A Fixed Array}
 * \endsphinx
 */
template <typename TValue, unsigned int VLength = 3>
class ITK_TEMPLATE_EXPORT FixedArray
{
public:
  /** Length constant */
  static constexpr unsigned int Length = VLength;

  /** Dimension constant */
  static constexpr unsigned int Dimension = VLength;

  /** The element type stored at each location in the FixedArray. */
  using ValueType = TValue;

  /** A type representing the C-array version of this FixedArray. */
  using CArray = ValueType[VLength];

  /** An iterator through the array. */
  using Iterator = ValueType *;

  /** A const iterator through the array. */
  using ConstIterator = const ValueType *;

  class ConstReverseIterator;

  /** \class ReverseIterator
   * \brief A reverse iterator through an array.
   * \ingroup ITKCommon
   */
  class ReverseIterator
  {
  public:
    explicit ReverseIterator(Iterator i)
      : m_Iterator(i)
    {}
    ReverseIterator
    operator++()
    {
      return ReverseIterator(--m_Iterator);
    }
    ReverseIterator
    operator++(int)
    {
      return ReverseIterator(m_Iterator--);
    }
    ReverseIterator
    operator--()
    {
      return ReverseIterator(++m_Iterator);
    }
    ReverseIterator
    operator--(int)
    {
      return ReverseIterator(m_Iterator++);
    }
    Iterator
    operator->() const
    {
      return (m_Iterator - 1);
    }
    ValueType &
    operator*() const
    {
      return *(m_Iterator - 1);
    }

    bool
    operator==(const ReverseIterator & rit) const
    {
      return m_Iterator == rit.m_Iterator;
    }

    ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(ReverseIterator);

  private:
    Iterator m_Iterator;
    friend class ConstReverseIterator;
  };

  /** \class ConstReverseIterator
   * \brief A const reverse iterator through an array.
   * \ingroup ITKCommon
   */
  class ConstReverseIterator
  {
  public:
    explicit ConstReverseIterator(ConstIterator i)
      : m_Iterator(i)
    {}
    ConstReverseIterator(const ReverseIterator & rit)
      : m_Iterator(rit.m_Iterator)
    {}
    ConstReverseIterator
    operator++()
    {
      return ConstReverseIterator(--m_Iterator);
    }
    ConstReverseIterator
    operator++(int)
    {
      return ConstReverseIterator(m_Iterator--);
    }
    ConstReverseIterator
    operator--()
    {
      return ConstReverseIterator(++m_Iterator);
    }
    ConstReverseIterator
    operator--(int)
    {
      return ConstReverseIterator(m_Iterator++);
    }
    ConstIterator
    operator->() const
    {
      return (m_Iterator - 1);
    }
    const ValueType &
    operator*() const
    {
      return *(m_Iterator - 1);
    }

    bool
    operator==(const ConstReverseIterator & rit) const
    {
      return m_Iterator == rit.m_Iterator;
    }

    ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(ConstReverseIterator);

  private:
    ConstIterator m_Iterator;
  };

  /** The type of an element. */
  using value_type = TValue;

  /** A pointer to the ValueType. */
  using pointer = ValueType *;

  /** A const pointer to the ValueType. */
  using const_pointer = const ValueType *;

  /** A reference to the ValueType. */
  using reference = ValueType &;

  /** A const reference to the ValueType. */
  using const_reference = const ValueType &;

  /** The return type of the non-const overloads of begin() and end(). */
  using iterator = ValueType *;

  /** The return type of cbegin() and cend(), and the const overloads of begin() and end(). */
  using const_iterator = const ValueType *;

  using reverse_iterator = std::reverse_iterator<iterator>;

  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using SizeType = unsigned int;

public:
  /** Default-constructor.
   * \note The other five "special member functions" are defaulted implicitly, following the C++ "Rule of Zero". */
  FixedArray() = default;

  /** Conversion constructors.
   *
   * Constructor assumes input points to array of correct size.
   * Values are copied individually instead of with a binary copy.  This
   * allows the ValueType's assignment operator to be executed.
   */
  /** @ITKStartGrouping */
  FixedArray(const ValueType r[VLength]);
  FixedArray(const ValueType &);
  /** @ITKEndGrouping */
  /** Explicit constructor for std::array. */
  explicit FixedArray(const std::array<ValueType, VLength> & stdArray)
  {
    std::copy_n(stdArray.cbegin(), VLength, m_InternalArray);
  }

  /** Constructor to initialize a fixed array from another of any data type */
  template <typename TFixedArrayValueType>
  FixedArray(const FixedArray<TFixedArrayValueType, VLength> & r)
  {
    auto input = r.cbegin();

    for (auto & element : m_InternalArray)
    {
      element = static_cast<TValue>(*input++);
    }
  }

  template <typename TScalarValue>
  FixedArray(const TScalarValue * r)
  {
    std::copy_n(r, VLength, m_InternalArray);
  }

  /** Operator= defined for a variety of types.
   *
   * The assignment operator assumes input points to array of correct size.
   * Values are copied individually instead of with a binary copy.  This
   * allows the ValueType's assignment operator to be executed.
   */
  template <typename TFixedArrayValueType>
  FixedArray &
  operator=(const FixedArray<TFixedArrayValueType, VLength> & r)
  {
    auto input = r.cbegin();

    for (auto & element : m_InternalArray)
    {
      element = static_cast<TValue>(*input++);
    }
    return *this;
  }

  FixedArray &
  operator=(const ValueType r[VLength]);

  /** Operators == and != are used to compare whether two arrays are equal.
   * Note that arrays are equal when the number of components (size) is the
   * same, and each component value is equal. */
  bool
  operator==(const FixedArray & r) const;

  ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(FixedArray);


  /** Allow the FixedArray to be indexed normally.  No bounds checking is done.
   */
  /** @ITKStartGrouping */
  // false positive warnings with GCC
  ITK_GCC_PRAGMA_PUSH
  ITK_GCC_SUPPRESS_Warray_bounds
  constexpr reference
  operator[](unsigned int index)
  {
    return m_InternalArray[index];
  }
  constexpr const_reference
  operator[](unsigned int index) const
  {
    return m_InternalArray[index];
  }
  ITK_GCC_PRAGMA_POP
  /** @ITKEndGrouping */
  /** Set/Get element methods are more convenient in wrapping languages */
  /** @ITKStartGrouping */
  void
  SetElement(unsigned int index, const_reference value)
  {
    m_InternalArray[index] = value;
  }
  [[nodiscard]] const_reference
  GetElement(unsigned int index) const
  {
    return m_InternalArray[index];
  }
  /** @ITKEndGrouping */
  /** Return a pointer to the data. */
  ValueType *
  GetDataPointer()
  {
    return m_InternalArray;
  }

  [[nodiscard]] const ValueType *
  GetDataPointer() const
  {
    return m_InternalArray;
  }

  ValueType *
  data()
  {
    return m_InternalArray;
  }

  [[nodiscard]] const ValueType *
  data() const
  {
    return m_InternalArray;
  }

  /** Get an Iterator for the beginning of the FixedArray. */
  Iterator
  Begin();

  /** Get a ConstIterator for the beginning of the FixedArray. */
  [[nodiscard]] ConstIterator
  Begin() const;

  /** Get an Iterator for the end of the FixedArray. */
  Iterator
  End();

  /** Get a ConstIterator for the end of the FixedArray. */
  [[nodiscard]] ConstIterator
  End() const;

  /** Get a begin ReverseIterator. */
  itkLegacyMacro(ReverseIterator rBegin();)

  /** Get a begin ConstReverseIterator. */
  itkLegacyMacro(ConstReverseIterator rBegin() const;)

  /** Get an end ReverseIterator. */
  itkLegacyMacro(ReverseIterator rEnd();)

  /** Get an end ConstReverseIterator. */
  itkLegacyMacro(ConstReverseIterator rEnd() const;)

  [[nodiscard]] constexpr const_iterator
  cbegin() const noexcept
  {
    return m_InternalArray;
  }

  constexpr iterator
  begin() noexcept
  {
    return m_InternalArray;
  }

  [[nodiscard]] constexpr const_iterator
  begin() const noexcept
  {
    return this->cbegin();
  }

  [[nodiscard]] constexpr const_iterator
  cend() const noexcept
  {
    return m_InternalArray + VLength;
  }

  constexpr iterator
  end() noexcept
  {
    return m_InternalArray + VLength;
  }

  [[nodiscard]] constexpr const_iterator
  end() const noexcept
  {
    return this->cend();
  }

  reverse_iterator
  rbegin()
  {
    return reverse_iterator{ this->end() };
  }

  [[nodiscard]] const_reverse_iterator
  crbegin() const
  {
    return const_reverse_iterator{ this->cend() };
  }

  [[nodiscard]] const_reverse_iterator
  rbegin() const
  {
    return this->crbegin();
  }

  reverse_iterator
  rend()
  {
    return reverse_iterator{ this->begin() };
  }

  [[nodiscard]] const_reverse_iterator
  crend() const
  {
    return const_reverse_iterator{ this->cbegin() };
  }

  [[nodiscard]] const_reverse_iterator
  rend() const
  {
    return this->crend();
  }

  /** Size of the container. */
  [[nodiscard]] SizeType
  Size() const;

  [[nodiscard]] constexpr SizeType
  size() const
  {
    return VLength;
  }

  /** Set all the elements of the container to the input value. */
  void
  Fill(const ValueType &);

  void
  swap(FixedArray & other) noexcept
  {
    std::swap(m_InternalArray, other.m_InternalArray);
  }

private:
  /** Internal C array representation. */
  CArray m_InternalArray;

public:
  /** Return an FixedArray with the given value assigned to all elements. */
  static constexpr FixedArray
  Filled(const ValueType & value)
  {
    return MakeFilled<FixedArray>(value);
  }
};

template <typename TValue, unsigned int VLength>
std::ostream &
operator<<(std::ostream & os, const FixedArray<TValue, VLength> & arr);


template <typename TValue, unsigned int VLength>
inline void
swap(FixedArray<TValue, VLength> & a, FixedArray<TValue, VLength> & b) noexcept
{
  a.swap(b);
}

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkFixedArray.hxx"
#endif

#include "itkNumericTraitsFixedArrayPixel.h"

#endif

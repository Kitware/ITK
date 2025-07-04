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
#ifndef itkRGBPixel_h
#define itkRGBPixel_h

// Undefine an eventual RGBPixel macro
#ifdef RGBPixel
#  undef RGBPixel
#endif

#include "itkIndent.h"
#include "itkFixedArray.h"
#include "itkMath.h"

namespace itk
{
/** \class RGBPixel
 * \brief Represent Red, Green and Blue components for color images.
 *
 * This class is templated over the representation used for each
 * component.
 *
 * The following syntax for assigning an index is allowed/suggested:
 *
   \code
      RGBPixel<float> pixel; pixel = 1.0f, 0.0f, .5f;
      RGBPixel<char> pixelArray[2];
      pixelArray[0] = 255, 255, 255;
      pixelArray[1] = 255, 255, 244;
   \endcode
 *
 * Since RGBPixel is a subclass of Array, you can access its components as:
 * pixel[0], pixel[1], pixel[2]
 * \ingroup ImageObjects
 * \ingroup ITKCommon
 *
 * \sphinx
 * \sphinxexample{Core/Common/CreateAnRGBImage,Create An RGB image}
 * \endsphinx
 */

template <typename TComponent = unsigned short>
class ITK_TEMPLATE_EXPORT RGBPixel : public FixedArray<TComponent, 3>
{
public:
  /** Standard class type aliases. */
  using Self = RGBPixel;
  using Superclass = FixedArray<TComponent, 3>;

  /** Convenience type alias. */
  using BaseArray = FixedArray<TComponent, 3>;

  /** Dimension of the vector space. */
  static constexpr unsigned int Dimension = 3;

  /** Length of the pixel. */
  static constexpr unsigned int Length = 3;

  /**  Define the component type. */
  using ComponentType = TComponent;
  using LuminanceType = typename NumericTraits<ComponentType>::RealType;

  /** Default-constructor.
   * \note The other five "special member functions" are defaulted implicitly, following the C++ "Rule of Zero". */
#ifdef ITK_FUTURE_LEGACY_REMOVE
  RGBPixel() = default;
#else
  constexpr RGBPixel()
    : Superclass(Superclass())
  {
    // `: Superclass(Superclass())` is a workaround for an old compiler bug. A simple `: Superclass()` triggered
    // warnings from GCC 9.4.0 saying: "warning: '<anonymous>' may be used uninitialized in this function
    // [-Wmaybe-uninitialized]".
  }
#endif

#if defined(ITK_LEGACY_REMOVE)
  /** Explicit constructor to fill Red=Blue=Green= r. */
  explicit RGBPixel(const ComponentType & r) { this->Fill(r); }

  /** Prevents copy-initialization from `nullptr`, as well as from `0` (NULL). */
  RGBPixel(std::nullptr_t) = delete;
#else

  /** Constructor to fill Red=Blue=Green= r.
   * \note ITK_LEGACY_REMOVE=ON will disallow implicit conversion from a component value. */
  RGBPixel(const ComponentType & r) { this->Fill(r); }
#endif

  /** Pass-through constructor for the Array base class. */
  /** @ITKStartGrouping */
  template <typename TRGBPixelValueType>
  RGBPixel(const RGBPixel<TRGBPixelValueType> & r)
    : BaseArray(r)
  {}
  RGBPixel(const ComponentType r[3])
    : BaseArray(r)
  {}
  /** @ITKEndGrouping */

  /** Pass-through assignment operator for the Array base class. */
  /** @ITKStartGrouping */
  template <typename TRGBPixelValueType>
  Self &
  operator=(const RGBPixel<TRGBPixelValueType> & r)
  {
    BaseArray::operator=(r);
    return *this;
  }
  /** @ITKEndGrouping */

  Self &
  operator=(const ComponentType r[3]);

  /** Arithmetic operations between pixels. Return a new RGBPixel. */
  /** @ITKStartGrouping */
  Self
  operator+(const Self & r) const;
  Self
  operator-(const Self & r) const;
  Self
  operator*(const ComponentType & r) const;
  Self
  operator/(const ComponentType & r) const;
  /** @ITKEndGrouping */

  /** Arithmetic-assignment operators. */
  /** @ITKStartGrouping */
  const Self &
  operator+=(const Self & r);
  const Self &
  operator-=(const Self & r);
  const Self &
  operator*=(const ComponentType & r);
  const Self &
  operator/=(const ComponentType & r);
  /** @ITKEndGrouping */

  /** Implements strict weak ordering. For use in STL, e.g. std::map. */
  bool
  operator<(const Self & r) const;

  bool
  operator==(const Self & r) const;

  /** Return the number of components. */
  static unsigned int
  GetNumberOfComponents()
  {
    return 3;
  }

  /** Return the value for the Nth component. */
  [[nodiscard]] ComponentType
  GetNthComponent(int c) const
  {
    return this->operator[](c);
  }

  /** Return the Euclidean norm of the vector defined by the RGB components. */
  [[nodiscard]] ComponentType
  GetScalarValue() const
  {
    return static_cast<ComponentType>(
      std::sqrt(static_cast<double>(this->operator[](0)) * static_cast<double>(this->operator[](0)) +
                static_cast<double>(this->operator[](1)) * static_cast<double>(this->operator[](1)) +
                static_cast<double>(this->operator[](2)) * static_cast<double>(this->operator[](2))));
  }

  /** Set the Nth component to v. */
  void
  SetNthComponent(int c, const ComponentType & v)
  {
    this->operator[](c) = v;
  }

  /** Set the Red component. */
  void
  SetRed(ComponentType red)
  {
    this->operator[](0) = red;
  }

  /** Set the Green component. */
  void
  SetGreen(ComponentType green)
  {
    this->operator[](1) = green;
  }

  /** Set the Blue component. */
  void
  SetBlue(ComponentType blue)
  {
    this->operator[](2) = blue;
  }

  /** Set the three components. */
  void
  Set(ComponentType red, ComponentType green, ComponentType blue)
  {
    this->operator[](0) = red;
    this->operator[](1) = green;
    this->operator[](2) = blue;
  }

  /** Get the Red component. */
  [[nodiscard]] const ComponentType &
  GetRed() const
  {
    return this->operator[](0);
  }

  /** Get the Green component. */
  [[nodiscard]] const ComponentType &
  GetGreen() const
  {
    return this->operator[](1);
  }

  /** Get the Blue component. */
  [[nodiscard]] const ComponentType &
  GetBlue() const
  {
    return this->operator[](2);
  }

  /** Get Luminance out of RGB */
  [[nodiscard]] LuminanceType
  GetLuminance() const;
};

template <typename TComponent>
std::ostream &
operator<<(std::ostream & os, const RGBPixel<TComponent> & c);

template <typename TComponent>
std::istream &
operator>>(std::istream & is, RGBPixel<TComponent> & c);

template <typename T>
inline void
swap(RGBPixel<T> & a, RGBPixel<T> & b) noexcept
{
  a.swap(b);
}

} // end namespace itk

//
// Numeric traits must be included after (optionally) including the explicit
// instantiations control of this class, in case the implicit instantiation
// needs to be disabled.
//
// NumericTraits must be included before (optionally) including the .hxx file,
// in case the .hxx requires to use NumericTraits.
//
#include "itkNumericTraitsRGBPixel.h"


#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkRGBPixel.hxx"
#endif

#endif

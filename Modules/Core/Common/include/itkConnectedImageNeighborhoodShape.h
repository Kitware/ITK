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

#ifndef itkConnectedImageNeighborhoodShape_h
#define itkConnectedImageNeighborhoodShape_h

#include "itkIntTypes.h" // For uintmax_t
#include "itkMath.h"
#include "itkOffset.h"

#include <array>
#include <cassert>
#include <limits>

namespace itk
{

/**
 * \class ConnectedImageNeighborhoodShape
 * Connected image-neighborhood shape, based on the topological property of
 * pixel connectivity. Eases creating a sequence of offsets to construct a
 * ShapedImageNeighborhoodRange object. Can also be used to specify the shape
 * of a ShapedNeighborhoodIterator, using its ActivateOffset member function.
 *
 * This shape class supports generating offsets in colexicographic order. Which
 * means that, for example, a sequence of generated offsets for a 2-dimensional
 * shape will have offset {1, 0} before offset {0, 1}. This order was chosen
 * because it is usually in agreement with the order of the corresponding
 * neighbor pixels, as stored in the internal image buffer.
 *
 * The following example generates the offsets for a 3-dimensional 18-connected
 * neighborhood shape, including the center pixel, and asserts that the result
 * is as expected:
   \code
   size_t maximumCityblockDistance = 2;
   bool includeCenterPixel = true;
   ConnectedImageNeighborhoodShape<3> shape{ maximumCityblockDistance, includeCenterPixel };
   std::vector<Offset<3>> offsets = GenerateImageNeighborhoodOffsets(shape);
   assert(offsets.size() == 19);
   assert(offsets == std::vector<Offset<3>>(
   {
     {{0, -1, -1}}, {{-1, 0, -1}}, {{0, 0, -1}},
     {{1, 0, -1}}, {{0, 1, -1}}, {{-1, -1, 0}},
     {{0, -1, 0}}, {{1, -1, 0}}, {{-1, 0, 0}},
     {{0, 0, 0}},
     {{1, 0, 0}}, {{-1, 1, 0}}, {{0, 1, 0}},
     {{1, 1, 0}}, {{0, -1, 1}}, {{-1, 0, 1}},
     {{0, 0, 1}}, {{1, 0, 1}}, {{0, 1, 1}}
   }));
   \endcode
 *
 * The following code shows how to create 4-connected, 8-connected,
 * 6-connected, 18-connected, and 26-connected neighborhood shapes:
   \code
   // 2-dimensional:
   ConnectedImageNeighborhoodShape<2> _4_connectedShape{ 1, includeCenterPixel };
   ConnectedImageNeighborhoodShape<2> _8_connectedShape{ 2, includeCenterPixel };
   // 3-dimensional:
   ConnectedImageNeighborhoodShape<3> _6_connectedShape{ 1, includeCenterPixel };
   ConnectedImageNeighborhoodShape<3> _18_connectedShape{ 2, includeCenterPixel };
   ConnectedImageNeighborhoodShape<3> _26_connectedShape{ 3, includeCenterPixel };
   \endcode
 *
 * \author Niels Dekker, LKEB, Leiden University Medical Center
 *
 * \see ShapedImageNeighborhoodRange
 * \see ShapedNeighborhoodIterator
 * \ingroup ImageIterators
 * \ingroup ITKCommon
 */
template <unsigned int VImageDimension>
class ConnectedImageNeighborhoodShape
{
public:
  static constexpr unsigned int ImageDimension = VImageDimension;

  /** Constructs a connected image-neighborhood shape. Its offsets contain only
   * the offset values -1, 0, and 1.
   * The parameter 'maximumCityblockDistance' specifies the maximum city-block
   * distance (Manhattan distance) between the center pixel and the connected
   * neighbor pixel. This distance measure corresponds to the number of
   * non-zero values of an offset. For example, in a 3-dimensional neighborhood,
   * offset {1,0,0}, {0,1,0}, and {0,0,1} have distance = 1 to the center,
   * while offset {1,1,1} has distance = 3.
   * The parameter 'includeCenterPixel' specifies whether or not the center
   * pixel (offset zero) should be included with the offsets for this shape.
   */
  constexpr explicit ConnectedImageNeighborhoodShape(const size_t maximumCityblockDistance,
                                                     const bool   includeCenterPixel) noexcept
    : m_MaximumCityblockDistance{ maximumCityblockDistance }
    , m_IncludeCenterPixel{ includeCenterPixel }
    , m_NumberOfOffsets{ CalculateNumberOfOffsets(maximumCityblockDistance, includeCenterPixel) }
  {}

  /** Returns the number of offsets needed for this shape. */
  [[nodiscard]] constexpr size_t
  GetNumberOfOffsets() const noexcept
  {
    return m_NumberOfOffsets;
  }


  /** Fills the specified buffer with the offsets for a neighborhood of this shape. */
  void
  FillOffsets(Offset<ImageDimension> * const offsets) const noexcept
  {
    if (m_NumberOfOffsets > 0)
    {
      assert(offsets != nullptr);
      Offset<ImageDimension> offset;
      std::fill_n(offset.begin(), ImageDimension, -1);

      size_t i = 0;

      while (i < m_NumberOfOffsets)
      {
        const size_t numberOfNonZeroOffsetValues =
          ImageDimension - static_cast<size_t>(std::count(offset.begin(), offset.end(), 0));

        if ((m_IncludeCenterPixel || (numberOfNonZeroOffsetValues > 0)) &&
            (numberOfNonZeroOffsetValues <= m_MaximumCityblockDistance))
        {
          offsets[i] = offset;
          ++i;
        }

        // Go to the next offset:
        for (unsigned int direction = 0; direction < ImageDimension; ++direction)
        {
          auto & offsetValue = offset[direction];

          ++offsetValue;

          if (offsetValue <= 1)
          {
            break;
          }
          offsetValue = -1;
        }
      }
    }
  }

private:
  // The maximum city-block distance (Manhattan distance) between the center
  // pixel and each connected neighbor pixel.
  size_t m_MaximumCityblockDistance;

  // Specifies whether or not the center pixel (offset zero) should be included
  // with the offsets for this shape.
  bool m_IncludeCenterPixel;

  // The number of offsets needed to represent this shape.
  size_t m_NumberOfOffsets;


  // Calculates a + b. Numeric overflow triggers a compilation error in
  // "constexpr context" and a debug assert failure at run-time.
  static constexpr uintmax_t
  CalculateSum(const uintmax_t a, const uintmax_t b) noexcept
  {
    return ((a + b) >= a) && ((a + b) >= b) ? (a + b) : (assert(!"CalculateSum overflow!"), 0);
  }


  // Calculates 2 ^ n. Numeric overflow triggers a compilation error in
  // "constexpr context" and a debug assert failure at run-time.
  static constexpr uintmax_t
  CalculatePowerOfTwo(const size_t n) noexcept
  {
    return (n < std::numeric_limits<uintmax_t>::digits) ? (uintmax_t{ 1 } << n)
                                                        : (assert(!"CalculatePowerOfTwo overflow!"), 0);
  }


  // Calculates the binomial coefficient, 'n' over 'k'.
  // Inspired by the 'binom' function from Walter, June 23, 2017:
  // https://stackoverflow.com/questions/44718971/calculate-binomial-coffeficient-very-reliable/44719165#44719165
  // Optimized for small values of 'k' (k <= n/2).
  static constexpr uintmax_t
  CalculateBinomialCoefficient(const uintmax_t n, const uintmax_t k) noexcept
  {
    return (k > n)    ? (assert(!"Out of range!"), 0)
           : (k == 0) ? 1
                      : Math::UnsignedProduct(n, CalculateBinomialCoefficient(n - 1, k - 1)) / k;
  }


  // Calculates the number of m-dimensional hypercubes on the boundary of an
  // n-cube, as described at https://en.wikipedia.org/wiki/Hypercube#Elements
  // (Which refers to H.S.M. Coxeter, Regular polytopes, 3rd ed., 1973, p.120.)
  static constexpr size_t
  CalculateNumberOfHypercubesOnBoundaryOfCube(const size_t m, const size_t n) noexcept
  {
    // Calculate 2^(n-m) * BinomialCoefficient(n, m)
    return Math::UnsignedProduct(CalculatePowerOfTwo(n - m),
                                 (((2 * m) < n) ?
                                                // Calculate either the binomial coefficient of (n, m) or (n, n - m).
                                                // Mathematically, both should yield the same number, but the
                                                // calculation is optimized for a smaller second argument.
                                    CalculateBinomialCoefficient(n, m)
                                                : CalculateBinomialCoefficient(n, n - m)));
  }


  // Iterates recursively from i = ImageDimension-1 down to m (inclusive).
  static constexpr size_t
  CalculateSumOfNumberOfHypercubesOnBoundaryOfCube(const size_t i, const size_t m) noexcept
  {
    return assert(i >= m), CalculateSum(CalculateNumberOfHypercubesOnBoundaryOfCube(i, ImageDimension),
                                        ((i <= m) ? 0 : CalculateSumOfNumberOfHypercubesOnBoundaryOfCube(i - 1, m)));
  }


  /** Calculates the number of neighbors connected to the center pixel. */
  static constexpr size_t
  CalculateNumberOfConnectedNeighbors(const size_t maximumCityblockDistance) noexcept
  {
    return (((maximumCityblockDistance == 0) || (ImageDimension == 0))
              ? 0
              : ((maximumCityblockDistance >= ImageDimension)
                   ? (Math::UnsignedPower(3, ImageDimension) - 1)
                   : CalculateSumOfNumberOfHypercubesOnBoundaryOfCube(ImageDimension - 1,
                                                                      (ImageDimension - maximumCityblockDistance))));
  }


  /** Calculates the number of offsets needed for this shape. */
  static constexpr size_t
  CalculateNumberOfOffsets(const size_t maximumCityblockDistance, const bool includeCenterPixel) noexcept
  {
    return (includeCenterPixel ? 1 : 0) + CalculateNumberOfConnectedNeighbors(maximumCityblockDistance);
  }
};

/** Generates the offsets for a connected image neighborhood shape. */
template <unsigned int VImageDimension, size_t VMaximumCityblockDistance, bool VIncludeCenterPixel>
auto
GenerateConnectedImageNeighborhoodShapeOffsets() noexcept
{
  constexpr ConnectedImageNeighborhoodShape<VImageDimension> shape{ VMaximumCityblockDistance, VIncludeCenterPixel };
  std::array<Offset<VImageDimension>, shape.GetNumberOfOffsets()> offsets;
  shape.FillOffsets(offsets.data());
  return offsets;
}

} // namespace itk

#endif

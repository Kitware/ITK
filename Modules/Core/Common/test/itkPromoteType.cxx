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

#include "itkPromoteType.h"
#include <complex>
#include <type_traits>

namespace itk::mpl
{

template <typename TA, typename TB>
struct PromoteType<std::complex<TA>, std::complex<TB>>
{
  using Type = std::complex<typename PromoteType<TA, TB>::Type>;
};
} // namespace itk::mpl


int
itkPromoteType(int, char *[])
{
  using namespace itk::mpl;

  // Prvalues of small integral types (such as char) may be converted to prvalues
  // of larger integral types (such as int). In particular, arithmetic operators
  // do not accept types smaller than int as arguments, and integral promotions
  // are automatically applied after lvalue-to-rvalue conversion, if applicable.
  // This conversion always preserves the value.
  //
  // The following implicit conversions are classified as integral promotions:
  // * signed char or short can be converted to int.
  // * unsigned char or unsigned short can be converted to int if it can hold
  //   its entire value range, and unsigned int otherwise.
  // * char can be converted to int or unsigned int depending on the underlying
  //   type: signed char or unsigned char (see above)
  // * wchar_t, char16_t, and char32_t can be converted to the first type from
  //   the following list able to hold their entire value range: int, unsigned
  //   int, long, unsigned long, long long, unsigned long long.

  static_assert(std::is_same_v<PromoteType<signed char, int>::Type, int>, "test failed");
  static_assert(std::is_same_v<PromoteType<signed char, short>::Type, int>, "test failed");
  static_assert(std::is_same_v<PromoteType<unsigned char, int>::Type, int>, "test failed");
  static_assert(std::is_same_v<PromoteType<unsigned char, unsigned int>::Type, unsigned int>, "test failed");
  static_assert(std::is_same_v<PromoteType<int, int>::Type, int>, "test failed");
  static_assert(std::is_same_v<PromoteType<short, int>::Type, int>, "test failed");
  static_assert(std::is_same_v<PromoteType<double, int>::Type, double>, "test failed");
  static_assert(std::is_same_v<PromoteType<float, int>::Type, float>, "test failed");
  static_assert(std::is_same_v<PromoteType<long, int>::Type, long>, "test failed");
  static_assert(std::is_same_v<PromoteType<long long, int>::Type, long long>, "test failed");
  static_assert(std::is_same_v<PromoteType<int, long long>::Type, long long>, "test failed");
  static_assert(std::is_same_v<PromoteType<long, long double>::Type, long double>, "test failed");
  static_assert(std::is_same_v<PromoteType<double, std::complex<double>>::Type, std::complex<double>>, "test failed");

  static_assert(std::is_same_v<PromoteType<std::complex<int>, std::complex<double>>::Type, std::complex<double>>,
                "test failed");
  return EXIT_SUCCESS;
}

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
#ifndef itkWindowedSincInterpolateImageFunction_h
#define itkWindowedSincInterpolateImageFunction_h

#include "itkConstNeighborhoodIterator.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkInterpolateImageFunction.h"
#include "itkMath.h"

namespace itk
{
// clang-format off
namespace Function
{
/**
 * \class CosineWindowFunction
 * \brief Window function for sinc interpolation.
 * \f[ w(x) = cos(\frac{\pi x}{2 m} ) \f]
 * \sa WindowedSincInterpolateImageFunction
 * \ingroup ITKImageFunction
 */
template <unsigned int VRadius, typename TInput = double, typename TOutput = double>
class ITK_TEMPLATE_EXPORT CosineWindowFunction
{
public:
  inline TOutput
  operator()(const TInput & A) const
  {

    /** Equal to \f$ \frac{\pi}{2 m} \f$ */
    static constexpr double factor = Math::pi / (2 * VRadius);
    return static_cast<TOutput>(std::cos(A * factor));
  }
};

/**
 * \class HammingWindowFunction
 * \brief Window function for sinc interpolation.
 * \f[ w(x) = 0.54 + 0.46 cos(\frac{\pi x}{m} ) \f]
 * \sa WindowedSincInterpolateImageFunction
 * \ingroup ITKImageFunction
 */
template <unsigned int VRadius, typename TInput = double, typename TOutput = double>
class ITK_TEMPLATE_EXPORT HammingWindowFunction
{
public:
  inline TOutput
  operator()(const TInput & A) const
  {

    /** Equal to \f$ \frac{\pi}{m} \f$ */
    static constexpr double factor = Math::pi / VRadius;
    return static_cast<TOutput>(0.54 + 0.46 * std::cos(A * factor));
  }
};

/**
 * \class WelchWindowFunction
 * \brief Window function for sinc interpolation.
 * \f[ w(x) = 1 - ( \frac{x^2}{m^2} ) \f]
 * \sa WindowedSincInterpolateImageFunction
 * \ingroup ITKImageFunction
 */
template <unsigned int VRadius, typename TInput = double, typename TOutput = double>
class ITK_TEMPLATE_EXPORT WelchWindowFunction
{
public:
  /** Equal to \f$ \frac{1}{m^2} \f$ */
  inline TOutput
  operator()(const TInput & A) const
  {

    static constexpr double factor = 1.0 / (VRadius * VRadius);
    return static_cast<TOutput>(1.0 - A * factor * A);
  }
};

/**
 * \class LanczosWindowFunction
 * \brief Window function for sinc interpolation.
 * \f[ w(x) = \textrm{sinc} ( \frac{x}{m} ) \f]
 * Note: Paper referenced in WindowedSincInterpolateImageFunction gives
 * an incorrect definition of this window function.
 * \sa WindowedSincInterpolateImageFunction
 * \ingroup ITKImageFunction
 */
template <unsigned int VRadius, typename TInput = double, typename TOutput = double>
class ITK_TEMPLATE_EXPORT LanczosWindowFunction
{
public:
  inline TOutput
  operator()(const TInput & A) const
  {
    if (A == 0.0)
    {
      return static_cast<TOutput>(1.0);
    } // namespace Function

    /** Equal to \f$ \frac{\pi}{m} \f$ */
    static constexpr double factor = Math::pi / VRadius;
    const double z = factor * A;
    return static_cast<TOutput>(std::sin(z) / z);
  } // namespace itk
};

/**
 * \class BlackmanWindowFunction
 * \brief Window function for sinc interpolation.
 * \f[ w(x) = 0.42 + 0.5 cos(\frac{\pi x}{m}) + 0.08 cos(\frac{2 \pi x}{m}) \f]
 * \sa WindowedSincInterpolateImageFunction
 * \ingroup ITKImageFunction
 */
template <unsigned int VRadius, typename TInput = double, typename TOutput = double>
class ITK_TEMPLATE_EXPORT BlackmanWindowFunction
{
public:
  inline TOutput
  operator()(const TInput & A) const
  {

    /** Equal to \f$ \frac{\pi}{m} \f$ */
    static constexpr double factor1 = Math::pi / VRadius;

    /** Equal to \f$ \frac{2 \pi}{m} \f$  */
    static constexpr double factor2 = 2.0 * Math::pi / VRadius;
    return static_cast<TOutput>(0.42 + 0.5 * std::cos(A * factor1) + 0.08 * std::cos(A * factor2));
  }
};
} // namespace Function
// clang-format on

/**
 * \class WindowedSincInterpolateImageFunction
 * \brief Use the windowed sinc function to interpolate
 * \author Paul A. Yushkevich
 *
 * \par THEORY
 *
 * This function is intended to provide an interpolation function that
 * has minimum aliasing artifacts, in contrast to linear interpolation.
 * According to sampling theory, the infinite-support sinc filter,
 * whose Fourier transform is the box filter, is optimal for resampling
 * a function. In practice, the infinite support sinc filter is
 * approximated using a limited support 'windowed' sinc filter.
 *
 * \par
 * This function is based on \cite meijering1999.
 *
 * \par
 * In this work, several 'windows' are estimated. In two dimensions, the
 * interpolation at a position (x,y) is given by the following
 * expression:
 *
 * \par
 * \f[
 *   I(x,y) =
 *     \sum_{i = \lfloor x \rfloor + 1 - m}^{\lfloor x \rfloor + m}
 *     \sum_{j = \lfloor y \rfloor + 1 - m}^{\lfloor y \rfloor + m}
 *     I_{i,j} K(x-i) K(y-j),
 * \f]
 *
 * \par
 * where m is the 'radius' of the window, (3,4 are reasonable numbers),
 * and K(t) is the kernel function, composed of the sinc function and
 * one of several possible window functions:
 *
 * \par
 * \f[
 *   K(t) = w(t) \textrm{sinc}(t) = w(t) \frac{\sin(\pi t)}{\pi t}
 * \f]
 *
 * \par
 * Several window functions are provided here in the itk::Function
 * namespace. The conclusions of the referenced paper suggest to use the
 * Welch, Cosine, Kaiser, and Lanczos windows for m = 4,5. These are based
 * on error in rotating medical images w.r.t. the linear interpolation
 * method. In some cases the results achieve a 20-fold improvement in
 * accuracy.
 *
 * \par USING THIS FILTER
 *
 * Use this filter the way you would use any ImageInterpolationFunction,
 * so for instance, you can plug it into the ResampleImageFilter class.
 * In order to initialize the filter you must choose several template
 * parameters.
 *
 * \par
 * The first (TInputImage) is the image type, that's standard.
 *
 * \par
 * The second (VRadius) is the radius of the kernel, i.e., the
 * \f$ m \f$ from the formula above.
 *
 * \par
 * The third (TWindowFunction) is the window function object, which you
 * can choose from about five different functions defined in this
 * header. The default is the Hamming window, which is commonly used
 * but not optimal according to the cited paper.
 *
 * \par
 * The fourth (TBoundaryCondition) is the boundary condition class used
 * to determine the values of pixels that fall off the image boundary.
 * This class has the same meaning here as in the NeighborhoodIterator
 * classes.
 *
 * \par
 * The fifth (TCoordinate) is again standard for interpolating functions,
 * and should be float or double.
 *
 * \par CAVEATS
 *
 * There are a few improvements that an enthusiastic ITK developer
 * could make to this filter. One issue is with the way that the kernel
 * is applied. The computational expense comes from two sources:
 * computing the kernel weights K(t) and multiplying the pixels in the
 * window by the kernel weights. The first is done more or less
 * efficiently in \f$ 2 m d \f$ operations (where d is the
 * dimensionality of the image). The second can be done
 * better. Presently, each pixel \f$ I(i,j,k) \f$ is multiplied by the
 * weights \f$ K(x-i), K(y-j), K(z-k) \f$ and added to the running
 * total. This results in \f$ d (2m)^d \f$ multiplication
 * operations. However, by keeping intermediate sums, it would be
 * possible to do the operation in \f$ O ( (2m)^d ) \f$
 * operations. This would require some creative coding. In addition, in
 * the case when one of the coordinates is integer, the computation
 * could be reduced by an order of magnitude.
 *
 * \sa LinearInterpolateImageFunction ResampleImageFilter
 * \sa Function::HammingWindowFunction
 * \sa Function::CosineWindowFunction
 * \sa Function::WelchWindowFunction
 * \sa Function::LanczosWindowFunction
 * \sa Function::BlackmanWindowFunction
 * \ingroup ImageFunctions ImageInterpolators
 * \ingroup ITKImageFunction
 */
template <typename TInputImage,
          unsigned int VRadius,
          typename TWindowFunction = Function::HammingWindowFunction<VRadius>,
          class TBoundaryCondition = ZeroFluxNeumannBoundaryCondition<TInputImage, TInputImage>,
          class TCoordinate = double>
class ITK_TEMPLATE_EXPORT WindowedSincInterpolateImageFunction
  : public InterpolateImageFunction<TInputImage, TCoordinate>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(WindowedSincInterpolateImageFunction);

  /** Standard class type aliases. */
  using Self = WindowedSincInterpolateImageFunction;
  using Superclass = InterpolateImageFunction<TInputImage, TCoordinate>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(WindowedSincInterpolateImageFunction);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** OutputType type alias support */
  using typename Superclass::OutputType;

  /** InputImageType type alias support */
  using typename Superclass::InputImageType;

  /** RealType type alias support */
  using typename Superclass::RealType;

  /** Dimension underlying input image. */
  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  /** Index type alias support */
  using typename Superclass::IndexType;
  using typename Superclass::IndexValueType;

  /** Size type alias support */
  using typename Superclass::SizeType;

  /** Image type definition */
  using ImageType = TInputImage;

  /** ContinuousIndex type alias support */
  using typename Superclass::ContinuousIndexType;

  void
  SetInputImage(const ImageType * image) override;

  /** Evaluate the function at a ContinuousIndex position
   *
   * Returns the interpolated image intensity at a
   * specified point position.  Bounds checking is based on the
   * type of the TBoundaryCondition specified.
   */
  OutputType
  EvaluateAtContinuousIndex(const ContinuousIndexType & index) const override;

  SizeType
  GetRadius() const override
  {
    constexpr auto radius = SizeType::Filled(VRadius);
    return radius;
  }

protected:
  WindowedSincInterpolateImageFunction() = default;
  ~WindowedSincInterpolateImageFunction() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

private:
  // Internal type alias
  using IteratorType = ConstNeighborhoodIterator<ImageType, TBoundaryCondition>;

  // Constant to store twice the radius
  static constexpr unsigned int m_WindowSize{ 2 * VRadius };

  /** The function object, used to compute window */
  TWindowFunction m_WindowFunction{};

  /** Size of the offset table */
  static constexpr unsigned int m_OffsetTableSize = Math::UnsignedPower(m_WindowSize, ImageDimension);

  /** The offset array, used to keep a list of relevant
   * offsets in the neighborhoodIterator */
  unsigned int m_OffsetTable[m_OffsetTableSize]{};

  /** Index into the weights array for each offset */
  unsigned int m_WeightOffsetTable[m_OffsetTableSize][ImageDimension]{};

  /** The sinc function */
  static double
  Sinc(const double x)
  {
    const double px = Math::pi * x;
    return (x == 0.0) ? 1.0 : std::sin(px) / px;
  }
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkWindowedSincInterpolateImageFunction.hxx"
#endif

#endif // _itkWindowedSincInterpolateImageFunction_h

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
#ifndef itkEllipsoidInteriorExteriorSpatialFunction_h
#define itkEllipsoidInteriorExteriorSpatialFunction_h

#include "itkInteriorExteriorSpatialFunction.h"
#include "vnl/vnl_matrix_fixed.h"

namespace itk
{
/**
 * \class EllipsoidInteriorExteriorSpatialFunction
 * \brief Function implementation of an ellipsoid
 *
 * Implements a function that returns 1 for points inside or on the
 * surface of a ellipsoid and 0 for points outside the ellipsoid. The
 * orientation of the  n-dimensional ellipsoid axes are defined by n
 * orthogonal vectors. See
 * Examples/EllipsoidInteriorExteriorSpatialFunction/README for an
 * example of creating an Ellipsoid in an image.
 * \ingroup ITKCommon
 */
template <unsigned int VDimension = 3, typename TInput = Point<double, VDimension>>
class ITK_TEMPLATE_EXPORT EllipsoidInteriorExteriorSpatialFunction
  : public InteriorExteriorSpatialFunction<VDimension, TInput>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(EllipsoidInteriorExteriorSpatialFunction);

  /** Standard class type aliases. */
  using Self = EllipsoidInteriorExteriorSpatialFunction;
  using Superclass = InteriorExteriorSpatialFunction<VDimension, TInput>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(EllipsoidInteriorExteriorSpatialFunction);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Input type for the function */
  using typename Superclass::InputType;

  /** Output type for the function */
  using typename Superclass::OutputType;

  /** Typedef for the orientation matrix */
  using OrientationType = vnl_matrix_fixed<double, VDimension, VDimension>;

  /** Set/Get and set the center of the ellipsoid. */
  /** @ITKStartGrouping */
  itkGetConstMacro(Center, InputType);
  itkSetMacro(Center, InputType);
  /** @ITKEndGrouping */
  /** Get and set the axes lengths of the ellipsoid. */
  /** @ITKStartGrouping */
  itkGetConstMacro(Axes, InputType);
  itkSetMacro(Axes, InputType);
  /** @ITKEndGrouping */
  /** Set the orientation vectors (must be orthogonal) of the ellipsoid axes.
   * Must be normalized!!!!! */
  void
  SetOrientations(const OrientationType &);

  /** Evaluates the function at a given position. */
  OutputType
  Evaluate(const InputType & position) const override;

protected:
  EllipsoidInteriorExteriorSpatialFunction() = default;
  ~EllipsoidInteriorExteriorSpatialFunction() override = default;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

private:
  /** The center of the ellipsoid. */
  InputType m_Center{};

  /** The axes lengths of the ellipsoid. */
  InputType m_Axes{ MakeFilled<InputType>(1.0f) };

  /** The orientation vectors (must be orthogonal) of the ellipsoid axes. */
  OrientationType m_Orientations{};
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkEllipsoidInteriorExteriorSpatialFunction.hxx"
#endif

#endif

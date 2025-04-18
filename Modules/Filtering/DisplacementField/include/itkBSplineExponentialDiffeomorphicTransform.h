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
#ifndef itkBSplineExponentialDiffeomorphicTransform_h
#define itkBSplineExponentialDiffeomorphicTransform_h

#include "itkConstantVelocityFieldTransform.h"
#include "itkDisplacementFieldToBSplineImageFilter.h"
#include "itkPointSet.h"

namespace itk
{

/** \class BSplineExponentialDiffeomorphicTransform
 * \brief Exponential transform using B-splines as the smoothing kernel.
 *
 * Exponential transform inspired by the work of J. Ashburner (see reference
 * below).  Assuming a constant velocity field, the transform takes as input
 * the update field at time point t = 1, \f$u\f$ and smooths it using a B-spline
 * smoothing (i.e. fitting) operation, \f$S_{update}\f$ defined by \c SplineOrder and
 * \c NumberOfControlPointsForTheUpdateField.  We add that the current estimate
 * of the velocity field and then perform a second smoothing step such that
 * the new velocity field is
 *
 * \f{eqnarray*}{
 *   v_{new} = S_{velocity}( v_{old} + S_{update}( u ) ).
 * \f}
 *
 * We then exponentiate \f$v_{new}\f$ using the class
 * \c ExponentialDisplacementImageFilter to yield both the forward and inverse
 * displacement fields.
 *
 * See \cite ashburner2007 for more details.
 *
 * \author Nick Tustison
 * \author Brian Avants
 *
 * \ingroup ITKDisplacementField
 */
template <typename TParametersValueType, unsigned int VDimension>
class ITK_TEMPLATE_EXPORT BSplineExponentialDiffeomorphicTransform
  : public ConstantVelocityFieldTransform<TParametersValueType, VDimension>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BSplineExponentialDiffeomorphicTransform);

  /** Standard class type aliases. */
  using Self = BSplineExponentialDiffeomorphicTransform;
  using Superclass = ConstantVelocityFieldTransform<TParametersValueType, VDimension>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(BSplineExponentialDiffeomorphicTransform);

  /** New macro for creation of through a Smart Pointer */
  itkNewMacro(Self);

  /** Dimension of the velocity field . */
  static constexpr unsigned int ConstantVelocityFieldDimension = VDimension;

  /** Dimension of the vector spaces. */
  static constexpr unsigned int Dimension = VDimension;

  /** Types from superclass */
  using typename Superclass::ScalarType;
  using typename Superclass::DerivativeType;
  using DerivativeValueType = typename DerivativeType::ValueType;

  using typename Superclass::ParametersType;
  using typename Superclass::ParametersValueType;
  using typename Superclass::FixedParametersType;
  using typename Superclass::FixedParametersValueType;

  using typename Superclass::DisplacementFieldType;
  using typename Superclass::DisplacementFieldPointer;
  using typename Superclass::ConstantVelocityFieldType;
  using typename Superclass::ConstantVelocityFieldPointer;

  using DisplacementVectorType = typename DisplacementFieldType::PixelType;

  /**
   * type alias for projecting the input displacement field onto a
   * B-spline field.
   */
  using PointSetType = PointSet<ConstantVelocityFieldType, Dimension>;
  using SplineOrderType = unsigned int;
  using BSplineFilterType = DisplacementFieldToBSplineImageFilter<ConstantVelocityFieldType>;
  using WeightsContainerType = typename BSplineFilterType::WeightsContainerType;
  using ArrayType = typename BSplineFilterType::ArrayType;
  using ArrayValueType = typename ArrayType::ValueType;

  /**
   * Update the transform's parameters by the values in \c update. We overwrite the
   * base class implementation as we might want to smooth the update field before
   * adding it to the velocity field
   */
  void
  UpdateTransformParameters(const DerivativeType & update, ScalarType factor = 1.0) override;

  /**
   * Smooth the constant velocity field in-place.
   */
  virtual ConstantVelocityFieldPointer
  BSplineSmoothConstantVelocityField(const ConstantVelocityFieldType *, const ArrayType &);

  /**
   * Set/Get the spline order.
   */
  /** @ITKStartGrouping */
  itkSetMacro(SplineOrder, SplineOrderType);
  itkGetConstMacro(SplineOrder, SplineOrderType);
  /** @ITKEndGrouping */
  /**
   * Set/Get the control point grid size defining the B-spline estimate of the
   * smoothed velocity field.  In each dimension, the B-spline mesh size is equal
   * to the number of control points in that dimension minus the spline order.
   * Default = 4 control points in each dimension for a mesh size of 1 in each
   * dimension.
   */
  /** @ITKStartGrouping */
  itkSetMacro(NumberOfControlPointsForTheConstantVelocityField, ArrayType);
  itkGetConstMacro(NumberOfControlPointsForTheConstantVelocityField, ArrayType);
  /** @ITKEndGrouping */
  /**
   * Set the control point grid size defining the B-spline estimate of the
   * smoothed update field.  In each dimension, the B-spline mesh size is equal
   * to the number of control points in that dimension minus the spline order.
   * Default = 4 control points in each dimension for a mesh size of 1 in each
   * dimension.
   */
  /** @ITKStartGrouping */
  itkSetMacro(NumberOfControlPointsForTheUpdateField, ArrayType);
  itkGetConstMacro(NumberOfControlPointsForTheUpdateField, ArrayType);
  /** @ITKEndGrouping */
  /**
   * Set the update field mesh size which is used to specify the control point
   * grid size.  The mesh size in each dimension is calculated as the
   * difference between the control point grid size and the spline order, i.e.
   * meshSize = controlPointGridSize - SplineOrder.
   */
  void
  SetMeshSizeForTheConstantVelocityField(const ArrayType &);

  /**
   * Set the velocity field mesh size which is used to specify the control point
   * grid size.  The mesh size in each dimension is calculated as the
   * difference between the control point grid size and the spline order, i.e.
   * meshSize = controlPointGridSize - SplineOrder.
   */
  void
  SetMeshSizeForTheUpdateField(const ArrayType &);

protected:
  BSplineExponentialDiffeomorphicTransform();
  ~BSplineExponentialDiffeomorphicTransform() override = default;

  void
  PrintSelf(std::ostream &, Indent) const override;

private:
  ArrayType m_NumberOfControlPointsForTheConstantVelocityField{};
  ArrayType m_NumberOfControlPointsForTheUpdateField{};

  SplineOrderType m_SplineOrder{ 3 };
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBSplineExponentialDiffeomorphicTransform.hxx"
#endif

#endif // itkBSplineExponentialDiffeomorphicTransform_h

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
#ifndef itkScaleVersor3DTransform_h
#define itkScaleVersor3DTransform_h

#include "itkVersorRigid3DTransform.h"

namespace itk
{
/** \class ScaleVersor3DTransform
 *
 * \brief This transform applies a Versor rotation, translation and
 * anisotropic scale to the space.
 *
 * The transform can be described as:
 * \f$ (\textbf{R}_v + \textbf{S})\textbf{x} \f$ where \f$\textbf{R}_v\f$ is the
 * rotation matrix given the versor, and
 * \f$S=\left( \begin{array}{ccc}s_0-1 & 0 & 0 \\ 0 & s_1-1 & 0 \\ 0 & 0 & s_2-1 \end{array} \right) \f$
 *
 *
 * \note This transform's scale parameters are not related to the
 * uniform scaling parameter of the Similarity3DTransform.
 *
 * \author Johnson H.J., Harris G., Williams K. University of Iowa Carver
 * College of Medicine, Department of Psychiatry NeuroImaging Center
 *
 * This implementation was taken from the Insight Journal paper:
 * https://doi.org/10.54294/hmb052
 *
 * \ingroup ITKTransform
 */
template <typename TParametersValueType = double>
class ITK_TEMPLATE_EXPORT ScaleVersor3DTransform : public VersorRigid3DTransform<TParametersValueType>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ScaleVersor3DTransform);

  /** Standard class type aliases. */
  using Self = ScaleVersor3DTransform;
  using Superclass = VersorRigid3DTransform<TParametersValueType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ScaleVersor3DTransform);

  /** Dimension of parameters. */
  static constexpr unsigned int InputSpaceDimension = 3;
  static constexpr unsigned int OutputSpaceDimension = 3;
  static constexpr unsigned int ParametersDimension = 9;

  /** Parameters Type   */
  using typename Superclass::ParametersType;
  using typename Superclass::FixedParametersType;
  using typename Superclass::JacobianType;
  using typename Superclass::JacobianPositionType;
  using typename Superclass::InverseJacobianPositionType;
  using typename Superclass::ScalarType;
  using typename Superclass::InputPointType;
  using typename Superclass::OutputPointType;
  using typename Superclass::InputVectorType;
  using typename Superclass::OutputVectorType;
  using typename Superclass::InputVnlVectorType;
  using typename Superclass::OutputVnlVectorType;
  using typename Superclass::InputCovariantVectorType;
  using typename Superclass::OutputCovariantVectorType;
  using typename Superclass::MatrixType;
  using typename Superclass::InverseMatrixType;
  using typename Superclass::CenterType;
  using typename Superclass::OffsetType;
  using typename Superclass::TranslationType;

  using typename Superclass::VersorType;
  using typename Superclass::AxisType;
  using typename Superclass::AngleType;

  /** Scale Vector Type. */
  using ScaleVectorType = Vector<TParametersValueType, 3>;

  /** Directly set the matrix of the transform.
   *
   * Orthogonality testing is bypassed in this case.
   *
   * \sa MatrixOffsetTransformBase::SetMatrix() */
  /** @ITKStartGrouping */
  void
  SetMatrix(const MatrixType & matrix) override;
  void
  SetMatrix(const MatrixType & matrix, const TParametersValueType tolerance) override;
  /** @ITKEndGrouping */
  /** Set the transformation from a container of parameters
   * This is typically used by optimizers.
   * There are 9 parameters:
   *   0-2   versor
   *   3-5   translation
   *   6-8   Scale
   **  */
  void
  SetParameters(const ParametersType & parameters) override;

  const ParametersType &
  GetParameters() const override;

  /** Set/Get the scale vector. These scale factors are associated to the axis
   * of coordinates. */
  void
  SetScale(const ScaleVectorType & scale);

  itkGetConstReferenceMacro(Scale, ScaleVectorType);

  /** Set the internal parameters of the transform in order to represent an
   * Identity transform. */
  void
  SetIdentity() override;

  /** This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the
   * transform is invertible at this point. */
  void
  ComputeJacobianWithRespectToParameters(const InputPointType & p, JacobianType & jacobian) const override;

protected:
  ScaleVersor3DTransform();
#if !defined(ITK_LEGACY_REMOVE)
  [[deprecated("Removed unused constructor")]] ScaleVersor3DTransform(const MatrixType &       matrix,
                                                                      const OutputVectorType & offset);
#endif
  ScaleVersor3DTransform(unsigned int parametersDimension);
  ~ScaleVersor3DTransform() override = default;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  void
  SetVarScale(const ScaleVectorType & scale)
  {
    m_Scale = scale;
  }

  /** Compute the components of the rotation matrix in the superclass. */
  void
  ComputeMatrix() override;

  void
  ComputeMatrixParameters() override;

private:
  /**  Vector containing the scale. */
  ScaleVectorType m_Scale{};
}; // class ScaleVersor3DTransform
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkScaleVersor3DTransform.hxx"
#endif

#endif /* __ScaleVersor3DTransform_h */

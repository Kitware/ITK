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
#ifndef itkDemonsRegistrationFilter_hxx
#define itkDemonsRegistrationFilter_hxx

namespace itk
{

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::DemonsRegistrationFilter()

{
  auto drfp = DemonsRegistrationFunctionType::New();
  this->SetDifferenceFunction(static_cast<FiniteDifferenceFunctionType *>(drfp.GetPointer()));
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::PrintSelf(std::ostream & os,
                                                                                   Indent         indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfBooleanMacro(UseMovingImageGradient);
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::InitializeIteration()
{
  // call the superclass  implementation
  Superclass::InitializeIteration();

  // set the gradient selection flag
  auto * drfp = dynamic_cast<DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to DemonsRegistrationFunction");
  }

  drfp->SetUseMovingImageGradient(m_UseMovingImageGradient);

  // Smooth the deformation field
  if (this->GetSmoothDisplacementField())
  {
    this->SmoothDisplacementField();
  }
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
double
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::GetMetric() const
{
  auto * drfp = dynamic_cast<DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to DemonsRegistrationFunction");
  }

  return drfp->GetMetric();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
double
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::GetIntensityDifferenceThreshold() const
{
  auto * drfp = dynamic_cast<DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to DemonsRegistrationFunction");
  }

  return drfp->GetIntensityDifferenceThreshold();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::SetIntensityDifferenceThreshold(
  double threshold)
{
  auto * drfp = dynamic_cast<DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to DemonsRegistrationFunction");
  }

  drfp->SetIntensityDifferenceThreshold(threshold);
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
DemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::ApplyUpdate(const TimeStepType & dt)
{
  // If we smooth the update buffer before applying it, then the are
  // approximating a viscous problem as opposed to an elastic problem
  if (this->GetSmoothUpdateField())
  {
    this->SmoothUpdateField();
  }

  this->Superclass::ApplyUpdate(dt);

  auto * drfp = dynamic_cast<DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to DemonsRegistrationFunction");
  }

  this->SetRMSChange(drfp->GetRMSChange());
}
} // end namespace itk

#endif

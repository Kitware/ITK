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
#ifndef itkFastSymmetricForcesDemonsRegistrationFilter_hxx
#define itkFastSymmetricForcesDemonsRegistrationFilter_hxx


namespace itk
{

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
  FastSymmetricForcesDemonsRegistrationFilter()
  : m_Multiplier(MultiplyByConstantType::New())
  , m_Adder(AdderType::New())
{
  auto drfp = DemonsRegistrationFunctionType::New();
  this->SetDifferenceFunction(drfp);
  m_Multiplier->InPlaceOn();
  m_Adder->InPlaceOn();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::InitializeIteration()
{
  // update variables in the equation object
  DemonsRegistrationFunctionType * f = this->DownCastDifferenceFunctionType();

  f->SetDisplacementField(this->GetDisplacementField());

  // call the superclass  implementation ( initializes f )
  Superclass::InitializeIteration();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
double
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::GetMetric() const
{
  const DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  return drfp->GetMetric();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
double
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
  GetIntensityDifferenceThreshold() const
{
  const DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  return drfp->GetIntensityDifferenceThreshold();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
  SetIntensityDifferenceThreshold(double threshold)
{
  DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  drfp->SetIntensityDifferenceThreshold(threshold);
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
double
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::GetMaximumUpdateStepLength()
  const
{
  const DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  return drfp->GetMaximumUpdateStepLength();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::SetMaximumUpdateStepLength(
  double threshold)
{
  DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  drfp->SetMaximumUpdateStepLength(threshold);
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
const double &
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::GetRMSChange() const
{
  const DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  return drfp->GetRMSChange();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
auto
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::GetUseGradientType() const
  -> GradientType
{
  const DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  return drfp->GetUseGradientType();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::SetUseGradientType(
  GradientType gtype)
{
  DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  drfp->SetUseGradientType(gtype);
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
typename FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
  DemonsRegistrationFunctionType *
  FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
    DownCastDifferenceFunctionType()
{
  auto * drfp = dynamic_cast<DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to SymmetricDemonsRegistrationFunction");
  }

  return drfp;
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
const typename FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
  DemonsRegistrationFunctionType *
  FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::
    DownCastDifferenceFunctionType() const
{
  const auto * drfp = dynamic_cast<const DemonsRegistrationFunctionType *>(this->GetDifferenceFunction().GetPointer());

  if (!drfp)
  {
    itkExceptionMacro("Could not cast difference function to SymmetricDemonsRegistrationFunction");
  }

  return drfp;
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::AllocateUpdateBuffer()
{
  // The update buffer looks just like the output.
  const DisplacementFieldPointer output = this->GetOutput();
  const DisplacementFieldPointer upbuf = this->GetUpdateBuffer();

  upbuf->SetLargestPossibleRegion(output->GetLargestPossibleRegion());
  upbuf->SetRequestedRegion(output->GetRequestedRegion());
  upbuf->SetBufferedRegion(output->GetBufferedRegion());
  upbuf->SetOrigin(output->GetOrigin());
  upbuf->SetSpacing(output->GetSpacing());
  upbuf->SetDirection(output->GetDirection());
  upbuf->Allocate();
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::ApplyUpdate(
  const TimeStepType & dt)
{
  // If we smooth the update buffer before applying it, then the are
  // approximating a viscous problem as opposed to an elastic problem
  if (this->GetSmoothUpdateField())
  {
    this->SmoothUpdateField();
  }

  // use time step if necessary
  if (itk::Math::abs(dt - 1.0) > 1.0e-4)
  {
    itkDebugMacro("Using timestep: " << dt);
    m_Multiplier->SetInput2(dt);
    m_Multiplier->SetInput(this->GetUpdateBuffer());
    m_Multiplier->GraftOutput(this->GetUpdateBuffer());
    // in place update
    m_Multiplier->Update();
    // graft output back to this->GetUpdateBuffer()
    this->GetUpdateBuffer()->Graft(m_Multiplier->GetOutput());
  }

  m_Adder->SetInput1(this->GetOutput());
  m_Adder->SetInput2(this->GetUpdateBuffer());

  m_Adder->GetOutput()->SetRequestedRegion(this->GetOutput()->GetRequestedRegion());
  m_Adder->Update();

  // Region passing stuff
  this->GraftOutput(m_Adder->GetOutput());

  DemonsRegistrationFunctionType * drfp = this->DownCastDifferenceFunctionType();

  this->SetRMSChange(drfp->GetRMSChange());

  // Smooth the deformation field
  if (this->GetSmoothDisplacementField())
  {
    this->SmoothDisplacementField();
  }
}

template <typename TFixedImage, typename TMovingImage, typename TDisplacementField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage, TMovingImage, TDisplacementField>::PrintSelf(
  std::ostream & os,
  Indent         indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Intensity difference threshold: " << this->GetIntensityDifferenceThreshold() << std::endl;
}
} // end namespace itk

#endif

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
#ifndef itkPathToImageFilter_hxx
#define itkPathToImageFilter_hxx

#include "itkImageRegionIteratorWithIndex.h"
#include "itkPathIterator.h"
#include "itkNumericTraits.h"
#include "itkMath.h"

namespace itk
{

template <typename TInputPath, typename TOutputImage>
PathToImageFilter<TInputPath, TOutputImage>::PathToImageFilter()
{
  this->SetNumberOfRequiredInputs(1);
  m_Size.Fill(0);

  for (unsigned int i = 0; i < OutputImageDimension; ++i)
  {
    // Set an image spacing for the user
    m_Spacing[i] = 1.0;
    m_Origin[i] = 0;
  }

  m_PathValue = NumericTraits<ValueType>::OneValue();
  m_BackgroundValue = ValueType{};
}

/** Set the Input SpatialObject */
template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::SetInput(const InputPathType * input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<InputPathType *>(input));
}

/** Connect one of the operands  */
template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::SetInput(unsigned int index, const InputPathType * path)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index, const_cast<InputPathType *>(path));
}

/** Get the input Path */
template <typename TInputPath, typename TOutputImage>
auto
PathToImageFilter<TInputPath, TOutputImage>::GetInput() -> const InputPathType *
{
  return itkDynamicCastInDebugMode<const TInputPath *>(this->GetPrimaryInput());
}

/** Get the input Path */
template <typename TInputPath, typename TOutputImage>
auto
PathToImageFilter<TInputPath, TOutputImage>::GetInput(unsigned int idx) -> const InputPathType *
{
  return itkDynamicCastInDebugMode<const TInputPath *>(this->ProcessObject::GetInput(idx));
}

//----------------------------------------------------------------------------
template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::SetSpacing(const double * spacing)
{
  if (ContainerCopyWithCheck(m_Spacing, spacing, OutputImageDimension))
  {
    this->Modified();
  }
}

template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::SetSpacing(const float * spacing)
{
  if (ContainerCopyWithCheck(m_Spacing, spacing, OutputImageDimension))
  {
    this->Modified();
  }
}

template <typename TInputPath, typename TOutputImage>
const double *
PathToImageFilter<TInputPath, TOutputImage>::GetSpacing() const
{
  return m_Spacing;
}

template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::SetOrigin(const double * origin)
{
  if (ContainerCopyWithCheck(m_Origin, origin, OutputImageDimension))
  {
    this->Modified();
  }
}

template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::SetOrigin(const float * origin)
{
  if (ContainerCopyWithCheck(m_Origin, origin, OutputImageDimension))
  {
    this->Modified();
  }
}

template <typename TInputPath, typename TOutputImage>
const double *
PathToImageFilter<TInputPath, TOutputImage>::GetOrigin() const
{
  return m_Origin;
}

template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::GenerateData()
{
  itkDebugMacro("PathToImageFilter::GenerateData() called");

  // Get the input and output pointers
  const InputPathType *    InputPath = this->GetInput();
  const OutputImagePointer OutputImage = this->GetOutput();

  // Generate the image
  double   origin[OutputImageDimension];
  SizeType size;

  for (unsigned int i = 0; i < OutputImageDimension; ++i)
  {
    // Set Image size to the size of the path's bounding box
    // size[i] = (SizeValueType)
    // (InputObject->GetBoundingBox()->GetMaximum()[i]
    //                              -
    // InputObject->GetBoundingBox()->GetMinimum()[i]);
    size[i] = 0;
    origin[i] = 0;
  }

  typename OutputImageType::RegionType region;

  // If the size of the output has been explicitly specified, the filter
  // will set the output size to the explicit size, otherwise the size from the
  // spatial
  // paths's bounding box will be used as default.

  bool specified = false;
  for (unsigned int i = 0; i < OutputImageDimension; ++i)
  {
    if (m_Size[i] != 0)
    {
      specified = true;
      break;
    }
  }

  if (specified)
  {
    region.SetSize(m_Size);
  }
  else
  {
    itkExceptionMacro("Currently, the user MUST specify an image size");
    // region.SetSize( size );
  }
  region.SetIndex({ { 0 } });

  OutputImage->SetRegions(region); // set the region

  // If the spacing has been explicitly specified, the filter
  // will set the output spacing to that explicit spacing, otherwise the spacing
  // from
  // the spatial object is used as default.

  specified = false;
  for (unsigned int i = 0; i < OutputImageDimension; ++i)
  {
    if (m_Spacing[i] != 0.0)
    {
      specified = true;
      break;
    }
  }

  if (specified)
  {
    OutputImage->SetSpacing(this->m_Spacing); // set spacing
  }
  else
  {
    itkExceptionMacro("Currently, the user MUST specify an image spacing");
    // OutputImage->SetSpacing(InputObject->GetIndexToObjectTransform()->GetScaleComponent());
    //   // set spacing
  }
  OutputImage->SetOrigin(origin); //   and origin
  OutputImage->Allocate();        // allocate the image

  for (ImageRegionIteratorWithIndex<OutputImageType> imageIt(OutputImage, region); !imageIt.IsAtEnd(); ++imageIt)
  {
    imageIt.Set(m_BackgroundValue);
  }

  for (PathIterator<OutputImageType, InputPathType> pathIt(OutputImage, InputPath); !pathIt.IsAtEnd(); ++pathIt)
  {
    pathIt.Set(m_PathValue);
  }

  itkDebugMacro("PathToImageFilter::GenerateData() finished");
}

template <typename TInputPath, typename TOutputImage>
void
PathToImageFilter<TInputPath, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Size: " << static_cast<typename NumericTraits<SizeType>::PrintType>(m_Size) << std::endl;
  os << indent << "Spacing: " << m_Spacing << std::endl;
  os << indent << "Origin: " << m_Origin << std::endl;
  os << indent << "PathValue : " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_PathValue) << std::endl;
  os << indent << "BackgroundValue : " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_BackgroundValue)
     << std::endl;
}
} // end namespace itk

#endif

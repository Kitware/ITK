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
#ifndef itkDisplacementFieldToBSplineImageFilter_hxx
#define itkDisplacementFieldToBSplineImageFilter_hxx


#include "itkContinuousIndex.h"
#include "itkImportImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"

namespace itk
{

/*
 * DisplacementFieldToBSplineImageFilter class definitions
 */
template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::
  DisplacementFieldToBSplineImageFilter()
  : m_PointWeights(nullptr)

{
  // Input 0 DisplacementField optional
  Self::SetPrimaryInputName("DisplacementField");
  Self::RemoveRequiredInputName("DisplacementField");
  // Input 1 ConfidenceImage optional
  Self::AddOptionalInputName("ConfidenceImage", 1);
  // Input 2 PointSet optional
  Self::AddOptionalInputName("PointSet", 2);
  this->m_NumberOfFittingLevels.Fill(1);
  this->m_NumberOfControlPoints.Fill(4);
  this->m_BSplineDomainOrigin.Fill(0.0);
  this->m_BSplineDomainSpacing.Fill(1.0);
  this->m_BSplineDomainSize.Fill(0);
  this->m_BSplineDomainDirection.SetIdentity();
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::SetBSplineDomain(
  OriginType    origin,
  SpacingType   spacing,
  SizeType      size,
  DirectionType direction)
{
  if (this->m_BSplineDomainOrigin != origin || this->m_BSplineDomainSpacing != spacing ||
      this->m_BSplineDomainSize != size || this->m_BSplineDomainDirection != direction)
  {
    this->m_BSplineDomainOrigin = origin;
    this->m_BSplineDomainSpacing = spacing;
    this->m_BSplineDomainSize = size;
    this->m_BSplineDomainDirection = direction;

    this->m_BSplineDomainIsDefined = true;
    this->m_UseInputFieldToDefineTheBSplineDomain = false;
    this->Modified();
  }
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::SetBSplineDomainFromImage(
  RealImageType * image)
{
  this->SetBSplineDomain(
    image->GetOrigin(), image->GetSpacing(), image->GetRequestedRegion().GetSize(), image->GetDirection());
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::SetBSplineDomainFromImage(
  InputFieldType * field)
{
  this->SetBSplineDomain(
    field->GetOrigin(), field->GetSpacing(), field->GetRequestedRegion().GetSize(), field->GetDirection());
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::SetPointSetConfidenceWeights(
  WeightsContainerType * weights)
{
  this->m_PointWeights = weights;
  this->m_UsePointWeights = true;
  this->Modified();
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::VerifyPreconditions() const
{
  Superclass::VerifyPreconditions();

  if (this->GetInput() == nullptr && this->GetPointSet() == nullptr)
  {
    itkExceptionMacro("Either a DisplacementField or PointSet input must be specified as Input.");
  }

  const InputPointSetType * inputPointSet = this->GetPointSet();
  if (inputPointSet && this->m_UsePointWeights && (this->m_PointWeights->Size() != inputPointSet->GetNumberOfPoints()))
  {
    itkExceptionMacro("The number of input points does not match the number of weight elements.");
  }


  if (!this->m_UseInputFieldToDefineTheBSplineDomain && !this->m_BSplineDomainIsDefined)
  {
    itkExceptionMacro("Output (B-spline) domain is undefined.");
  }
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::GenerateData()
{
  const InputFieldType * inputField = this->GetInput();

  if (inputField && this->m_UseInputFieldToDefineTheBSplineDomain)
  {
    this->m_BSplineDomainOrigin = inputField->GetOrigin();
    this->m_BSplineDomainSpacing = inputField->GetSpacing();
    this->m_BSplineDomainSize = inputField->GetRequestedRegion().GetSize();
    this->m_BSplineDomainDirection = inputField->GetDirection();

    this->m_BSplineDomainIsDefined = true;
  }

  const RealImageType * confidenceImage = this->GetConfidenceImage();

  const InputPointSetType * inputPointSet = this->GetPointSet();

  auto fieldPoints = InputPointSetType::New();

  auto weights = WeightsContainerType::New();

  IdentifierType numberOfPoints{};

  const typename WeightsContainerType::Element boundaryWeight = 1.0e10;

  using ContinuousIndexType = ContinuousIndex<typename InputFieldPointType::CoordinateType, ImageDimension>;

  // Create an output field based on the b-spline domain to determine boundary
  // points and whether or not specified points are inside or outside the domain.

  typename InputFieldType::DirectionType identity;
  identity.SetIdentity();

  auto bsplinePhysicalDomainField = OutputFieldType::New();
  bsplinePhysicalDomainField->SetOrigin(this->m_BSplineDomainOrigin);
  bsplinePhysicalDomainField->SetSpacing(this->m_BSplineDomainSpacing);
  bsplinePhysicalDomainField->SetRegions(this->m_BSplineDomainSize);
  bsplinePhysicalDomainField->SetDirection(this->m_BSplineDomainDirection);
  // bsplinePhysicalDomainField->Allocate();

  auto bsplineParametricDomainField = OutputFieldType::New();
  bsplineParametricDomainField->SetOrigin(this->m_BSplineDomainOrigin);
  bsplineParametricDomainField->SetSpacing(this->m_BSplineDomainSpacing);
  bsplineParametricDomainField->SetRegions(this->m_BSplineDomainSize);
  bsplineParametricDomainField->SetDirection(identity);
  bsplineParametricDomainField->Allocate();

  typename OutputFieldType::IndexType startIndex = bsplineParametricDomainField->GetBufferedRegion().GetIndex();

  // Add the boundary points here if we have a b-spline domain not defined by the
  // input field.  This more general case doesn't take advantage of the speed up
  // within the B-spline scattered data fitting filter when consecutive points
  // are arranged consecutively on the grid.

  if (this->m_EnforceStationaryBoundary && !this->m_UseInputFieldToDefineTheBSplineDomain)
  {
    ImageRegionConstIteratorWithIndex<OutputFieldType> ItB(bsplineParametricDomainField,
                                                           bsplineParametricDomainField->GetBufferedRegion());

    for (ItB.GoToBegin(); !ItB.IsAtEnd(); ++ItB)
    {
      typename OutputFieldType::IndexType index = ItB.GetIndex();

      bool isOnStationaryBoundary = false;
      for (unsigned int d = 0; d < ImageDimension; ++d)
      {
        if (index[d] == startIndex[d] || index[d] == startIndex[d] + static_cast<int>(this->m_BSplineDomainSize[d]) - 1)
        {
          isOnStationaryBoundary = true;
          break;
        }
      }
      if (isOnStationaryBoundary)
      {
        const VectorType                      data{};
        typename InputPointSetType::PointType point;

        bsplineParametricDomainField->TransformIndexToPhysicalPoint(index, point);

        fieldPoints->SetPoint(numberOfPoints, point);
        fieldPoints->SetPointData(numberOfPoints, data);
        weights->InsertElement(numberOfPoints, boundaryWeight);
        ++numberOfPoints;
      }
    }
  }

  if (inputField)
  {
    itkDebugMacro("Gathering information from the input displacement field. ");

    ImageRegionConstIteratorWithIndex<InputFieldType> It(inputField, inputField->GetBufferedRegion());

    itkDebugMacro("Extracting points from input displacement field.");

    for (It.GoToBegin(); !It.IsAtEnd(); ++It)
    {
      typename DisplacementFieldType::IndexType index = It.GetIndex();

      bool isOnStationaryBoundary = false;
      if (this->m_EnforceStationaryBoundary && this->m_UseInputFieldToDefineTheBSplineDomain)
      {
        for (unsigned int d = 0; d < ImageDimension; ++d)
        {
          if (index[d] == startIndex[d] ||
              index[d] == startIndex[d] + static_cast<int>(this->m_BSplineDomainSize[d]) - 1)
          {
            isOnStationaryBoundary = true;
            break;
          }
        }
      }

      if (confidenceImage && confidenceImage->GetPixel(index) <= 0.0 && !isOnStationaryBoundary)
      {
        continue;
      }

      typename WeightsContainerType::Element weight = 1.0;
      if (confidenceImage && confidenceImage->GetPixel(index) > 0.0)
      {
        weight = static_cast<typename WeightsContainerType::Element>(confidenceImage->GetPixel(index));
      }

      PointType                             parametricPoint;
      typename InputPointSetType::PointType physicalPoint;

      inputField->TransformIndexToPhysicalPoint(index, physicalPoint);
      const ContinuousIndexType cidx =
        bsplinePhysicalDomainField
          ->template TransformPhysicalPointToContinuousIndex<typename InputFieldPointType::CoordinateType>(
            physicalPoint);
      bsplineParametricDomainField->TransformContinuousIndexToPhysicalPoint(cidx, parametricPoint);

      bool isInside = true;

      VectorType data = It.Get();

      if (isOnStationaryBoundary)
      {
        data.Fill(0.0);
        weight = boundaryWeight;
      }
      else if (this->m_EstimateInverse || !this->m_UseInputFieldToDefineTheBSplineDomain)
      {
        if (this->m_EstimateInverse)
        {
          for (unsigned int d = 0; d < ImageDimension; ++d)
          {
            physicalPoint[d] += data[d];
          }
          data *= -1.0;
        }

        InputFieldPointType imagePoint;
        imagePoint.CastFrom(physicalPoint);

        ContinuousIndexType cidx2;
        isInside = bsplinePhysicalDomainField->TransformPhysicalPointToContinuousIndex(imagePoint, cidx2);
        if (isInside)
        {
          bsplineParametricDomainField->TransformContinuousIndexToPhysicalPoint(cidx2, parametricPoint);
        }
      }

      if (isInside)
      {
        fieldPoints->SetPoint(numberOfPoints, parametricPoint);
        fieldPoints->SetPointData(numberOfPoints, data);
        weights->InsertElement(numberOfPoints, weight);
        ++numberOfPoints;
      }
    }
  }

  if (inputPointSet)
  {
    itkDebugMacro("Gathering information from the input point set. ");

    typename PointsContainerType::ConstIterator    ItP = inputPointSet->GetPoints()->Begin();
    typename PointDataContainerType::ConstIterator ItD = inputPointSet->GetPointData()->Begin();

    while (ItP != inputPointSet->GetPoints()->End())
    {

      PointType parametricPoint;

      PointType  physicalPoint = ItP.Value();
      VectorType data = ItD.Value();

      typename WeightsContainerType::Element weight = 1.0;
      if (this->m_UsePointWeights)
      {
        weight = this->m_PointWeights->GetElement(ItP.Index());
      }

      bool isInside = true;

      if (this->m_EstimateInverse)
      {
        for (unsigned int d = 0; d < ImageDimension; ++d)
        {
          physicalPoint[d] += data[d];
        }
        data *= -1.0;
      }

      InputFieldPointType imagePoint;
      imagePoint.CastFrom(physicalPoint);

      ContinuousIndexType cidx;
      isInside = bsplinePhysicalDomainField->TransformPhysicalPointToContinuousIndex(imagePoint, cidx);

      if (isInside && this->m_EnforceStationaryBoundary)
      {
        // If we enforce the stationary and the point is on the boundary (or really close
        // to the boundary), we can ignore it.
        for (unsigned int d = 0; d < ImageDimension; ++d)
        {
          if (cidx[d] < static_cast<typename ContinuousIndexType::CoordinateType>(startIndex[d]) + 0.5 ||
              cidx[d] > static_cast<typename ContinuousIndexType::CoordinateType>(
                          startIndex[d] + static_cast<int>(this->m_BSplineDomainSize[d]) - 1) -
                          0.5)
          {
            isInside = false;
            break;
          }
        }
      }

      if (isInside)
      {
        bsplineParametricDomainField->TransformContinuousIndexToPhysicalPoint(cidx, parametricPoint);

        fieldPoints->SetPoint(numberOfPoints, parametricPoint);
        fieldPoints->SetPointData(numberOfPoints, data);
        weights->InsertElement(numberOfPoints, weight);
        ++numberOfPoints;
      }

      ++ItP;
      ++ItD;
    }
  }

  if (numberOfPoints == 0)
  {
    itkExceptionMacro("No points were found.  Check that one or both inputs (displacement field/point set) are set.");
  }

  itkDebugMacro("Calculating the B-spline displacement field. ");

  constexpr auto close = MakeFilled<ArrayType>(false);

  auto bspliner = BSplineFilterType::New();
  bspliner->SetOrigin(this->m_BSplineDomainOrigin);
  bspliner->SetSpacing(this->m_BSplineDomainSpacing);
  bspliner->SetSize(this->m_BSplineDomainSize);
  bspliner->SetDirection(this->m_BSplineDomainDirection);
  bspliner->SetNumberOfLevels(this->m_NumberOfFittingLevels);
  bspliner->SetSplineOrder(this->m_SplineOrder);
  bspliner->SetNumberOfControlPoints(this->m_NumberOfControlPoints);
  bspliner->SetCloseDimension(close);
  bspliner->SetInput(fieldPoints);
  bspliner->SetPointWeights(weights);
  bspliner->SetGenerateOutputImage(true);

  bspliner->GraftOutput(this->GetOutput());
  bspliner->Update();
  this->GraftOutput(bspliner->GetOutput());
}

template <typename TInputImage, typename TInputPointSet, typename TOutputImage>
void
DisplacementFieldToBSplineImageFilter<TInputImage, TInputPointSet, TOutputImage>::PrintSelf(std::ostream & os,
                                                                                            Indent         indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfBooleanMacro(EstimateInverse);
  itkPrintSelfBooleanMacro(EnforceStationaryBoundary);
  os << indent << "NumberOfControlPoints: " << m_NumberOfControlPoints << std::endl;
  os << indent << "NumberOfFittingLevels: " << m_NumberOfFittingLevels << std::endl;

  itkPrintSelfObjectMacro(PointWeights);

  itkPrintSelfBooleanMacro(UsePointWeights);

  os << indent
     << "BSplineDomainOrigin: " << static_cast<typename NumericTraits<OriginType>::PrintType>(m_BSplineDomainOrigin)
     << std::endl;
  os << indent
     << "BSplineDomainSpacing: " << static_cast<typename NumericTraits<SpacingType>::PrintType>(m_BSplineDomainSpacing)
     << std::endl;
  os << indent << "BSplineDomainSize: " << static_cast<typename NumericTraits<SizeType>::PrintType>(m_BSplineDomainSize)
     << std::endl;
  os << indent << "BSplineDomainDirection: "
     << static_cast<typename NumericTraits<DirectionType>::PrintType>(m_BSplineDomainDirection) << std::endl;

  itkPrintSelfBooleanMacro(BSplineDomainIsDefined);
  itkPrintSelfBooleanMacro(UseInputFieldToDefineTheBSplineDomain);
}

} // end namespace itk

#endif

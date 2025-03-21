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

#include <iostream>
#include "itkSpatialObjectToImageStatisticsCalculator.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkEllipseSpatialObject.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkTestingMacros.h"

int
itkSpatialObjectToImageStatisticsCalculatorTest(int, char *[])
{
  using PixelType = unsigned char;
  using ImageType = itk::Image<PixelType, 2>;
  using EllipseType = itk::EllipseSpatialObject<2>;

  // Image Definition
  auto size = ImageType::SizeType::Filled(50);
  auto spacing = itk::MakeFilled<ImageType::SpacingType>(1);

  // Circle definition
  auto ellipse = EllipseType::New();
  ellipse->SetRadiusInObjectSpace(10);
  ellipse->Update();

  auto offset = itk::MakeFilled<EllipseType::VectorType>(25);
  ellipse->GetModifiableObjectToParentTransform()->SetOffset(offset);
  ellipse->Update();


  // Create a test image
  using ImageFilterType = itk::SpatialObjectToImageFilter<EllipseType, ImageType>;
  auto filter = ImageFilterType::New();
  filter->SetInput(ellipse);
  filter->SetSize(size);
  filter->SetSpacing(spacing);
  filter->SetInsideValue(255);
  filter->Update();

  const ImageType::Pointer image = filter->GetOutput();

  offset.Fill(25);
  ellipse->GetModifiableObjectToParentTransform()->SetOffset(offset);
  ellipse->Update();

  using CalculatorType = itk::SpatialObjectToImageStatisticsCalculator<ImageType, EllipseType>;
  auto calculator = CalculatorType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(calculator, SpatialObjectToImageStatisticsCalculator, Object);


  calculator->SetImage(image);
  calculator->SetSpatialObject(ellipse);

  constexpr unsigned int sampleDirection = CalculatorType::SampleDimension - 1;
  calculator->SetSampleDirection(sampleDirection);
  ITK_TEST_SET_GET_VALUE(sampleDirection, calculator->GetSampleDirection());

  calculator->Update();

  std::cout << " --- Ellipse and Image perfectly aligned --- " << std::endl;
  std::cout << "Sample mean = " << calculator->GetMean() << std::endl;
  std::cout << "Sample covariance = " << calculator->GetCovarianceMatrix();

  if (calculator->GetMean() != itk::MakeFilled<CalculatorType::VectorType>(255) ||
      itk::Math::NotAlmostEquals(calculator->GetCovarianceMatrix()[0][0], CalculatorType::MatrixType::ValueType{}))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[PASSED]" << std::endl;

  offset.Fill(20);
  ellipse->GetModifiableObjectToParentTransform()->SetOffset(offset);
  ellipse->Update();
  calculator->Update();

  std::cout << " --- Ellipse and Image mismatched left --- " << std::endl;
  std::cout << "Sample mean = " << calculator->GetMean() << std::endl;
  std::cout << "Sample covariance = " << calculator->GetCovarianceMatrix();

  if ((itk::Math::abs(calculator->GetMean()[0] - 140.0) > 1.0) ||
      (itk::Math::abs(calculator->GetCovarianceMatrix()[0][0] - 16141.0) > 1.0))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[PASSED]" << std::endl;

  std::cout << " --- Ellipse and Image mismatched right --- " << std::endl;

  offset.Fill(30);
  ellipse->GetModifiableObjectToParentTransform()->SetOffset(offset);
  ellipse->ComputeObjectToParentTransform();
  ellipse->Update();
  calculator->Update();

  std::cout << "Sample mean = " << calculator->GetMean() << std::endl;
  std::cout << "Sample covariance = " << calculator->GetCovarianceMatrix();

  if ((itk::Math::abs(calculator->GetMean()[0] - 140.0) > 1.0) ||
      (itk::Math::abs(calculator->GetCovarianceMatrix()[0][0] - 16141.0) > 1.0))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[PASSED]" << std::endl;

  std::cout << " --- Testing higher dimensionality --- " << std::endl;

  // Create a new 3D image
  using Image3DType = itk::Image<PixelType, 3>;
  auto image3D = Image3DType::New();

  using RegionType = Image3DType::RegionType;
  using SizeType = Image3DType::SizeType;
  using IndexType = Image3DType::IndexType;

  SizeType size3D;
  size3D[0] = 50;
  size3D[1] = 50;
  size3D[2] = 3;
  constexpr IndexType start{};
  RegionType          region3D;
  region3D.SetIndex(start);
  region3D.SetSize(size3D);
  image3D->SetRegions(region3D);
  image3D->Allocate();
  image3D->FillBuffer(255);

  // Fill the image
  std::cout << "Allocating image." << std::endl;
  using SliceIteratorType = itk::ImageSliceIteratorWithIndex<Image3DType>;
  SliceIteratorType it(image3D, region3D);

  it.SetFirstDirection(0);  // 0=x, 1=y, 2=z
  it.SetSecondDirection(1); // 0=x, 1=y, 2=z

  unsigned int value = 0;
  while (!it.IsAtEnd())
  {
    while (!it.IsAtEndOfSlice())
    {
      while (!it.IsAtEndOfLine())
      {
        it.Set(value);
        ++it;
      }
      it.NextLine();
    }
    it.NextSlice();
    value++;
  }

  std::cout << "Allocating spatial object." << std::endl;
  using Ellipse3DType = itk::EllipseSpatialObject<3>;
  auto   ellipse3D = Ellipse3DType::New();
  double radii[3];
  radii[0] = 10;
  radii[1] = 10;
  radii[2] = 0;
  ellipse3D->SetRadiusInObjectSpace(radii);

  auto offset3D = itk::MakeFilled<Ellipse3DType::VectorType>(25);
  offset3D[2] = 0; // first slice
  ellipse3D->GetModifiableObjectToParentTransform()->SetOffset(offset3D);
  ellipse3D->Update();

  // Create a new calculator with a sample size of 3
  std::cout << "Updating calculator." << std::endl;
  using Calculator3DType = itk::SpatialObjectToImageStatisticsCalculator<Image3DType, Ellipse3DType, 3>;
  auto calculator3D = Calculator3DType::New();
  calculator3D->SetImage(image3D);
  calculator3D->SetSpatialObject(ellipse3D);
  calculator3D->Update();

  std::cout << "Sample mean = " << calculator3D->GetMean() << std::endl;
  std::cout << "Sample covariance = " << calculator3D->GetCovarianceMatrix();

  if ((itk::Math::abs(calculator3D->GetMean()[0] - 0.0) > 1.0) ||
      (itk::Math::abs(calculator3D->GetMean()[1] - 1.0) > 1.0) ||
      (itk::Math::abs(calculator3D->GetMean()[2] - 2.0) > 1.0))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Number of pixels = " << calculator3D->GetNumberOfPixels() << std::endl;
  if (calculator3D->GetNumberOfPixels() != 305)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Sum = " << calculator3D->GetSum() << std::endl;
  if (calculator3D->GetSum() != 915)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "Test finished" << std::endl;
  return EXIT_SUCCESS;
}

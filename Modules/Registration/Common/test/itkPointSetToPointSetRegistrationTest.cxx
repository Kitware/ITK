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

#include "itkTranslationTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkPointSetToImageFilter.h"
#include "itkTestingMacros.h"

#include <iostream>

/**
 *
 *  This program tests the registration of a PointSet against an other PointSet.
 *
 */

int
itkPointSetToPointSetRegistrationTest(int, char *[])
{
  constexpr unsigned int PointSetDimension = 2;

  using PointSetPointType = float;

  // Fixed Point Set
  using FixedPointSetType = itk::PointSet<PointSetPointType, PointSetDimension>;
  auto fixedPointSet = FixedPointSetType::New();

  constexpr unsigned int numberOfPoints = 500;

  fixedPointSet->SetPointData(FixedPointSetType::PointDataContainer::New());

  fixedPointSet->GetPoints()->Reserve(numberOfPoints);
  fixedPointSet->GetPointData()->Reserve(numberOfPoints);

  FixedPointSetType::PointType point;

  unsigned int id = 0;
  for (unsigned int i = 0; i < numberOfPoints / 2; ++i)
  {
    point[0] = 0;
    point[1] = i;
    fixedPointSet->SetPoint(id++, point);
  }
  for (unsigned int i = 0; i < numberOfPoints / 2; ++i)
  {
    point[0] = i;
    point[1] = 0;
    fixedPointSet->SetPoint(id++, point);
  }

  // Moving Point Set
  using MovingPointSetType = itk::PointSet<PointSetPointType, PointSetDimension>;
  auto movingPointSet = MovingPointSetType::New();

  movingPointSet->SetPointData(MovingPointSetType::PointDataContainer::New());

  movingPointSet->GetPoints()->Reserve(numberOfPoints);
  movingPointSet->GetPointData()->Reserve(numberOfPoints);

  id = 0;
  for (unsigned int i = 0; i < numberOfPoints / 2; ++i)
  {
    point[0] = 0;
    point[1] = i;
    movingPointSet->SetPoint(id++, point);
  }
  for (unsigned int i = 0; i < numberOfPoints / 2; ++i)
  {
    point[0] = i;
    point[1] = 0;
    movingPointSet->SetPoint(id++, point);
  }

  // Set up the Metric
  using MetricType = itk::EuclideanDistancePointMetric<FixedPointSetType, MovingPointSetType>;

  using TransformBaseType = MetricType::TransformType;
  using ParametersType = TransformBaseType::ParametersType;

  auto metric = MetricType::New();

  // Set up the Transform
  using TransformType = itk::TranslationTransform<double, PointSetDimension>;
  auto transform = TransformType::New();

  // Set up the Optimizer
  using OptimizerType = itk::LevenbergMarquardtOptimizer;
  auto optimizer = OptimizerType::New();

  optimizer->SetUseCostFunctionGradient(false);

  // Set up the Registration method
  using RegistrationType = itk::PointSetToPointSetRegistrationMethod<FixedPointSetType, MovingPointSetType>;
  auto registration = RegistrationType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(registration, PointSetToPointSetRegistrationMethod, ProcessObject);

  // Scale the translation components of the Transform in the Optimizer
  OptimizerType::ScalesType scales(transform->GetNumberOfParameters());
  scales.Fill(1.0);

  constexpr unsigned long numberOfIterations = 100;
  constexpr double        gradientTolerance = 1e-1; // convergence criterion
  constexpr double        valueTolerance = 1e-1;    // convergence criterion
  constexpr double        epsilonFunction = 1e-9;   // convergence criterion

  optimizer->SetScales(scales);
  optimizer->SetNumberOfIterations(numberOfIterations);
  optimizer->SetValueTolerance(valueTolerance);
  optimizer->SetGradientTolerance(gradientTolerance);
  optimizer->SetEpsilonFunction(epsilonFunction);

  // Connect all the components required for the registration
  registration->SetMetric(metric);
  ITK_TEST_SET_GET_VALUE(metric, registration->GetMetric());

  registration->SetOptimizer(optimizer);
  ITK_TEST_SET_GET_VALUE(optimizer, registration->GetOptimizer());

  registration->SetTransform(transform);
  ITK_TEST_SET_GET_VALUE(transform, registration->GetTransform());

  registration->SetFixedPointSet(fixedPointSet);
  ITK_TEST_SET_GET_VALUE(fixedPointSet, registration->GetFixedPointSet());

  registration->SetMovingPointSet(movingPointSet);
  ITK_TEST_SET_GET_VALUE(movingPointSet, registration->GetMovingPointSet());

  // Set up transform parameters
  ParametersType parameters(transform->GetNumberOfParameters());

  // Initialize the offset/vector part
  for (double & parameter : parameters)
  {
    parameter = 10.0;
  }
  transform->SetParameters(parameters);
  registration->SetInitialTransformParameters(transform->GetParameters());
  ITK_TEST_SET_GET_VALUE(transform->GetParameters(), registration->GetInitialTransformParameters());


  ITK_TRY_EXPECT_NO_EXCEPTION(registration->Update());


  // Print the last transform parameters to improve coverage
  //
  ParametersType finalParameters = registration->GetLastTransformParameters();

  const unsigned int numberOfParameters = parameters.Size();

  std::cout << "Last Transform Parameters: " << std::endl;
  for (unsigned int i = 0; i < numberOfParameters; ++i)
  {
    std::cout << finalParameters[i] << std::endl;
  }

  std::cout << "Solution = " << transform->GetParameters() << std::endl;

  if ((itk::Math::abs(transform->GetParameters()[0]) > 1.0) || (itk::Math::abs(transform->GetParameters()[1]) > 1.0))
  {
    return EXIT_FAILURE;
  }

  //
  // Test with the Danielsson distance map.
  //

  constexpr unsigned int ImageDimension = 2;

  using BinaryImageType = itk::Image<unsigned char, ImageDimension>;
  using ImageType = itk::Image<unsigned short, ImageDimension>;

  using PSToImageFilterType = itk::PointSetToImageFilter<FixedPointSetType, BinaryImageType>;
  auto psToImageFilter = PSToImageFilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(psToImageFilter, PointSetToImageFilter, ImageSource);

  psToImageFilter->SetInput(fixedPointSet);

  double origin[2] = { 0.0, 0.0 };
  double spacing[2] = { 1.0, 1.0 };

  psToImageFilter->SetSpacing(spacing);
  psToImageFilter->SetOrigin(origin);

  std::cout << "Spacing and origin: [" << psToImageFilter->GetSpacing() << "], ,[" << psToImageFilter->GetOrigin()
            << ']' << std::endl;


  ITK_TRY_EXPECT_NO_EXCEPTION(psToImageFilter->Update());

  const BinaryImageType::Pointer binaryImage = psToImageFilter->GetOutput();

  using DDFilterType = itk::DanielssonDistanceMapImageFilter<BinaryImageType, ImageType>;
  auto ddFilter = DDFilterType::New();

  ddFilter->SetInput(binaryImage);

  ITK_TRY_EXPECT_NO_EXCEPTION(ddFilter->Update());


  const typename DDFilterType::OutputImageType::Pointer distanceMap = ddFilter->GetOutput();
  metric->SetDistanceMap(distanceMap);
  ITK_TEST_SET_GET_VALUE(distanceMap, metric->GetDistanceMap());

  metric->ComputeSquaredDistanceOn();

  // Initialize the offset/vector part
  for (unsigned int k = 0; k < PointSetDimension; ++k)
  {
    parameters[k] = 10.0;
  }

  transform->SetParameters(parameters);
  registration->SetInitialTransformParameters(transform->GetParameters());

  ITK_TRY_EXPECT_NO_EXCEPTION(registration->Update());

  std::cout << "Solution = " << transform->GetParameters() << std::endl;

  if ((itk::Math::abs(transform->GetParameters()[0]) > 1.0) || (itk::Math::abs(transform->GetParameters()[1]) > 1.0))
  {
    return EXIT_FAILURE;
  }

  std::cout << "TEST DONE" << std::endl;

  return EXIT_SUCCESS;
}

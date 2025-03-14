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

#include "itkCumulativeGaussianOptimizer.h"
#include "itkMath.h"
#include "itkTestingMacros.h"

#include <iostream>

/**
 * Generate test data with the Cumulative Gaussian Cost Function
 * given parameter values for mean, standard deviation,
 * lower and upper asymptotes of a Cumulative Gaussian.
 * Estimate the parameters of the test data with the
 * Cumulative Gaussian optimizer. The solution should
 * be within differenceTolerance of the fitError.
 */

int
itkCumulativeGaussianOptimizerTest(int, char *[])
{
  constexpr double mean = 3;                    // Mean of the Cumulative Gaussian.
                                                // Ranges from 0 to N-1, where N is numberOfSamples.
  constexpr double standardDeviation = 2;       // Standard deviation of the Cumulative Gaussian.
  constexpr double lowerAsymptote = -10;        // Lower asymptotic value of the Cumulative Gaussian.
  constexpr int    numberOfSamples = 9;         // Number of data samples.
  constexpr double upperAsymptote = 10;         // Upper asymptotic value of the Cumulative Gaussian.
  constexpr double differenceTolerance = 1e-20; // Tolerance allowed for the difference between Gaussian iterations.

  // Typedef and initialization for the Cumulative Gaussian Optimizer.
  using CumulativeGaussianOptimizerType = itk::CumulativeGaussianOptimizer;
  auto optimizer = CumulativeGaussianOptimizerType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(optimizer, CumulativeGaussianOptimizer, MultipleValuedNonLinearOptimizer);


  // Typedef and initialization for the Cumulative Gaussian Cost Function.
  using CostFunctionType = itk::CumulativeGaussianCostFunction;
  auto costFunction = CostFunctionType::New();

  // Declare and initialize the data array.
  // CostFunctionType::MeasureType * cumGaussianArray = new CostFunctionType::MeasureType();
  // cumGaussianArray->SetSize(numberOfSamples);

  // Set the parameters.
  CostFunctionType::ParametersType parameters;
  parameters.SetSize(4);
  parameters[0] = mean;
  parameters[1] = standardDeviation;
  parameters[2] = lowerAsymptote;
  parameters[3] = upperAsymptote;

  // Set the range of data sampled from a Cumulative Gaussian.
  costFunction->Initialize(numberOfSamples);

  // Generate data given a set of parameters.
  CostFunctionType::MeasureType * cumGaussianArray = costFunction->GetValuePointer(parameters);

  // Set the data array.
  costFunction->SetOriginalDataArray(cumGaussianArray);

  // Not used; empty method body; called for coverage purposes
  CostFunctionType::DerivativeType derivative;
  costFunction->GetDerivative(parameters, derivative);

  // Set the cost function.
  optimizer->SetCostFunction(costFunction);

  // Set the tolerance for the Gaussian iteration error.
  optimizer->SetDifferenceTolerance(differenceTolerance);
  ITK_TEST_SET_GET_VALUE(differenceTolerance, optimizer->GetDifferenceTolerance());

  // Print results after each iteration.
  constexpr bool verbose = true;
  ITK_TEST_SET_GET_BOOLEAN(optimizer, Verbose, verbose);

  // Set the data array.
  optimizer->SetDataArray(cumGaussianArray);

  // Start optimization;
  optimizer->StartOptimization();

  std::cout << "StopConditionDescription: " << optimizer->GetStopConditionDescription() << std::endl;

  // The test passes if the difference between the given parameters and estimated parameters
  // is less than or equal to 0.1.
  if (itk::Math::abs(optimizer->GetComputedMean() - mean) <= 0.1 &&
      itk::Math::abs(optimizer->GetComputedStandardDeviation() - standardDeviation) <= 0.1 &&
      itk::Math::abs(optimizer->GetUpperAsymptote() - upperAsymptote) <= 0.1 &&
      itk::Math::abs(optimizer->GetLowerAsymptote() - lowerAsymptote) <= 0.1)
  {
    std::cerr << std::endl << "Test Passed with a Fit Error of " << optimizer->GetFitError() << std::endl << std::endl;

    // Print out the resulting parameters.
    std::cerr << "Fitted mean = " << optimizer->GetComputedMean() << std::endl;
    std::cerr << "Fitted standard deviation = " << optimizer->GetComputedStandardDeviation() << std::endl;
    std::cerr << "Fitted upper intensity = " << optimizer->GetUpperAsymptote() << std::endl;
    std::cerr << "Fitted lower intensity = " << optimizer->GetLowerAsymptote() << std::endl;

    std::cerr << "FinalSampledArray: " << optimizer->GetFinalSampledArray() << std::endl;

    std::cout << "[TEST DONE]" << std::endl;
    return EXIT_SUCCESS;
  }

  std::cerr << std::endl << "Test Failed with a Fit Error of " << optimizer->GetFitError() << std::endl << std::endl;

  // Print out the resulting parameters.
  std::cerr << "Fitted mean = " << optimizer->GetComputedMean() << std::endl;
  std::cerr << "Fitted standard deviation = " << optimizer->GetComputedStandardDeviation() << std::endl;
  std::cerr << "Fitted upper asymptote = " << optimizer->GetUpperAsymptote() << std::endl;
  std::cerr << "Fitted lower asymptote = " << optimizer->GetLowerAsymptote() << std::endl;

  return EXIT_FAILURE;
}

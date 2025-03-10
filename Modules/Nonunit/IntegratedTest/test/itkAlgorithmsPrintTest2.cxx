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

#include "itkHistogram.h"
#include "itkPointSet.h"

#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMinMaxCurvatureFlowImageFilter.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiResolutionPDEDeformableRegistration.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkNormalizedCorrelationPointSetToImageMetric.h"
#include "itkOtsuThresholdCalculator.h"
#include "itkRGBGibbsPriorFilter.h"
#include "itkReinitializeLevelSetImageFilter.h"


int
main(int, char *[])
{
  using InputType = itk::Image<float, 2>;
  using OutputType = itk::Image<float, 2>;
  using UShortImageType3D = itk::Image<unsigned short, 3>;

  using VectorType = itk::Vector<float, 2>;
  using VectorImageType = itk::Image<VectorType, 2>;
  using VectorImageType3D = itk::Image<VectorType, 3>;

  // Used for NormalizedCorrelationPointSetToImageMetric
  using PointSetType = itk::PointSet<float, 2>;

  const itk::MattesMutualInformationImageToImageMetric<InputType, InputType>::Pointer
    MattesMutualInformationImageToImageMetricObj =
      itk::MattesMutualInformationImageToImageMetric<InputType, InputType>::New();
  std::cout << "-------------MattesMutualInformationImageToImageMetric "
            << MattesMutualInformationImageToImageMetricObj;

  /*itk::MeanSquaresPointSetToImageMetric<InputType,OutputType>::Pointer MeanSquaresPointSetToImageMetricObj =
    itk::MeanSquaresPointSetToImageMetric<InputType,OutputType>::New();
  std:: cout << "-------------MeanSquaresPointSetToImageMetric " << MeanSquaresPointSetToImageMetricObj;*/
  const itk::MeanSquaresImageToImageMetric<InputType, InputType>::Pointer MeanSquaresImageToImageMetricObj =
    itk::MeanSquaresImageToImageMetric<InputType, InputType>::New();
  std::cout << "-------------MeanSquaresImageToImageMetric " << MeanSquaresImageToImageMetricObj;
  const itk::MinMaxCurvatureFlowFunction<InputType>::Pointer MinMaxCurvatureFlowFunctionObj =
    itk::MinMaxCurvatureFlowFunction<InputType>::New();
  std::cout << "-------------MinMaxCurvatureFlowFunction " << MinMaxCurvatureFlowFunctionObj;

  const itk::MinMaxCurvatureFlowImageFilter<InputType, OutputType>::Pointer MinMaxCurvatureFlowImageFilterObj =
    itk::MinMaxCurvatureFlowImageFilter<InputType, OutputType>::New();
  std::cout << "-------------MinMaxCurvatureFlowImageFilter " << MinMaxCurvatureFlowImageFilterObj;

  const itk::MultiResolutionImageRegistrationMethod<InputType, InputType>::Pointer
    MultiResolutionImageRegistrationMethodObj =
      itk::MultiResolutionImageRegistrationMethod<InputType, InputType>::New();
  std::cout << "-------------MultiResolutionImageRegistrationMethod " << MultiResolutionImageRegistrationMethodObj;

  const itk::MultiResolutionPDEDeformableRegistration<InputType, OutputType, VectorImageType>::Pointer
    MultiResolutionPDEDeformableRegistrationObj =
      itk::MultiResolutionPDEDeformableRegistration<InputType, OutputType, VectorImageType>::New();
  std::cout << "-------------MultiResolutionPDEDeformableRegistration " << MultiResolutionPDEDeformableRegistrationObj;

  const itk::MultiResolutionPyramidImageFilter<InputType, OutputType>::Pointer MultiResolutionPyramidImageFilterObj =
    itk::MultiResolutionPyramidImageFilter<InputType, OutputType>::New();
  std::cout << "-------------MultiResolutionPyramidImageFilter " << MultiResolutionPyramidImageFilterObj;

  const itk::MutualInformationImageToImageMetric<InputType, InputType>::Pointer MutualInformationImageToImageMetricObj =
    itk::MutualInformationImageToImageMetric<InputType, InputType>::New();
  std::cout << "-------------MutualInformationImageToImageMetric " << MutualInformationImageToImageMetricObj;

  const itk::NormalizedCorrelationImageToImageMetric<InputType, InputType>::Pointer
    NormalizedCorrelationImageToImageMetricObj =
      itk::NormalizedCorrelationImageToImageMetric<InputType, InputType>::New();
  std::cout << "-------------NormalizedCorrelationImageToImageMetric " << NormalizedCorrelationImageToImageMetricObj;

  const itk::NormalizedCorrelationPointSetToImageMetric<PointSetType, InputType>::Pointer
    NormalizedCorrelationPointSetToImageMetricObj =
      itk::NormalizedCorrelationPointSetToImageMetric<PointSetType, InputType>::New();
  std::cout << "-------------NormalizedCorrelationPointSetToImageMetric "
            << NormalizedCorrelationPointSetToImageMetricObj;

  using HistogramType = itk::Statistics::Histogram<double>;
  const itk::OtsuThresholdCalculator<HistogramType>::Pointer OtsuThresholdCalculatorObj =
    itk::OtsuThresholdCalculator<HistogramType>::New();
  std::cout << "-------------OtsuThresholdCalculator " << OtsuThresholdCalculatorObj;

  const itk::PDEDeformableRegistrationFilter<InputType, InputType, VectorImageType>::Pointer
    PDEDeformableRegistrationFilterObj =
      itk::PDEDeformableRegistrationFilter<InputType, InputType, VectorImageType>::New();
  std::cout << "-------------PDEDeformableRegistrationFilter " << PDEDeformableRegistrationFilterObj;

  // NOTE:  RGBGibbsPriorFilter only works in 3D
  const itk::RGBGibbsPriorFilter<VectorImageType3D, UShortImageType3D>::Pointer RGBGibbsPriorFilterObj =
    itk::RGBGibbsPriorFilter<VectorImageType3D, UShortImageType3D>::New();
  std::cout << "-------------RGBGibbsPriorFilter " << RGBGibbsPriorFilterObj;

  const itk::RecursiveMultiResolutionPyramidImageFilter<InputType, OutputType>::Pointer
    RecursiveMultiResolutionPyramidImageFilterObj =
      itk::RecursiveMultiResolutionPyramidImageFilter<InputType, OutputType>::New();
  std::cout << "-------------RecursiveMultiResolutionPyramidImageFilter "
            << RecursiveMultiResolutionPyramidImageFilterObj;

  const itk::ReinitializeLevelSetImageFilter<InputType>::Pointer ReinitializeLevelSetImageFilterObj =
    itk::ReinitializeLevelSetImageFilter<InputType>::New();
  std::cout << "-------------ReinitializeLevelSetImageFilter " << ReinitializeLevelSetImageFilterObj;

  return EXIT_SUCCESS;
}

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

#include "itkCentralDifferenceImageFunction.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkTestingMacros.h"

template <unsigned int vecLength>
int
itkCentralDifferenceImageFunctionOnVectorSpeedTestRun(char * argv[])
{
  const int  imageSize = std::stoi(argv[1]);
  const int  reps = std::stoi(argv[2]);
  const bool doEAI = std::stoi(argv[3]);
  const bool doEACI = std::stoi(argv[4]);
  const bool doE = std::stoi(argv[5]);

  std::cout << "imageSize: " << imageSize << " reps: " << reps << " doEAI, doEACI, doE: " << doEAI << ", " << doEACI
            << ", " << doE << std::endl;
  std::cout << "vecLength: " << vecLength << std::endl;

  constexpr unsigned int ImageDimension = 2;
  using PixelType = itk::Vector<float, vecLength>;
  using ImageType = itk::Image<PixelType, ImageDimension>;

  auto                                 image = ImageType::New();
  auto                                 size = ImageType::SizeType::Filled(imageSize);
  const typename ImageType::RegionType region(size);

  image->SetRegions(region);
  image->Allocate();

  // make a test image
  using Iterator = itk::ImageRegionIteratorWithIndex<ImageType>;
  Iterator iter(image, region);
  iter.GoToBegin();
  unsigned int counter = 0;

  while (!iter.IsAtEnd())
  {
    PixelType pix;
    for (unsigned int n = 0; n < vecLength; ++n)
    {
      pix[n] = counter; //(n+1) + counter;
    }
    iter.Set(pix);
    ++counter;
    ++iter;
  }

  // set up central difference calculator
  using CoordinateType = float;
  using DerivativeType = itk::Matrix<double, vecLength, 2>;

  using FunctionType = itk::CentralDifferenceImageFunction<ImageType, CoordinateType, DerivativeType>;
  using OutputType = typename FunctionType::OutputType;

  auto function = FunctionType::New();

  function->SetInputImage(image);

  typename ImageType::IndexType index;

  OutputType indexOutput;
  OutputType total{};

  std::cout << "UseImageDirection: " << function->GetUseImageDirection() << std::endl;

  /// loop
  for (int l = 0; l < reps; ++l)
  {
    iter.GoToBegin();
    while (!iter.IsAtEnd())
    {
      index = iter.GetIndex();
      if (doEAI)
      {
        indexOutput = function->EvaluateAtIndex(index);
        total += indexOutput;
      }

      if (doEACI)
      {
        typename FunctionType::ContinuousIndexType cindex;
        cindex[0] = index[0] + 0.1;
        cindex[1] = index[1] + 0.1;
        const OutputType continuousIndexOutput = function->EvaluateAtContinuousIndex(cindex);
        total += continuousIndexOutput;
      }

      if (doE)
      {
        typename FunctionType::PointType point;
        image->TransformIndexToPhysicalPoint(index, point);
        const OutputType pointOutput = function->Evaluate(point);
        total += pointOutput;
      }

      ++iter;
    }
  }

  return EXIT_SUCCESS;
}


int
itkCentralDifferenceImageFunctionOnVectorSpeedTest(int argc, char * argv[])
{
  if (argc != 7)
  {
    std::cerr << "usage: " << itkNameOfTestExecutableMacro(argv) << " size reps doEAI doEACI doE vecLength"
              << std::endl;
    return EXIT_FAILURE;
  }
  const int vecLength = std::stoi(argv[6]);

  switch (vecLength)
  {
    case 1:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<1>(argv);
      break;
    case 2:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<2>(argv);
      break;
    case 3:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<3>(argv);
      break;
    case 4:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<4>(argv);
      break;
    case 5:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<5>(argv);
      break;
    case 6:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<6>(argv);
      break;
    case 7:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<7>(argv);
      break;
    case 8:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<8>(argv);
      break;
    case 9:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<9>(argv);
      break;
    case 10:
      itkCentralDifferenceImageFunctionOnVectorSpeedTestRun<10>(argv);
      break;
    default:
      std::cout << "Invalid vecLength" << std::endl;
      break;
  }
  return EXIT_SUCCESS;
}

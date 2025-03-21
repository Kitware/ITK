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


#include "itkJoinSeriesImageFilter.h"
#include "itkStreamingImageFilter.h"
#include "itkTestingMacros.h"

#include <iostream>

class ShowProgressObject
{
public:
  ShowProgressObject(itk::ProcessObject * o) { m_Process = o; }
  void
  ShowProgress()
  {
    std::cout << "Progress " << m_Process->GetProgress() << std::endl;
  }
  itk::ProcessObject::Pointer m_Process;
};

int
itkJoinSeriesImageFilterTest(int, char *[])
{

  constexpr unsigned int streamDivisions = 2;
  using PixelType = unsigned char;
  using InputImageType = itk::Image<PixelType, 2>;
  using OutputImageType = itk::Image<PixelType, 4>;

  // Expected result
  constexpr OutputImageType::IndexType expectedIndex = { { 1, 2, 0, 0 } };
  constexpr OutputImageType::SizeType  expectedSize = { { 8, 5, 4, 1 } };
  const OutputImageType::RegionType    expectedRegion{ expectedIndex, expectedSize };
  OutputImageType::SpacingType         expectedSpacing;
  expectedSpacing[0] = 1.1;
  expectedSpacing[1] = 1.2;
  expectedSpacing[2] = 1.3;
  expectedSpacing[3] = 1.0;
  OutputImageType::PointType expectedOrigin;
  expectedOrigin[0] = 0.1;
  expectedOrigin[1] = 0.2;
  expectedOrigin[2] = 0.3;
  expectedOrigin[3] = 0.0;

  // Create the input images
  constexpr int                       numInputs = 4;
  constexpr InputImageType::IndexType index = { { 1, 2 } };
  constexpr InputImageType::SizeType  size = { { 8, 5 } };
  const InputImageType::RegionType    region{ index, size };
  constexpr double                    spacingValue = 1.3;
  InputImageType::SpacingType         spacing;
  spacing[0] = 1.1;
  spacing[1] = 1.2;
  constexpr double          originValue = 0.3;
  InputImageType::PointType origin;
  origin[0] = 0.1;
  origin[1] = 0.2;

  std::vector<InputImageType::Pointer> inputs;

  PixelType counter1 = 0;
  for (int i = 0; i < numInputs; ++i)
  {
    inputs.push_back(InputImageType::New());
    inputs[i]->SetLargestPossibleRegion(region);
    inputs[i]->SetBufferedRegion(region);
    inputs[i]->Allocate();

    itk::ImageRegionIterator<InputImageType> inputIter(inputs[i], inputs[i]->GetBufferedRegion());
    while (!inputIter.IsAtEnd())
    {
      inputIter.Set(counter1);
      ++counter1;
      ++inputIter;
    }

    inputs[i]->SetSpacing(spacing);
    inputs[i]->SetOrigin(origin);
  }

  // Create the filter
  using JoinSeriesImageType = itk::JoinSeriesImageFilter<InputImageType, OutputImageType>;

  auto joinSeriesImage = JoinSeriesImageType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(joinSeriesImage, JoinSeriesImageFilter, ImageToImageFilter);

  // Check the default values
  if (joinSeriesImage->GetSpacing() != 1.0)
  {
    std::cout << "Default spacing is not 1.0" << std::endl;
    return EXIT_FAILURE;
  }
  if (joinSeriesImage->GetOrigin() != 0.0)
  {
    std::cout << "Default origin is not 0.0" << std::endl;
    return EXIT_FAILURE;
  }

  // Setup the filter
  joinSeriesImage->SetSpacing(spacingValue);
  ITK_TEST_SET_GET_VALUE(spacingValue, joinSeriesImage->GetSpacing());

  joinSeriesImage->SetOrigin(originValue);
  ITK_TEST_SET_GET_VALUE(originValue, joinSeriesImage->GetOrigin());

  for (int i = 0; i < numInputs; ++i)
  {
    joinSeriesImage->SetInput(i, inputs[i]);
  }

  // Test the ProgressReporter
  ShowProgressObject progressWatch(joinSeriesImage);
  using CommandType = itk::SimpleMemberCommand<ShowProgressObject>;
  auto command = CommandType::New();
  command->SetCallbackFunction(&progressWatch, &ShowProgressObject::ShowProgress);
  joinSeriesImage->AddObserver(itk::ProgressEvent(), command);

  // Test streaming
  using StreamingImageType = itk::StreamingImageFilter<OutputImageType, OutputImageType>;
  auto streamingImage = StreamingImageType::New();
  streamingImage->SetInput(joinSeriesImage->GetOutput());
  streamingImage->SetNumberOfStreamDivisions(streamDivisions);

  try
  {
    streamingImage->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << err << std::endl;
    const auto * const errp = dynamic_cast<const itk::DataObjectError * const>(&err);
    if (errp)
    {
      errp->GetDataObject()->Print(std::cout);
    }
    return EXIT_FAILURE;
  }

  const OutputImageType::Pointer output = streamingImage->GetOutput();


  // Check the information
  if (output->GetLargestPossibleRegion() != expectedRegion)
  {
    std::cout << "LargestPossibleRegion mismatch" << std::endl;
    return EXIT_FAILURE;
  }
  if (output->GetSpacing() != expectedSpacing)
  {
    std::cout << "Spacing mismatch" << std::endl;
    return EXIT_FAILURE;
  }
  if (output->GetOrigin() != expectedOrigin)
  {
    std::cout << "Origin mismatch" << std::endl;
    return EXIT_FAILURE;
  }

  // Check the contents
  bool passed = true;

  PixelType                                 counter2 = 0;
  itk::ImageRegionIterator<OutputImageType> outputIter(output, output->GetBufferedRegion());
  while (!outputIter.IsAtEnd())
  {
    if (outputIter.Get() != counter2)
    {
      passed = false;
      std::cout << "Mismatch at index: " << outputIter.GetIndex() << std::endl;
    }
    ++counter2;
    ++outputIter;
  }

  if (!passed || counter1 != counter2)
  {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }


  // An exception is raised when an input is missing.
  passed = false;

  // Set the 2nd input null
  joinSeriesImage->SetInput(1, nullptr);
  try
  {
    joinSeriesImage->Update();
  }
  catch (const itk::InvalidRequestedRegionError & err)
  {
    std::cout << err << std::endl;
    passed = true;
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << err << std::endl;
    return EXIT_FAILURE;
  }

  if (!passed)
  {
    std::cout << "Expected exception is missing" << std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "Test finished." << std::endl;
  return EXIT_SUCCESS;
}

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
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSimpleFilterWatcher.h"

#include "itkBinaryStatisticsOpeningImageFilter.h"

#include "itkTestingMacros.h"

int
itkBinaryStatisticsOpeningImageFilterTest1(int argc, char * argv[])
{

  if (argc != 10)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv);
    std::cerr << " input feature output";
    std::cerr << " foreground background lambda";
    std::cerr << "reverseOrdering connectivity attribute" << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int dim = 2;

  using IType = itk::Image<unsigned char, dim>;

  using ReaderType = itk::ImageFileReader<IType>;
  auto reader = ReaderType::New();
  reader->SetFileName(argv[1]);

  auto reader2 = ReaderType::New();
  reader2->SetFileName(argv[2]);

  using BinaryOpeningType = itk::BinaryStatisticsOpeningImageFilter<IType, IType>;
  auto opening = BinaryOpeningType::New();

  opening->SetInput(reader->GetOutput());
  opening->SetFeatureImage(reader2->GetOutput());

  // testing get/set ForegroundValue macro
  const int ForegroundValue = (std::stoi(argv[4]));
  opening->SetForegroundValue(ForegroundValue);
  ITK_TEST_SET_GET_VALUE(ForegroundValue, opening->GetForegroundValue());

  // testing get/set BackgroundValue macro
  const int BackgroundValue = (std::stoi(argv[5]));
  opening->SetBackgroundValue(BackgroundValue);
  ITK_TEST_SET_GET_VALUE(BackgroundValue, opening->GetBackgroundValue());

  // testing get and set macros for Lambda
  const double lambda = std::stod(argv[6]);
  opening->SetLambda(lambda);
  ITK_TEST_SET_GET_VALUE(lambda, opening->GetLambda());

  // testing boolean macro for ReverseOrdering
  opening->ReverseOrderingOn();
  ITK_TEST_SET_GET_VALUE(true, opening->GetReverseOrdering());

  opening->ReverseOrderingOff();
  ITK_TEST_SET_GET_VALUE(false, opening->GetReverseOrdering());

  // testing get and set macros or ReverseOrdering
  const bool reverseOrdering = std::stoi(argv[7]);
  opening->SetReverseOrdering(reverseOrdering);
  ITK_TEST_SET_GET_VALUE(reverseOrdering, opening->GetReverseOrdering());

  // testing boolean macro for FullyConnected
  opening->FullyConnectedOn();
  ITK_TEST_SET_GET_VALUE(true, opening->GetFullyConnected());

  opening->FullyConnectedOff();
  ITK_TEST_SET_GET_VALUE(false, opening->GetFullyConnected());

  // testing get and set macros or FullyConnected
  const bool fullyConnected = std::stoi(argv[8]);
  opening->SetFullyConnected(fullyConnected);
  ITK_TEST_SET_GET_VALUE(fullyConnected, opening->GetFullyConnected());

  // testing get and set macros for Attribute
  const BinaryOpeningType::AttributeType attribute = std::stoi(argv[9]);
  opening->SetAttribute(attribute);
  ITK_TEST_SET_GET_VALUE(attribute, opening->GetAttribute());

  const itk::SimpleFilterWatcher watcher(opening, "filter");

  using WriterType = itk::ImageFileWriter<IType>;
  auto writer = WriterType::New();
  writer->SetInput(opening->GetOutput());
  writer->SetFileName(argv[3]);
  writer->UseCompressionOn();

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());

  std::cout << "Test Complete!" << std::endl;

  return EXIT_SUCCESS;
}

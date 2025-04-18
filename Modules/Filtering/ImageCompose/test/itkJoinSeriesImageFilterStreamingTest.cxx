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
#include "itkJoinSeriesImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkPipelineMonitorImageFilter.h"
#include "itkTestingMacros.h"

int
itkJoinSeriesImageFilterStreamingTest(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " InputImage OutputImage" << std::endl;
    return EXIT_FAILURE;
  }

  using ImageType = itk::Image<unsigned char, 3>;
  using SliceImageType = itk::Image<unsigned char, 2>;

  using ImageFileReaderType = itk::ImageFileReader<ImageType>;
  using SliceExtractorFilterType = itk::ExtractImageFilter<ImageType, SliceImageType>;
  using JoinSeriesFilterType = itk::JoinSeriesImageFilter<SliceImageType, ImageType>;
  using ImageFileWriterType = itk::ImageFileWriter<ImageType>;

  const std::string inputFileName = argv[1];
  const std::string outputFileName = argv[2];

  auto reader = ImageFileReaderType::New();
  reader->SetFileName(inputFileName);
  reader->UpdateOutputInformation();


  const auto numberOfSlices =
    itk::Math::CastWithRangeCheck<unsigned int>(reader->GetOutput()->GetLargestPossibleRegion().GetSize(2));


  const itk::PipelineMonitorImageFilter<ImageType>::Pointer monitor1 =
    itk::PipelineMonitorImageFilter<ImageType>::New();
  monitor1->SetInput(reader->GetOutput());

  std::vector<itk::ProcessObject::Pointer> savedPointers;

  auto joinSeries = JoinSeriesFilterType::New();
  joinSeries->SetOrigin(reader->GetOutput()->GetOrigin()[2]);
  joinSeries->SetSpacing(reader->GetOutput()->GetSpacing()[2]);

  for (ImageType::SizeValueType z = 0; z < numberOfSlices; ++z)
  {

    auto extractor = SliceExtractorFilterType::New();
    extractor->SetDirectionCollapseToSubmatrix();

    SliceExtractorFilterType::InputImageRegionType slice(reader->GetOutput()->GetLargestPossibleRegion());
    slice.SetSize(2, 0);
    slice.SetIndex(2, z);

    extractor->SetExtractionRegion(slice);
    extractor->SetInput(monitor1->GetOutput());
    extractor->InPlaceOn();
    extractor->ReleaseDataFlagOn();

    savedPointers.emplace_back(extractor);

    joinSeries->PushBackInput(extractor->GetOutput());
  }


  const itk::PipelineMonitorImageFilter<ImageType>::Pointer monitor2 =
    itk::PipelineMonitorImageFilter<ImageType>::New();
  monitor2->SetInput(joinSeries->GetOutput());

  auto writer = ImageFileWriterType::New();
  writer->SetInput(monitor2->GetOutput());
  writer->SetFileName(outputFileName);
  writer->SetNumberOfStreamDivisions(numberOfSlices);

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());

  std::cout << "Number of Updates: " << monitor1->GetNumberOfUpdates() << std::endl;
  std::cout << "Verifying ImageFileReader to ExtractImageFilter pipeline interaction" << std::endl;

  // We can not use one of the standard verify all methods due to
  // multiple filters connected to the output of the reader
  if (!(monitor1->VerifyInputFilterExecutedStreaming(numberOfSlices) &&
        monitor1->VerifyInputFilterMatchedUpdateOutputInformation()))
  {
    std::cerr << monitor1;
    return EXIT_FAILURE;
  }

  std::cout << "Verifying JoinSeriesImageFilter to ImageFileWriter pipeline interaction" << std::endl;
  if (!monitor2->VerifyAllInputCanStream(numberOfSlices))
  {
    std::cerr << monitor2;
    return EXIT_FAILURE;
  }

  std::cout << "Test finished." << std::endl;
  return EXIT_SUCCESS;
}

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
// Disable warning for long symbol names in this file only

/*
 * This is a test file for the itkImageMaskSpatialObject class.
 * The supported pixel types does not include itkRGBPixel, itkRGBAPixel, etc...
 * So far it only allows to manage images of simple types like unsigned short,
 * unsigned int, or itk::Vector<...>.
 */


#include "itkImageRegionIterator.h"

#include "itkImageMaskSpatialObject.h"
#include "itkTestingMacros.h"


int
itkImageMaskSpatialObjectTest5(int, char *[])
{
  constexpr unsigned int VDimension = 3;

  using ImageMaskSpatialObject = itk::ImageMaskSpatialObject<VDimension>;
  using PixelType = ImageMaskSpatialObject::PixelType;
  using ImageType = ImageMaskSpatialObject::ImageType;
  using Iterator = itk::ImageRegionIterator<ImageType>;

  auto                           image = ImageType::New();
  constexpr ImageType::SizeType  size = { { 50, 50, 50 } };
  constexpr ImageType::IndexType index = { { 0, 0, 0 } };
  ImageType::RegionType          region;

  region.SetSize(size);
  region.SetIndex(index);

  image->SetRegions(region);
  image->AllocateInitialized();

  ImageType::RegionType          insideRegion;
  constexpr ImageType::SizeType  insideSize = { { 30, 30, 30 } };
  constexpr ImageType::IndexType insideIndex = { { 10, 10, 10 } };
  insideRegion.SetSize(insideSize);
  insideRegion.SetIndex(insideIndex);


  Iterator it(image, insideRegion);
  it.GoToBegin();

  while (!it.IsAtEnd())
  {
    it.Set(itk::NumericTraits<PixelType>::max());
    ++it;
  }

  auto maskSO = ImageMaskSpatialObject::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(maskSO, ImageMaskSpatialObject, ImageSpatialObject);


  maskSO->SetImage(image);
  maskSO->Update();

  Iterator itr(image, region);
  itr.GoToBegin();
  while (!itr.IsAtEnd())
  {
    const ImageType::IndexType constIndex = itr.GetIndex();
    const bool                 reference = insideRegion.IsInside(constIndex);
    ImageType::PointType       point;
    image->TransformIndexToPhysicalPoint(constIndex, point);
    const bool test = maskSO->IsInsideInWorldSpace(point);
    if (test != reference)
    {
      std::cerr << "Error in the evaluation of IsInside() - Part 1 " << std::endl;
      std::cerr << "Index failed = " << constIndex << std::endl;
      std::cerr << "   Mask " << test << " != " << reference << std::endl;
      return EXIT_FAILURE;
    }
    ++itr;
  }

  // Repeat test using a mask value
  maskSO->SetMaskValue(itk::NumericTraits<PixelType>::max());
  maskSO->SetUseMaskValue(true);
  itr.GoToBegin();
  while (!itr.IsAtEnd())
  {
    const ImageType::IndexType constIndex = itr.GetIndex();
    const bool                 reference = insideRegion.IsInside(constIndex);
    ImageType::PointType       point;
    image->TransformIndexToPhysicalPoint(constIndex, point);
    const bool test = maskSO->IsInsideInWorldSpace(point);
    if (test != reference)
    {
      std::cerr << "Error in the evaluation of IsInside() - Part 2" << std::endl;
      std::cerr << "Index failed = " << constIndex << std::endl;
      std::cerr << "   Mask " << test << " != " << reference << std::endl;
      return EXIT_FAILURE;
    }
    ++itr;
  }

  // The following should result in all IsInsideInWorldSpace calls
  // returning false
  maskSO->SetMaskValue(itk::NumericTraits<PixelType>::OneValue());
  maskSO->SetUseMaskValue(true);
  itr.GoToBegin();
  while (!itr.IsAtEnd())
  {
    const ImageType::IndexType constIndex = itr.GetIndex();
    const bool                 reference = insideRegion.IsInside(constIndex);
    ImageType::PointType       point;
    image->TransformIndexToPhysicalPoint(constIndex, point);
    const bool test = maskSO->IsInsideInWorldSpace(point);
    if (test == true)
    {
      std::cerr << "Error in the evaluation of IsInside() - Part 3" << std::endl;
      std::cerr << "Index failed = " << constIndex << std::endl;
      std::cerr << "   Mask " << test << " != " << reference << std::endl;
      return EXIT_FAILURE;
    }
    ++itr;
  }

  std::cout << "Test finished" << std::endl;
  return EXIT_SUCCESS;
}

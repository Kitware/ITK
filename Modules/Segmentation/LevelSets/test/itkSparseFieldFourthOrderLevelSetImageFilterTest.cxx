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

#include "itkSparseFieldFourthOrderLevelSetImageFilter.h"
#include "itkTestingMacros.h"
#include <iostream>

/*
 * This test exercises the SparseFieldFourthOrderLevelSetImageFilter
 * framework. A 2D image of a square is created and passed as input to the
 * filter which performs 500 iterations. This application will perform
 * isotropic fourth order diffusion on the input; therefore, the square will
 * morph towards a circle. The classes tested are the following:
 *
 * SparseImage
 * FiniteDifferenceSparseImageFilter
 * FiniteDifferenceSparseImageFunction
 * ImplicitManifoldNormalDiffusionFilter
 * NormalVectorFunctionBase
 * NormalVectorDiffusionFunction
 * LevelSetFunctionWithRefitTerm
 * SparseFieldFourthOrderLevelSetImageFilter
 *
 */

namespace SFFOLSIFT
{ // local namespace for helper functions

constexpr unsigned int HEIGHT = (128);
constexpr unsigned int WIDTH = (128);

#define RADIUS (std::min(HEIGHT, WIDTH) / 4)

// Distance transform function for square
float
square(unsigned int x, unsigned int y)
{
  const float X = itk::Math::abs(x - float{ WIDTH } / 2.0);
  const float Y = itk::Math::abs(y - float{ HEIGHT } / 2.0);
  float       dis = -std::sqrt((X - RADIUS) * (X - RADIUS) + (Y - RADIUS) * (Y - RADIUS));
  if (!((X > RADIUS) && (Y > RADIUS)))
  {
    dis = RADIUS - std::max(X, Y);
  }
  return (dis);
}

// Evaluates a function at each pixel in the itk image
void
evaluate_function(itk::Image<float, 2> * im, float (*f)(unsigned int, unsigned int))

{
  itk::Image<float, 2>::IndexType idx;
  for (unsigned int x = 0; x < WIDTH; ++x)
  {
    idx[0] = x;
    for (unsigned int y = 0; y < HEIGHT; ++y)
    {
      idx[1] = y;
      im->SetPixel(idx, f(x, y));
    }
  }
}

} // namespace SFFOLSIFT

namespace itk
{
template <typename TInputImage, typename TOutputImage>
class IsotropicDiffusionLevelSetFilter : public SparseFieldFourthOrderLevelSetImageFilter<TInputImage, TOutputImage>
{
public:
  using Self = IsotropicDiffusionLevelSetFilter;
  using Superclass = SparseFieldFourthOrderLevelSetImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkOverrideGetNameOfClassMacro(IsotropicDiffusionLevelSetFilter);
  itkNewMacro(Self);

  using typename Superclass::SparseImageType;
  using FunctionType = LevelSetFunctionWithRefitTerm<TOutputImage, SparseImageType>;
  using RadiusType = typename FunctionType::RadiusType;

protected:
  typename FunctionType::Pointer m_Function;
  IsotropicDiffusionLevelSetFilter()
  {
    RadiusType radius;
    for (unsigned int j = 0; j < TInputImage::ImageDimension; ++j)
    {
      radius[j] = 1;
    }

    m_Function = FunctionType::New();
    this->SetLevelSetFunction(m_Function);
    this->SetNumberOfLayers(this->GetMinimumNumberOfLayers());

    this->SetMaxNormalIteration(10);
    this->SetMaxRefitIteration(40);
    m_Function->Initialize(radius);
    this->SetNormalProcessType(0);

    m_Function->Print(std::cout);
  }

  bool
  Halt() override
  {
    if (this->GetElapsedIterations() == 50)
    {
      return true;
    }

    return false;
  }
};

} // end namespace itk

int
itkSparseFieldFourthOrderLevelSetImageFilterTest(int, char *[])
{
  using ImageType = itk::Image<float, 2>;

  auto image = ImageType::New();

  ImageType::RegionType          r;
  constexpr ImageType::SizeType  sz = { { SFFOLSIFT::HEIGHT, SFFOLSIFT::WIDTH } };
  constexpr ImageType::IndexType idx = { { 0, 0 } };
  r.SetSize(sz);
  r.SetIndex(idx);

  image->SetRegions(r);
  image->Allocate();

  SFFOLSIFT::evaluate_function(image, SFFOLSIFT::square);
  using FilterType = itk::IsotropicDiffusionLevelSetFilter<ImageType, ImageType>;
  auto filter = FilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(
    filter, IsotropicDiffusionLevelSetFilter, SparseFieldFourthOrderLevelSetImageFilter);


  constexpr unsigned int maxRefitIteration = 0;
  filter->SetMaxRefitIteration(maxRefitIteration);
  ITK_TEST_SET_GET_VALUE(maxRefitIteration, filter->GetMaxRefitIteration());

  constexpr unsigned int maxNormalIteration = 100;
  filter->SetMaxNormalIteration(maxNormalIteration);
  ITK_TEST_SET_GET_VALUE(maxNormalIteration, filter->GetMaxNormalIteration());

  constexpr typename FilterType::ValueType curvatureBandWidth = 4;
  filter->SetCurvatureBandWidth(curvatureBandWidth);
  ITK_TEST_SET_GET_VALUE(curvatureBandWidth, filter->GetCurvatureBandWidth());

  constexpr typename FilterType::ValueType rmsChangeNormalProcessTrigger = 0.001;
  filter->SetRMSChangeNormalProcessTrigger(rmsChangeNormalProcessTrigger);
  ITK_TEST_SET_GET_VALUE(rmsChangeNormalProcessTrigger, filter->GetRMSChangeNormalProcessTrigger());

  constexpr int normalProcessType = 0;
  filter->SetNormalProcessType(normalProcessType);
  ITK_TEST_SET_GET_VALUE(normalProcessType, filter->GetNormalProcessType());

  constexpr typename FilterType::ValueType normalProcessConductance{};
  filter->SetNormalProcessConductance(normalProcessConductance);
  ITK_TEST_SET_GET_VALUE(normalProcessConductance, filter->GetNormalProcessConductance());

  constexpr bool normalProcessUnsharpFlag = false;
  filter->SetNormalProcessUnsharpFlag(normalProcessUnsharpFlag);
  ITK_TEST_SET_GET_BOOLEAN(filter, NormalProcessUnsharpFlag, normalProcessUnsharpFlag);

  constexpr typename FilterType::ValueType normalProcessUnsharpWeight{};
  filter->SetNormalProcessUnsharpWeight(normalProcessUnsharpWeight);
  ITK_TEST_SET_GET_VALUE(normalProcessUnsharpWeight, filter->GetNormalProcessUnsharpWeight());

  filter->SetInput(image);

  ITK_TRY_EXPECT_NO_EXCEPTION(filter->Update());


  std::cout << "Test finished." << std::endl;
  return EXIT_SUCCESS;
}

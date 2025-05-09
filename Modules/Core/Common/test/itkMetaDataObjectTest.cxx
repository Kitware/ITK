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

#include "itkMetaDataObject.h"
#include "itkImage.h"
#include "itkMath.h"
#include "itkTestingMacros.h"

template <typename TMetaData>
int
testMetaData(const TMetaData & value)
{
  using MetaDataType = TMetaData;

  using MetaDataObjectType = itk::MetaDataObject<MetaDataType>;

  auto metaDataObject = MetaDataObjectType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(metaDataObject, MetaDataObject, MetaDataObjectBase);


  metaDataObject->SetMetaDataObjectValue(value);
  if (itk::Math::NotExactlyEquals(metaDataObject->GetMetaDataObjectValue(), value))
  {
    std::cerr << "Set value does not equal original value!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "The metadata's type name is: " << metaDataObject->GetMetaDataObjectTypeName() << std::endl;
  std::cout << "The metadata object: " << std::endl;
  metaDataObject->Print(std::cout);

  std::cout << std::endl << std::endl;

  return EXIT_SUCCESS;
}

int
itkMetaDataObjectTest(int, char *[])
{

  int result = EXIT_SUCCESS;
  result += testMetaData<unsigned char>(24);
  result += testMetaData<char>(-24);
  result += testMetaData<unsigned short>(24);
  result += testMetaData<short>(-24);
  result += testMetaData<unsigned int>(24);
  result += testMetaData<int>(-24);
  result += testMetaData<unsigned long>(24);
  result += testMetaData<long>(-24);
  result += testMetaData<unsigned long long>(24);
  result += testMetaData<long long>(-24);
  result += testMetaData<float>(-24);
  result += testMetaData<double>(-24);
  result += testMetaData<std::string>("I T K");
  result += testMetaData<std::vector<double>>({ 1.0, 2.0, 3.0 });
  result += testMetaData<std::vector<std::vector<double>>>({ { 1.0, 2.0, 3.0 }, { 4.0, 5.0, 6.0 } });
  result += testMetaData(itk::Array<char>(3, 'I'));
  result += testMetaData<itk::Array<double>>(itk::Array<double>(3, 3.0));
  result += testMetaData(itk::Matrix<float, 4, 4>());
  result += testMetaData<itk::Matrix<double, 3, 3>>(itk::Matrix<double, 3, 3>::GetIdentity());
  using ImageType = itk::Image<unsigned short, 3>;
  const ImageType::Pointer image = nullptr;
  result += testMetaData<ImageType::Pointer>(image);

  return result;
}

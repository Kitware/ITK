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

#include "itkHDF5TransformIOFactory.h"
#include "itkHDF5TransformIO.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkAffineTransform.h"
#include "itkTransformFactory.h"
#include "itkSimilarity2DTransform.h"
#include "itkBSplineTransform.h"
#include "itkTestingMacros.h"
#include "itksys/SystemTools.hxx"
#include <iomanip>

// Transforms from Filtering/DisplacementField/include
#include "itkBSplineExponentialDiffeomorphicTransform.h"
#include "itkBSplineSmoothingOnUpdateDisplacementFieldTransform.h"
#include "itkConstantVelocityFieldTransform.h"
#include "itkDisplacementFieldTransform.h"
#include "itkGaussianExponentialDiffeomorphicTransform.h"
#include "itkGaussianSmoothingOnUpdateDisplacementFieldTransform.h"
#include "itkMath.h"

template <typename TParametersValueType, typename DisplacementTransformType>
static int
ReadWriteTest(const std::string & fileName, const bool isRealDisplacementField, const bool useCompression)
{
  // First make a DisplacementField with known values
  constexpr double aNumberThatCanNotBeRepresentedInFloatingPoint = 1e-5 + 1e-7 + 1e-9 + 1e-13;
  constexpr double requiredSpacing = 1.2 + aNumberThatCanNotBeRepresentedInFloatingPoint;
  constexpr double requiredOrigin = 23.0 + aNumberThatCanNotBeRepresentedInFloatingPoint;
  auto             displacementTransform = DisplacementTransformType::New();
  using FieldType = typename DisplacementTransformType::DisplacementFieldType;
  auto knownField = FieldType::New(); // This is based on itk::Image
  {
    constexpr int                        dimLength = 20;
    auto                                 size = FieldType::SizeType::Filled(dimLength);
    const typename FieldType::IndexType  start{};
    const typename FieldType::RegionType region{ start, size };
    knownField->SetRegions(region);

    auto spacing = itk::MakeFilled<typename FieldType::SpacingType>(requiredSpacing);
    knownField->SetSpacing(spacing);
    auto origin = itk::MakeFilled<typename FieldType::PointType>(requiredOrigin);
    knownField->SetOrigin(origin);
    knownField->Allocate();

    auto zeroVector = itk::MakeFilled<typename DisplacementTransformType::OutputVectorType>(
      aNumberThatCanNotBeRepresentedInFloatingPoint);
    knownField->FillBuffer(zeroVector);

    displacementTransform->SetDisplacementField(knownField);
  }

  // Now test reading/writing many different transform types.

  using TransformReaderType = itk::TransformFileReaderTemplate<TParametersValueType>;
  auto reader = TransformReaderType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(reader, TransformFileReaderTemplate, LightProcessObject);


  reader->SetFileName(fileName);
  ITK_TEST_SET_GET_VALUE(fileName, reader->GetFileName());

  using TransformIOType = itk::HDF5TransformIOTemplate<TParametersValueType>;
  auto transformIO = TransformIOType::New();

  reader->SetTransformIO(transformIO);
  ITK_TEST_SET_GET_VALUE(transformIO, reader->GetTransformIO());

  using TransformWriterType = itk::TransformFileWriterTemplate<TParametersValueType>;
  auto writer = TransformWriterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(writer, TransformFileWriterTemplate, LightProcessObject);


  writer->SetFileName(fileName);
  ITK_TEST_SET_GET_VALUE(fileName, writer->GetFileName());

  writer->SetTransformIO(transformIO);
  ITK_TEST_SET_GET_VALUE(transformIO, writer->GetTransformIO());

  ITK_TEST_SET_GET_BOOLEAN(writer, UseCompression, useCompression);

  try
  {
    writer->AddTransform(displacementTransform);
    writer->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    reader->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Error while reading the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  if (isRealDisplacementField)
  {
    // New verify that the transform was read properly from disk without loss
    // Read first transform
    const typename itk::TransformFileReaderTemplate<TParametersValueType>::TransformListType * list =
      reader->GetTransformList();
    const typename DisplacementTransformType::ConstPointer readDisplacementTransform =
      static_cast<DisplacementTransformType *>((*(list->begin())).GetPointer());
    if (readDisplacementTransform.IsNull())
    {
      std::cerr << " ERROR: Read DisplacementTransform is null! " << std::endl;
      std::cerr << typeid(TParametersValueType).name() << std::endl;
      std::cerr << typeid(DisplacementTransformType).name() << std::endl;
      return EXIT_FAILURE;
    }
    const typename DisplacementTransformType::DisplacementFieldType::ConstPointer readDisplacement =
      readDisplacementTransform->GetDisplacementField();
    if (readDisplacement.IsNull())
    {
      std::cerr << " ERROR: GetDisplacementField failed! " << std::endl;
      std::cerr << typeid(TParametersValueType).name() << std::endl;
      std::cerr << typeid(DisplacementTransformType).name() << std::endl;
      std::cerr << "\n\n\n" << std::endl;

      (*list->begin())->Print(std::cerr);
      return EXIT_FAILURE;
    }

    if ((readDisplacement->GetSpacing() != knownField->GetSpacing()))
    //  || ( readDisplacement->GetSpacing()[0] != requiredSpacing ) )
    {
      std::cerr << "Error invalid spacing restored from disk" << std::endl;
      std::cerr << std::setprecision(17) << '\n'
                << readDisplacement->GetSpacing() << " != " << knownField->GetSpacing() << '\n'
                << requiredSpacing << "It is likely going trough a float truncation "
                << static_cast<float>(requiredSpacing) << std::endl;
      return EXIT_FAILURE;
    }
    if ((readDisplacement->GetOrigin() != knownField->GetOrigin()) ||
        (itk::Math::NotExactlyEquals(readDisplacement->GetOrigin()[0], requiredOrigin)))
    {
      std::cerr << "Error invalid origin restored from disk" << std::endl;
      std::cerr << std::setprecision(17) << '\n'
                << readDisplacement->GetOrigin() << " != " << knownField->GetOrigin() << '\n'
                << requiredOrigin << std::endl;
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

template <typename TParametersValueType>
static int
oneTest(const std::string & goodname, const std::string badname, const bool useCompression)
{
  using AffineTransformType = typename itk::AffineTransform<TParametersValueType, 4>;
  using AffineTransformTypeNotRegistered = typename itk::AffineTransform<TParametersValueType, 10>;
  auto affine = AffineTransformType::New();

  // Set its parameters
  {
    typename AffineTransformType::ParametersType p = affine->GetParameters();
    for (unsigned int i = 0; i < p.GetSize(); ++i)
    {
      p[i] = i;
    }
    affine->SetParameters(p);
  }
  {
    typename AffineTransformType::FixedParametersType p = affine->GetFixedParameters();
    for (unsigned int i = 0; i < p.GetSize(); ++i)
    {
      p[i] = i;
    }
    affine->SetFixedParameters(p);
  }
  const typename itk::TransformFileWriterTemplate<TParametersValueType>::Pointer writer =
    itk::TransformFileWriterTemplate<TParametersValueType>::New();
  writer->SetUseCompression(useCompression);
  const typename itk::TransformFileReaderTemplate<TParametersValueType>::Pointer reader =
    itk::TransformFileReaderTemplate<TParametersValueType>::New();

  writer->AddTransform(affine);

  writer->SetFileName(goodname);
  reader->SetFileName(goodname);

  // Testing writing std::cout << "Testing write : ";
  affine->Print(std::cout);
  try
  {
    writer->Update();
    std::cout << std::endl;
    std::cout << "Testing read : " << std::endl;
    reader->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }


  try
  {
    const typename itk::TransformFileReaderTemplate<TParametersValueType>::TransformListType * list =
      reader->GetTransformList();
    auto lit = list->begin();
    while (lit != list->end())
    {
      (*lit)->Print(std::cout);
      std::cout << "Input space name: " << (*lit)->GetInputSpaceName() << std::endl;
      std::cout << "Output space name: " << (*lit)->GetOutputSpaceName() << std::endl;
      ++lit;
    }
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "Creating bad writer" << std::endl;
  auto Bogus = AffineTransformTypeNotRegistered::New();

  // Set its parameters
  {
    typename AffineTransformType::ParametersType p = Bogus->GetParameters();
    for (unsigned int i = 0; i < p.GetSize(); ++i)
    {
      p[i] = i;
    }
    Bogus->SetParameters(p);
  }
  {
    typename AffineTransformType::FixedParametersType p = Bogus->GetFixedParameters();
    for (unsigned int i = 0; i < p.GetSize(); ++i)
    {
      p[i] = i;
    }
    Bogus->SetFixedParameters(p);
  }
  const typename itk::TransformFileWriterTemplate<TParametersValueType>::Pointer badwriter =
    itk::TransformFileWriterTemplate<TParametersValueType>::New();
  badwriter->SetUseCompression(useCompression);
  const typename itk::TransformFileReaderTemplate<TParametersValueType>::Pointer badreader =
    itk::TransformFileReaderTemplate<TParametersValueType>::New();
  badwriter->AddTransform(Bogus);
  badwriter->SetFileName(badname);
  badreader->SetFileName(badname);

  // Testing writing
  std::cout << "Testing write of non register transform : " << std::endl;
  std::cout << std::flush;
  try
  {
    badwriter->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // Testing writing
  std::cout << "Testing read of non register transform : " << std::endl;
  std::cout << std::flush;
  bool caught = false;
  try
  {
    badreader->Update();
  }
  catch (const itk::ExceptionObject & excp)
  {
    caught = true;
    std::cout << "Caught exception as expected" << std::endl;
    std::cout << excp << std::endl;
  }
  if (!caught)
  {
    std::cerr << "Did not catch non registered transform" << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  int error_sum = 0;
  error_sum += ReadWriteTest<float, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<float, 2>>(
    "f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<float, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<float, 3>>(
    "f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<double, 2>>(
    goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<double, 3>>(
    goodname, true, useCompression);

  error_sum += ReadWriteTest<float, itk::DisplacementFieldTransform<float, 2>>("f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<float, itk::DisplacementFieldTransform<float, 3>>("f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::DisplacementFieldTransform<double, 2>>(goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::DisplacementFieldTransform<double, 3>>(goodname, true, useCompression);

  error_sum += ReadWriteTest<float, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<float, 2>>(
    "f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<float, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<float, 3>>(
    "f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<double, 2>>(
    goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<double, 3>>(
    goodname, true, useCompression);

  error_sum += ReadWriteTest<float, itk::GaussianSmoothingOnUpdateDisplacementFieldTransform<float, 2>>(
    "f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<float, itk::GaussianSmoothingOnUpdateDisplacementFieldTransform<float, 3>>(
    "f" + goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::GaussianSmoothingOnUpdateDisplacementFieldTransform<double, 2>>(
    goodname, true, useCompression);
  error_sum += ReadWriteTest<double, itk::GaussianSmoothingOnUpdateDisplacementFieldTransform<double, 3>>(
    goodname, true, useCompression);

  error_sum +=
    ReadWriteTest<float, itk::ConstantVelocityFieldTransform<float, 2>>("f" + goodname, false, useCompression);
  error_sum +=
    ReadWriteTest<float, itk::ConstantVelocityFieldTransform<float, 3>>("f" + goodname, false, useCompression);
  error_sum += ReadWriteTest<double, itk::ConstantVelocityFieldTransform<double, 2>>(goodname, false, useCompression);
  error_sum += ReadWriteTest<double, itk::ConstantVelocityFieldTransform<double, 3>>(goodname, false, useCompression);

  error_sum += ReadWriteTest<float, itk::GaussianExponentialDiffeomorphicTransform<float, 2>>(
    "f" + goodname, false, useCompression);
  error_sum += ReadWriteTest<float, itk::GaussianExponentialDiffeomorphicTransform<float, 3>>(
    "f" + goodname, false, useCompression);
  error_sum +=
    ReadWriteTest<double, itk::GaussianExponentialDiffeomorphicTransform<double, 2>>(goodname, false, useCompression);
  error_sum +=
    ReadWriteTest<double, itk::GaussianExponentialDiffeomorphicTransform<double, 3>>(goodname, false, useCompression);

  if (error_sum > 0)
  {
    std::cerr << "Atleast 1 transform type could not be read/written " << error_sum << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[PASSED]" << std::endl;
  return EXIT_SUCCESS;
}

int
itkIOTransformHDF5Test(int argc, char * argv[])
{

  itk::ObjectFactoryBase::RegisterFactory(itk::HDF5TransformIOFactory::New());
  if (argc > 2)
  {
    itksys::SystemTools::ChangeDirectory(argv[2]);
  }
  if (argc > 1)
  {
    const std::string testType{ argv[1] };
    if (testType == "uncompressed")
    {
      const int result1 = oneTest<float>("Transforms_float.h5", "TransformsBad_float.h5", false);
      const int result2 = oneTest<double>("Transforms_double.hdf5", "TransformsBad_double.hdf5", false);
      return (!(result1 == EXIT_SUCCESS && result2 == EXIT_SUCCESS));
    }
    if (testType == "compressed")
    {
      const int result1 = oneTest<float>("Transforms_float_compressed.h5", "TransformsBad_float_compressed.h5", true);
      const int result2 =
        oneTest<double>("Transforms_double_compressed.hdf5", "TransformsBad_double_compressed.hdf5", true);
      return (!(result1 == EXIT_SUCCESS && result2 == EXIT_SUCCESS));
    }
    else if (itksys::SystemTools::FileExists(testType)) // Assume the final parameter is a filename to be read
    {
      // This test only verifies that the test can read the transform.
      using TFM_READER_TYPE = typename itk::TransformFileReaderTemplate<double>;
      auto reader = TFM_READER_TYPE::New();
      reader->SetFileName(testType);
      reader->Update();
      const TFM_READER_TYPE::TransformListType * const myTransformList = reader->GetTransformList();
      if (myTransformList->size() != 1)
      {
        return EXIT_FAILURE;
      }
      if (myTransformList->front()->GetTransformTypeAsString() != "VersorRigid3DTransform_double_3_3")
      {
        std::cerr << "Incorrect transform type identified " << myTransformList->front()->GetTransformTypeAsString()
                  << " != VersorRigid3DTransform_double_3_3" << std::endl;
        return EXIT_FAILURE;
      }
      return EXIT_SUCCESS;
    }
  }
  std::cerr << "ERROR: first argument must be one of [uncompressed|compressed|<filename to read>]" << std::endl;
  return EXIT_FAILURE;
}

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
#ifndef itkMetaImageConverter_h
#define itkMetaImageConverter_h

// to avoid an Internal Compiler Error in Visual Studio 6.0
#include "metaImage.h"
#include "itkMetaConverterBase.h"
#include "itkImageSpatialObject.h"

namespace itk
{

/**
 * \class MetaImageConverter
 *  \brief converts between MetaObject<->SpatialObject.
 *
 *  \sa MetaConverterBase
 *  \ingroup ITKSpatialObjects
 */
template <unsigned int VDimension = 3,
          typename TPixel = unsigned char,
          typename TSpatialObjectType = ImageSpatialObject<VDimension, TPixel>>
class ITK_TEMPLATE_EXPORT MetaImageConverter : public MetaConverterBase<VDimension>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(MetaImageConverter);

  /** Standard class type aliases */
  using Self = MetaImageConverter;
  using Superclass = MetaConverterBase<VDimension>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(MetaImageConverter);

  using typename Superclass::SpatialObjectType;
  using SpatialObjectPointer = typename SpatialObjectType::Pointer;
  using typename Superclass::MetaObjectType;

  /** Specific class types for conversion */
  using ImageSpatialObjectType = TSpatialObjectType;
  using ImageSpatialObjectPointer = typename ImageSpatialObjectType::Pointer;
  using ImageSpatialObjectConstPointer = typename ImageSpatialObjectType::ConstPointer;
  using ImageMetaObjectType = MetaImage;
  using ImageType = Image<TPixel, VDimension>;

  /** Convert the MetaObject to Spatial Object */
  SpatialObjectPointer
  MetaObjectToSpatialObject(const MetaObjectType * mo) override;

  /** Convert the SpatialObject to MetaObject */
  MetaObjectType *
  SpatialObjectToMetaObject(const SpatialObjectType * so) override;

protected:
  /** Create the specific MetaObject for this class */
  /** @ITKStartGrouping */
  MetaObjectType *
  CreateMetaObject() override;
  virtual const char *
  GetMetaObjectSubType();
  /** @ITKEndGrouping */
  typename ImageType::Pointer
  AllocateImage(const ImageMetaObjectType * image);

  MetaImageConverter() = default;
  ~MetaImageConverter() override = default;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkMetaImageConverter.hxx"
#endif

#endif

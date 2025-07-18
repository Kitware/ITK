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
#ifndef itkImageSpatialObject_h
#define itkImageSpatialObject_h

#include "itkImage.h"
#include "itkContinuousIndex.h"
#include "itkSpatialObject.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

namespace itk
{
/**
 * \class ImageSpatialObject
 * \brief Implementation of an image as spatial object.
 *
 * This class combines functionalities from a spatial object,
 * and an image.
 *
 * \sa SpatialObject CompositeSpatialObject
 * \ingroup ITKSpatialObjects
 */

template <unsigned int TDimension = 3, typename TPixelType = unsigned char>
class ITK_TEMPLATE_EXPORT ImageSpatialObject : public SpatialObject<TDimension>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ImageSpatialObject);

  using ScalarType = double;
  using Self = ImageSpatialObject<TDimension, TPixelType>;
  using Superclass = SpatialObject<TDimension>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using ContinuousIndexType = ContinuousIndex<double, TDimension>;
  using PixelType = TPixelType;
  using ImageType = Image<PixelType, TDimension>;
  using ImagePointer = typename ImageType::ConstPointer;
  using IndexType = typename ImageType::IndexType;
  using typename Superclass::PointType;
  using InterpolatorType = InterpolateImageFunction<ImageType>;

  using NNInterpolatorType = NearestNeighborInterpolateImageFunction<ImageType>;

  static constexpr unsigned int ObjectDimension = TDimension;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ImageSpatialObject);

  /** Reset the spatial object to its initial condition, yet preserves
   *   Id, Parent, and Child information */
  void
  Clear() override;

  /** Set the image. */
  void
  SetImage(const ImageType * image);

  /** Get a pointer to the image currently attached to the object. */
  const ImageType *
  GetImage() const;

  /** Returns true if the point is inside, false otherwise. */
  bool
  IsInsideInObjectSpace(const PointType & point) const override;

  /* Avoid hiding the overload that supports depth and name arguments */
  using Superclass::IsInsideInObjectSpace;

  /** Returns the value of the image at the requested point.
   *
   * Returns true if that value is valid.
   *
   * The value returned is always of type double; for RGB Images the value returned is the value of the first channel.
   */
  bool
  ValueAtInObjectSpace(const PointType &   point,
                       double &            value,
                       unsigned int        depth = 0,
                       const std::string & name = "") const override;

  /** Returns the latest modified time of the object and its component. */
  [[nodiscard]] ModifiedTimeType
  GetMTime() const override;

  /** Set the slice position */
  /** @ITKStartGrouping */
  itkSetMacro(SliceNumber, IndexType);
  void
  SetSliceNumber(unsigned int dimension, int position);
  /** @ITKEndGrouping */
  /** Get the slice position */
  /** @ITKStartGrouping */
  itkGetConstMacro(SliceNumber, IndexType);
  int
  GetSliceNumber(unsigned int dimension)
  {
    return m_SliceNumber[dimension];
  }
  /** @ITKEndGrouping */
#if !defined(ITK_LEGACY_REMOVE)
  itkLegacyMacro(const char * GetPixelTypeName())
  {
    return m_PixelType.c_str();
  }
#endif

  /** Set/Get the interpolator. */
  /** @ITKStartGrouping */
  void
  SetInterpolator(InterpolatorType * interpolator);
  itkGetConstMacro(Interpolator, InterpolatorType *);
  /** @ITKEndGrouping */
  /** Updates the regions of this spatial object in accordance with its current image, and calls its Superclass. */
  void
  Update() override;

protected:
  /** Compute the boundaries of the image spatial object. */
  void
  ComputeMyBoundingBox() override;

  ImageSpatialObject();
  ~ImageSpatialObject() override;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  [[nodiscard]] typename LightObject::Pointer
  InternalClone() const override;

private:
  ImagePointer m_Image{};

  IndexType m_SliceNumber{};

#if !defined(ITK_LEGACY_REMOVE)
  std::string m_PixelType{};
#endif

  typename InterpolatorType::Pointer m_Interpolator{};

  /** Updates the regions of this spatial object in accordance with its current image. */
  void
  UpdateImageRegions();

#if !defined(ITK_LEGACY_REMOVE)
  template <typename T>
  void
  SetPixelTypeName(const T *)
  {
    itkWarningMacro("itk::ImageSpatialObject() : PixelType not recognized");
  }

  void
  SetPixelTypeName(const short *)
  {
    m_PixelType = "short";
  }

  void
  SetPixelTypeName(const unsigned char *)
  {
    m_PixelType = "unsigned char";
  }

  void
  SetPixelTypeName(const unsigned short *)
  {
    m_PixelType = "unsigned short";
  }

  void
  SetPixelTypeName(const float *)
  {
    m_PixelType = "float";
  }

  void
  SetPixelTypeName(const double *)
  {
    m_PixelType = "double";
  }
#endif
};
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkImageSpatialObject.hxx"
#endif

#endif // itkImageSpatialObject_h

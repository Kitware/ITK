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
#ifndef itkImageIterator_h
#define itkImageIterator_h

#include "itkImageConstIterator.h"

namespace itk
{
/**
 * \class ImageIterator
 * \brief A multi-dimensional iterator templated over image type.
 *
 * This is a base class of ImageConstIterator that adds write-access
 * functionality.  Please see ImageConstIterator for more information.
 *
 * \par MORE INFORMATION
 * For a complete description of the ITK Image Iterators and their API, please
 * see the Iterators chapter in the ITK Software Guide.  The ITK Software Guide
 * is available in print and as a free .pdf download from https://www.itk.org.
 *
 * \ingroup ImageIterators
 *
 * \sa ImageConstIterator \sa ConditionalConstIterator
 * \sa ConstNeighborhoodIterator \sa ConstShapedNeighborhoodIterator
 * \sa ConstSliceIterator  \sa CorrespondenceDataStructureIterator
 * \sa FloodFilledFunctionConditionalConstIterator
 * \sa FloodFilledImageFunctionConditionalConstIterator
 * \sa FloodFilledImageFunctionConditionalIterator
 * \sa FloodFilledSpatialFunctionConditionalConstIterator
 * \sa FloodFilledSpatialFunctionConditionalIterator
 * \sa ImageConstIterator \sa ImageConstIteratorWithIndex
 * \sa ImageIterator \sa ImageIteratorWithIndex
 * \sa ImageLinearConstIteratorWithIndex  \sa ImageLinearIteratorWithIndex
 * \sa ImageRandomConstIteratorWithIndex  \sa ImageRandomIteratorWithIndex
 * \sa ImageRegionConstIterator \sa ImageRegionConstIteratorWithIndex
 * \sa ImageRegionExclusionConstIteratorWithIndex
 * \sa ImageRegionExclusionIteratorWithIndex
 * \sa ImageRegionIterator  \sa ImageRegionIteratorWithIndex
 * \sa ImageRegionReverseConstIterator  \sa ImageRegionReverseIterator
 * \sa ImageReverseConstIterator  \sa ImageReverseIterator
 * \sa ImageSliceConstIteratorWithIndex  \sa ImageSliceIteratorWithIndex
 * \sa NeighborhoodIterator \sa PathConstIterator  \sa PathIterator
 * \sa ShapedNeighborhoodIterator  \sa SliceIterator
 *
 *
 * \ingroup ITKCommon
 */
template <typename TImage>
class ITK_TEMPLATE_EXPORT ImageIterator : public ImageConstIterator<TImage>
{
public:
  ITK_DEFAULT_COPY_AND_MOVE(ImageIterator);

  /** Standard class type aliases. */
  using Self = ImageIterator;

  /** Dimension of the image the iterator walks.  This constant is needed so
   * functions that are templated over image iterator type (as opposed to
   * being templated over pixel type and dimension) can have compile time
   * access to the dimension of the image that the iterator walks. */
  static constexpr unsigned int ImageIteratorDimension = TImage::ImageDimension;

  /** Define the superclass */
  using Superclass = ImageConstIterator<TImage>;

  /** Inherit types from the superclass */
  using typename Superclass::IndexType;
  using typename Superclass::SizeType;
  using typename Superclass::OffsetType;
  using typename Superclass::RegionType;
  using typename Superclass::ImageType;
  using typename Superclass::PixelContainer;
  using typename Superclass::PixelContainerPointer;
  using typename Superclass::InternalPixelType;
  using typename Superclass::PixelType;
  using typename Superclass::AccessorType;

  /** Default Constructor. Need to provide a default constructor since we
   * provide a copy constructor. */
  ImageIterator() = default;

  /** Default Destructor */
  ~ImageIterator() override = default;

  /** Constructor establishes an iterator to walk a particular image and a particular region of that image. Initializes
   * the iterator at the begin of the region. */
  ImageIterator(TImage * ptr, const RegionType & region);

  /** Set the pixel value */
  void
  Set(const PixelType & value) const
  {
    // const_cast is needed here because m_Buffer is declared as a const
    // pointer in the superclass which is the ConstIterator.
    this->m_PixelAccessorFunctor.Set(*(const_cast<InternalPixelType *>(this->m_Buffer) + this->m_Offset), value);
  }

  /** Return a reference to the pixel
   * This method will provide the fastest access to pixel
   * data, but it will NOT support ImageAdaptors. */
  PixelType &
  Value()
  {
    // const_cast is needed here because m_Buffer is declared as a const
    // pointer in the superclass which is the ConstIterator.
    return *(const_cast<InternalPixelType *>(this->m_Buffer) + this->m_Offset);
  }

  /** Get the image that this iterator walks. */
  [[nodiscard]] ImageType *
  GetImage() const
  {
    // const_cast is needed here because m_Image is declared as a const pointer
    // in the base class which is the ConstIterator.
    return const_cast<ImageType *>(this->m_Image.GetPointer());
  }

protected:
  /** This constructor is declared protected in order to enforce
    const-correctness */
  /** @ITKStartGrouping */
  ImageIterator(const ImageConstIterator<TImage> & it);
  Self &
  operator=(const ImageConstIterator<TImage> & it);
  /** @ITKEndGrouping */
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkImageIterator.hxx"
#endif

#endif

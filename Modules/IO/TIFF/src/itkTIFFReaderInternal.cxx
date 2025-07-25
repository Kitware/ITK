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

#include "itkMacro.h"
#include "itkObject.h"
#include "itkTIFFReaderInternal.h"
#include <sys/stat.h>

namespace itk
{

// Macro added in libtiff 4.5.0
#if defined(TIFFLIB_AT_LEAST)
namespace
{
class tiff_open_options_free
{
public:
  void
  operator()(TIFFOpenOptions * ptr) const
  {
    TIFFOpenOptionsFree(ptr);
  }
};

ITK_FORMAT_PRINTF(4, 0)
int
itkTIFFErrorHandlerExtR([[maybe_unused]] TIFF * tif,
                        [[maybe_unused]] void * user_data,
                        const char *            module,
                        const char *            fmt,
                        va_list                 ap)
{
  auto * self = reinterpret_cast<TIFFReaderInternal *>(user_data);
  if (::itk::Object::GetGlobalWarningDisplay() && !self->m_ErrorSilence)
  {
    char out[256];
    ITK_GCC_PRAGMA_PUSH
    ITK_GCC_SUPPRESS_Wformat_nonliteral;
    vsnprintf(out, sizeof(out), fmt, ap);
    ITK_GCC_PRAGMA_POP
    std::ostringstream itkmsg;
    itkmsg << "ERROR: libtiff(" << (module ? module : "") << ") message: " << out << std::endl;
    ::itk::OutputWindowDisplayErrorText(itkmsg.str().c_str());
  }
  return 1;
}

ITK_FORMAT_PRINTF(4, 0)
int
itkTIFFWarningHandlerExtR([[maybe_unused]] TIFF * tif,
                          [[maybe_unused]] void * user_data,
                          const char *            module,
                          const char *            fmt,
                          va_list                 ap)
{

  auto * self = reinterpret_cast<TIFFReaderInternal *>(user_data);
  if (::itk::Object::GetGlobalWarningDisplay() && !self->m_WarningSilence)
  {
    char out[256];
    ITK_GCC_PRAGMA_PUSH
    ITK_GCC_SUPPRESS_Wformat_nonliteral;
    vsnprintf(out, sizeof(out), fmt, ap);
    ITK_GCC_PRAGMA_POP
    std::ostringstream itkmsg;
    itkmsg << "WARNING: libtiff(" << (module ? module : "") << ") message: " << out << std::endl;
    ::itk::OutputWindowDisplayWarningText(itkmsg.str().c_str());
  }
  return 1;
}
} // namespace
#endif

int
TIFFReaderInternal::Open(const char * filename, bool silent)
{
  this->Clean();
  struct stat fs;
  if (stat(filename, &fs))
  {
#if defined(_WIN32) && !defined(__MINGW32_VERSION)
    struct _stat64 fs64;
    if (_stat64(filename, &fs64))
    {
      // Both stat() and _stat64() return != 0
      return 0;
    }
#else
    return 0;
#endif
  }
// Macro added in libtiff 4.5.0
#if defined(TIFFLIB_AT_LEAST)

  std::unique_ptr<TIFFOpenOptions, tiff_open_options_free> const options(TIFFOpenOptionsAlloc());
  TIFFOpenOptionsSetErrorHandlerExtR(options.get(), itkTIFFErrorHandlerExtR, this);
  TIFFOpenOptionsSetWarningHandlerExtR(options.get(), itkTIFFWarningHandlerExtR, this);
  if (silent)
  {
    this->m_ErrorSilence = true;
  }
  this->m_Image = TIFFOpenExt(filename, "r", options.get());

#else
  if (silent)
  {

    // Now check if this is a valid TIFF image
    TIFFErrorHandler error_save = TIFFSetErrorHandler(nullptr);

    this->m_Image = TIFFOpen(filename, "r");
    TIFFSetErrorHandler(error_save);
  }
  else
  {
    this->m_Image = TIFFOpen(filename, "r");
  }

#endif
  if (!this->m_Image)
  {
    this->Clean();
    return 0;
  }
  if (!this->Initialize())
  {
    this->Clean();
    return 0;
  }

  this->m_WarningSilence = false;
  this->m_ErrorSilence = false;
  this->m_IsOpen = true;
  return 1;
}

void
TIFFReaderInternal::Clean()
{
  if (this->m_Image)
  {
    TIFFClose(this->m_Image);
  }
  this->m_Image = nullptr;
  this->m_Width = 0;
  this->m_Height = 0;
  this->m_SamplesPerPixel = 0;
  this->m_Compression = 0;
  this->m_BitsPerSample = 0;
  this->m_Photometrics = 0;
  this->m_HasValidPhotometricInterpretation = false;
  this->m_PlanarConfig = 0;
  this->m_CurrentPage = 0;
  this->m_NumberOfPages = 0;
  this->m_NumberOfTiles = 0;
  this->m_Orientation = ORIENTATION_TOPLEFT;
  this->m_TileRows = 0;
  this->m_TileColumns = 0;
  this->m_TileWidth = 0;
  this->m_TileHeight = 0;
  this->m_XResolution = 1;
  this->m_YResolution = 1;
  this->m_SubFiles = 0;
  this->m_IgnoredSubFiles = 0;
  this->m_SampleFormat = 1;
  this->m_ResolutionUnit = 1; // none
  this->m_IsOpen = false;

  this->m_WarningSilence = false;
  this->m_ErrorSilence = false;
}


TIFFReaderInternal::TIFFReaderInternal() { this->Clean(); }

int
TIFFReaderInternal::Initialize()
{
  if (this->m_Image)
  {
    if (!TIFFGetField(this->m_Image, TIFFTAG_IMAGEWIDTH, &this->m_Width) ||
        !TIFFGetField(this->m_Image, TIFFTAG_IMAGELENGTH, &this->m_Height))
    {
      return 0;
    }

    // Get the resolution in each direction
    TIFFGetField(this->m_Image, TIFFTAG_XRESOLUTION, &this->m_XResolution);
    TIFFGetField(this->m_Image, TIFFTAG_YRESOLUTION, &this->m_YResolution);
    TIFFGetField(this->m_Image, TIFFTAG_RESOLUTIONUNIT, &this->m_ResolutionUnit);

    // Check the number of pages. First by looking at the number of directories
    this->m_NumberOfPages = TIFFNumberOfDirectories(this->m_Image);

    if (this->m_NumberOfPages == 0)
    {
      itkGenericExceptionMacro("No directories found in TIFF file.");
    }

    if (TIFFIsTiled(this->m_Image))
    {
      this->m_NumberOfTiles = TIFFNumberOfTiles(this->m_Image);

      if (!TIFFGetField(this->m_Image, TIFFTAG_TILEWIDTH, &this->m_TileWidth) ||
          !TIFFGetField(this->m_Image, TIFFTAG_TILELENGTH, &this->m_TileHeight))
      {
        itkGenericExceptionMacro("Cannot read tile width and tile length from file");
      }
      else
      {
        this->m_TileRows = this->m_Height / this->m_TileHeight;
        this->m_TileColumns = this->m_Width / this->m_TileWidth;
      }
    }

    // Checking if the TIFF contains subfiles
    if (this->m_NumberOfPages > 1)
    {
      this->m_SubFiles = 0;
      this->m_IgnoredSubFiles = 0;

      for (unsigned int page = 0; page < this->m_NumberOfPages; ++page)
      {
        int32_t subfiletype = 6;
        if (TIFFGetField(this->m_Image, TIFFTAG_SUBFILETYPE, &subfiletype))
        {
          if (subfiletype == 0)
          {
            this->m_SubFiles += 1;
          }
          // ignored flags
          else if (subfiletype & FILETYPE_REDUCEDIMAGE || subfiletype & FILETYPE_MASK)
          {
            ++this->m_IgnoredSubFiles;
          }
        }
        TIFFReadDirectory(this->m_Image);
      }

      // Set the directory to the first image, and reads it
      TIFFSetDirectory(this->m_Image, 0);
    }

    TIFFGetFieldDefaulted(this->m_Image, TIFFTAG_ORIENTATION, &this->m_Orientation);
    TIFFGetFieldDefaulted(this->m_Image, TIFFTAG_SAMPLESPERPIXEL, &this->m_SamplesPerPixel);
    TIFFGetFieldDefaulted(this->m_Image, TIFFTAG_COMPRESSION, &this->m_Compression);
    TIFFGetFieldDefaulted(this->m_Image, TIFFTAG_BITSPERSAMPLE, &this->m_BitsPerSample);
    TIFFGetFieldDefaulted(this->m_Image, TIFFTAG_PLANARCONFIG, &this->m_PlanarConfig);
    TIFFGetFieldDefaulted(this->m_Image, TIFFTAG_SAMPLEFORMAT, &this->m_SampleFormat);

    // If TIFFGetField returns false, there's no Photometric Interpretation
    // set for this image, but that's a required field so we set a warning flag.
    // (Because the "Photometrics" field is an enum, we can't rely on setting
    // this->m_Photometrics to some signal value.)
    if (TIFFGetField(this->m_Image, TIFFTAG_PHOTOMETRIC, &this->m_Photometrics))
    {
      this->m_HasValidPhotometricInterpretation = true;
    }
    else
    {
      this->m_HasValidPhotometricInterpretation = false;
    }
  }

  return 1;
}

int
TIFFReaderInternal::CanRead()
{
  const bool compressionSupported = (TIFFIsCODECConfigured(this->m_Compression) == 1);
  return (this->m_Image && (this->m_Width > 0) && (this->m_Height > 0) && (this->m_SamplesPerPixel > 0) &&
          compressionSupported && (m_NumberOfTiles == 0) // just use TIFFReadRGBAImage, an
                                                         // native optimized version would be nice
          && (this->m_HasValidPhotometricInterpretation) &&
          (this->m_Photometrics == PHOTOMETRIC_RGB || this->m_Photometrics == PHOTOMETRIC_MINISWHITE ||
           this->m_Photometrics == PHOTOMETRIC_MINISBLACK ||
           (this->m_Photometrics == PHOTOMETRIC_PALETTE && this->m_BitsPerSample != 32)) &&
          (this->m_PlanarConfig == PLANARCONFIG_CONTIG || this->m_SamplesPerPixel == 1) &&
          (this->m_Orientation == ORIENTATION_TOPLEFT || this->m_Orientation == ORIENTATION_BOTLEFT) &&
          (this->m_BitsPerSample == 8 || this->m_BitsPerSample == 16 || this->m_BitsPerSample == 32));
}

} // namespace itk

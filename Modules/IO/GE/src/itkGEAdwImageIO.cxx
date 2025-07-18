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
#include "itkGEAdwImageIO.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

// From uiig library "The University of Iowa Imaging Group-UIIG"

namespace itk
{
// Default constructor
GEAdwImageIO::GEAdwImageIO() = default;
GEAdwImageIO::~GEAdwImageIO() = default;

bool
GEAdwImageIO::CanReadFile(const char * FileNameToRead)
{
  //
  // Can you open it?
  std::ifstream f;
  try
  {
    this->OpenFileForReading(f, FileNameToRead);
  }
  catch (const ExceptionObject &)
  {
    return false;
  }

  //
  // This test basically snoops out the image dimensions, and the
  // length of the variable-length part of the header, and computes
  // the size the file should be and compares it with the actual size.
  // if it's not reading a GEAdw file, chances are overwhelmingly good
  // that this operation will fail somewhere along the line.
  short matrixX = 0;
  if (this->GetShortAt(f, GE_ADW_IM_IMATRIX_X, &matrixX, false) != 0)
  {
    f.close();
    return false;
  }
  short matrixY = 0;
  if (this->GetShortAt(f, GE_ADW_IM_IMATRIX_Y, &matrixY, false) != 0)
  {
    f.close();
    return false;
  }
  int varHdrSize = 0;
  if (this->GetIntAt(f, GE_ADW_VARIABLE_HDR_LENGTH, &varHdrSize, false) != 0)
  {
    f.close();
    return false;
  }

  const size_t imageSize = varHdrSize + GE_ADW_FIXED_HDR_LENGTH + (matrixX * matrixY * sizeof(short));

  if (imageSize != itksys::SystemTools::FileLength(FileNameToRead))
  {
    f.close();
    return false;
  }
  f.close();
  return true;
}

GEImageHeader *
GEAdwImageIO::ReadHeader(const char * FileNameToRead)
{
  char tmpbuf[1024];

  if (!this->CanReadFile(FileNameToRead))
  {
    RAISE_EXCEPTION();
  }
  auto * hdr = new GEImageHeader;
  //
  // Next, can you open it?
  std::ifstream f;
  this->OpenFileForReading(f, FileNameToRead);

  snprintf(hdr->scanner, sizeof(hdr->scanner), "GE-ADW");
  this->GetStringAt(f, GE_ADW_EX_PATID, tmpbuf, 12);
  tmpbuf[12] = '\0';
  hdr->patientId[0] = '\0';
  for (char * ptr = strtok(tmpbuf, "-"); ptr != nullptr; ptr = strtok(nullptr, "-"))
  {
    strcat(hdr->patientId, ptr);
  }

  this->GetStringAt(f, GE_ADW_EX_TYP, hdr->modality, GE_ADW_EX_TYP_LEN);
  hdr->modality[GE_ADW_EX_TYP_LEN] = '\0';

  this->GetStringAt(f, GE_ADW_EX_PATNAME, hdr->name, GE_ADW_EX_PATNAME_LEN);
  hdr->name[GE_ADW_EX_PATNAME_LEN] = '\0';

  this->GetStringAt(f, GE_ADW_EX_HOSPNAME, hdr->hospital, 34);
  hdr->hospital[33] = '\0';

  int timeStamp = 0;
  this->GetIntAt(f, GE_ADW_EX_DATETIME, &timeStamp);
  this->statTimeToAscii(&timeStamp, hdr->date, sizeof(hdr->date));

  this->GetStringAt(f, GE_ADW_SU_PRODID, hdr->scanner, 13);
  hdr->scanner[13] = '\0';

  this->GetShortAt(f, GE_ADW_SE_NO, &(hdr->seriesNumber));

  this->GetShortAt(f, GE_ADW_IM_NO, &(hdr->imageNumber));

  this->GetShortAt(f, GE_ADW_IM_CPHASENUM, &(hdr->imagesPerSlice));

  this->GetShortAt(f, GE_ADW_IM_CPHASENUM, &(hdr->turboFactor));

  this->GetFloatAt(f, GE_ADW_IM_SLTHICK, &(hdr->sliceThickness));
  hdr->sliceGap = 0.0f;

  this->GetShortAt(f, GE_ADW_IM_IMATRIX_X, &(hdr->imageXsize));

  this->GetShortAt(f, GE_ADW_IM_IMATRIX_Y, &(hdr->imageYsize));

  hdr->acqXsize = hdr->imageXsize;
  hdr->acqYsize = hdr->imageYsize;

  this->GetFloatAt(f, GE_ADW_IM_DFOV, &hdr->xFOV);
  hdr->yFOV = hdr->xFOV;

  this->GetFloatAt(f, GE_ADW_IM_PIXSIZE_X, &hdr->imageXres);

  this->GetFloatAt(f, GE_ADW_IM_PIXSIZE_Y, &hdr->imageYres);

  short tmpShort = 0;
  this->GetShortAt(f, GE_ADW_IM_PLANE, &tmpShort);
  switch (tmpShort)
  {
    case GE_CORONAL:
      hdr->coordinateOrientation = AnatomicalOrientation(AnatomicalOrientation::CoordinateEnum::RightToLeft,
                                                         AnatomicalOrientation::CoordinateEnum::SuperiorToInferior,
                                                         AnatomicalOrientation::CoordinateEnum::PosteriorToAnterior);
      break;
    case GE_SAGITTAL:
      hdr->coordinateOrientation = AnatomicalOrientation(AnatomicalOrientation::CoordinateEnum::AnteriorToPosterior,
                                                         AnatomicalOrientation::CoordinateEnum::InferiorToSuperior,
                                                         AnatomicalOrientation::CoordinateEnum::RightToLeft);
      break;
    case GE_AXIAL:
      hdr->coordinateOrientation = AnatomicalOrientation(AnatomicalOrientation::CoordinateEnum::RightToLeft,
                                                         AnatomicalOrientation::CoordinateEnum::AnteriorToPosterior,
                                                         AnatomicalOrientation::CoordinateEnum::InferiorToSuperior);
      break;
    default:
      hdr->coordinateOrientation = AnatomicalOrientation(AnatomicalOrientation::CoordinateEnum::RightToLeft,
                                                         AnatomicalOrientation::CoordinateEnum::SuperiorToInferior,
                                                         AnatomicalOrientation::CoordinateEnum::PosteriorToAnterior);
      break;
  }
  this->GetFloatAt(f, GE_ADW_IM_LOC, &(hdr->sliceLocation));

  int tmpInt = 0;
  this->GetIntAt(f, GE_ADW_IM_TR, &tmpInt);
  hdr->TR = static_cast<float>(tmpInt) / 1000.0f;

  this->GetIntAt(f, GE_ADW_IM_TI, &tmpInt);
  hdr->TI = static_cast<float>(tmpInt) / 1000.0f;

  this->GetIntAt(f, GE_ADW_IM_TE, &tmpInt);
  hdr->TE = static_cast<float>(tmpInt) / 1000.0f;

  this->GetShortAt(f, GE_ADW_IM_NUMECHO, &(hdr->numberOfEchoes));

  this->GetShortAt(f, GE_ADW_IM_ECHONUM, &(hdr->echoNumber));

  float tmpFloat = NAN;
  this->GetFloatAt(f, GE_ADW_IM_NEX, &tmpFloat);

  hdr->NEX = static_cast<int>(tmpFloat);

  this->GetShortAt(f, GE_ADW_IM_MR_FLIP, &hdr->flipAngle);

  this->GetStringAt(f, GE_ADW_IM_PSDNAME, hdr->pulseSequence, 31);
  hdr->pulseSequence[31] = '\0';

  this->GetShortAt(f, GE_ADW_IM_SLQUANT, &(hdr->numberOfSlices));

  this->GetIntAt(f, GE_ADW_VARIABLE_HDR_LENGTH, &tmpInt);
  hdr->offset = GE_ADW_FIXED_HDR_LENGTH + tmpInt;

  strncpy(hdr->filename, FileNameToRead, IOCommon::ITK_MAXPATHLEN);
  hdr->filename[IOCommon::ITK_MAXPATHLEN] = '\0';

  return hdr;
}
} // end namespace itk

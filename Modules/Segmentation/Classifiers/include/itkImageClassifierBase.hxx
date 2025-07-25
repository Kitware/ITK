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
#ifndef itkImageClassifierBase_hxx
#define itkImageClassifierBase_hxx

namespace itk
{

template <typename TInputImage, typename TClassifiedImage>
void
ImageClassifierBase<TInputImage, TClassifiedImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfObjectMacro(InputImage);
  itkPrintSelfObjectMacro(ClassifiedImage);
}

template <typename TInputImage, typename TClassifiedImage>
void
ImageClassifierBase<TInputImage, TClassifiedImage>::GenerateData()
{
  this->Classify();
}

template <typename TInputImage, typename TClassifiedImage>
void
ImageClassifierBase<TInputImage, TClassifiedImage>::Classify()
{
  ClassifiedImagePointer classifiedImage = this->GetClassifiedImage();

  // Check if the an output buffer has been allocated
  if (!classifiedImage)
  {
    this->Allocate();

    // To trigger the pipeline process
    this->Modified();
  }

  // Set the iterators and the pixel type definition for the input image
  const InputImageConstPointer inputImage = this->GetInputImage();
  InputImageConstIterator      inIt(inputImage, inputImage->GetBufferedRegion());

  // Set the iterators and the pixel type definition for the classified image
  classifiedImage = this->GetClassifiedImage();

  ClassifiedImageIterator classifiedIt(classifiedImage, classifiedImage->GetBufferedRegion());

  // Set up the vector to store the image  data
  InputImagePixelType      inputImagePixel;
  ClassifiedImagePixelType outputClassifiedLabel;

  // Set up the storage containers to record the probability
  // measures for each class.
  const unsigned int numberOfClasses = this->GetNumberOfMembershipFunctions();

  std::vector<double> discriminantScores;
  discriminantScores.resize(numberOfClasses);

  // support progress methods/callbacks
  const SizeValueType totalPixels = inputImage->GetBufferedRegion().GetNumberOfPixels();
  SizeValueType       updateVisits = totalPixels / 10;
  if (updateVisits < 1)
  {
    updateVisits = 1;
  }
  int k = 0;

  for (inIt.GoToBegin(); !inIt.IsAtEnd(); ++inIt, ++classifiedIt, ++k)
  {
    if (!(k % updateVisits))
    {
      this->UpdateProgress(static_cast<float>(k) / static_cast<float>(totalPixels));
    }

    // Read the input vector
    inputImagePixel = inIt.Get();
    for (unsigned int classIndex = 0; classIndex < numberOfClasses; ++classIndex)
    {
      discriminantScores[classIndex] = (this->GetMembershipFunction(classIndex))->Evaluate(inputImagePixel);
    }

    auto classLabel = static_cast<unsigned int>(this->GetDecisionRule()->Evaluate(discriminantScores));

    outputClassifiedLabel = ClassifiedImagePixelType(classLabel);
    classifiedIt.Set(outputClassifiedLabel);
  }
}

template <typename TInputImage, typename TClassifiedImage>
void
ImageClassifierBase<TInputImage, TClassifiedImage>::Allocate()
{
  const InputImageConstPointer inputImage = this->GetInputImage();

  const InputImageSizeType inputImageSize = inputImage->GetBufferedRegion().GetSize();

  const ClassifiedImagePointer classifiedImage = TClassifiedImage::New();

  this->SetClassifiedImage(classifiedImage);

  const typename TClassifiedImage::RegionType classifiedImageRegion(inputImageSize);

  classifiedImage->SetLargestPossibleRegion(classifiedImageRegion);
  classifiedImage->SetBufferedRegion(classifiedImageRegion);
  classifiedImage->Allocate();
}

template <typename TInputImage, typename TClassifiedImage>
std::vector<double>
ImageClassifierBase<TInputImage, TClassifiedImage>::GetPixelMembershipValue(const InputImagePixelType inputImagePixel)
{
  const unsigned int  numberOfClasses = this->GetNumberOfClasses();
  std::vector<double> pixelMembershipValue(numberOfClasses);

  for (unsigned int classIndex = 0; classIndex < numberOfClasses; ++classIndex)
  {
    pixelMembershipValue[classIndex] = (this->GetMembershipFunction(classIndex))->Evaluate(inputImagePixel);
  }

  return pixelMembershipValue;
}
} // namespace itk

#endif

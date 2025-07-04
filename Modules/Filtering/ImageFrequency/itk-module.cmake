set(
  DOCUMENTATION
  "This module contains classes to iterate over images in the frequency domain,
and filters to apply band pass filters based on frequency.

A more detailed description can be found in the Insight Journal article::

Cerdan, P.H. \"ITK Wavelet Module\".
  https://doi.org/10.54294/0e1c0748fe24
  September, 2016.
"
)

itk_module(
  ITKImageFrequency
  DEPENDS
  ITKCommon
  TEST_DEPENDS
  ITKTestKernel
  ITKImageIntensity
  ITKFFT
  DESCRIPTION
  "${DOCUMENTATION}"
)

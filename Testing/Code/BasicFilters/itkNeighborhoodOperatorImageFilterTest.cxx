/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkNeighborhoodOperatorImageFilterTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include <iostream>
#include "itkImage.h"
#include "itkNeighborhoodOperatorImageFilter.h"
#include "itkDerivativeOperator.h"
#include "itkNullImageToImageFilterDriver.txx"

int main(int argc, char *argv[])
{
  try
    {
      typedef  itk::Image<float, 3> ImageType;
      // Set up operator
      itk::DerivativeOperator<float, 3> oper;
      oper.SetOrder(1);
      oper.SetDirection(2);
      oper.CreateDirectional();

      // Set up filter
      itk::NeighborhoodOperatorImageFilter<ImageType, ImageType>::Pointer filter
        = itk::NeighborhoodOperatorImageFilter<ImageType, ImageType>::New();
      filter->SetOperator(oper);

      // Run Test
      itk::Size<3> sz;
      sz[0]=256;
      sz[1]=256;
      sz[2]=5;
      itk::NullImageToImageFilterDriver< ImageType, ImageType >
        test1;
      test1.SetImageSize(sz);
      test1.SetFilter(filter.GetPointer());
      test1.Execute();
    }
  catch(itk::ExceptionObject &err)
    {
      (&err)->Print(std::cerr);
      return 1;
    } 
  return 0;   
}

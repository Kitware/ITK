set(
  ITKReview_SRC
  itkVoxBoCUBImageIO.cxx
  itkVoxBoCUBImageIOFactory.cxx
)

add_library(ITKReview ${ITKReview_SRC})
itk_module_link_dependencies()
target_link_libraries(
  ITKReview
  LINK_PUBLIC
    ${ITKTestKernel_LIBRARIES}
    ${ITKIOLSM_LIBRARIES}
    itkopenjpeg
)
itk_module_target(ITKReview)

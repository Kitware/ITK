itk_module_test()
set(ITKIOTransformMatlabTests itkIOTransformMatlabTest.cxx)

createtestdriver(ITKIOTransformMatlab "${ITKIOTransformMatlab-Test_LIBRARIES}" "${ITKIOTransformMatlabTests}")

itk_add_test(
  NAME
  itkIOTransformMatlabTest
  COMMAND
  ITKIOTransformMatlabTestDriver
  itkIOTransformMatlabTest
)

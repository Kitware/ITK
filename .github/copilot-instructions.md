# ITK (Insight Toolkit) for Development Instructions for Copilot AI

## Project Overview

ITK is an open-source, cross-platform C++ toolkit for N-dimensional scientific image processing, segmentation, and registration.


## Repository Structure

- `/Modules/`: Core ITK modules organized by functionality
  - `/Core/`: Fundamental classes and algorithms
  - `/Filtering/`: Image processing filters
  - `/IO/`: Input/output for various image formats
  - `/Registration/`: Image registration algorithms
  - `/Segmentation/`: Image segmentation algorithms
  - `/Numerics/`: Numerical algorithms and optimizers
  - `/ThirdParty/`: External dependencies
- `/Examples/`: Example code demonstrating ITK usage
- `/Testing/`: Test data and testing infrastructure
- `/Wrapping/`: Python and other language bindings
- `/Documentation/`: User guides and API documentation
- `/CMake/`: CMake configuration files and macros
- `/Utilities/`: Development tools and scripts

For directly paths like '/Modules/Core/Common' the 'Common' part has the module name of 'ITKCommon' and 'Core' would be the group. Notable exception is that modules in the IO group include that in the module name such as ITKIOImageBase.

## Development Workflow

### Git Workflow - Topic Branch Model
- DO NOT commit to the main branch.
- DO NOT push to the official repository.
- Branch names should be concise but descriptive (like function names)

### Create a PR
- Target branch `upstream/main` for features or `upstream/release` for backports.
- Fetch upstream. Then create new topic branch based on current upstream target branch.
- Make concise commits, following the commit message standards.
- Push branches to user's remote, NOT the official repository.
- Create a DRAFT pull request against the target branch.


## Build and Testing

DO NOTE USE pixie for development.

Use the CMakePresets.json and CMakeUserPresets.json files to configure the build environment: `cmake --preset PRESET_NAME` to configure the build and `cmake --build --preset PRESET_NAME` to build.

The full compilation of ITK and it's test suit is time consuming. Building specific targets can speed the development process.

To build targets `cmake --build --preset PRESET_NAME --target TARGET` should be used.

Each module name is also a build target for the module which include tests targets.

The test drivers are named such as ITKCommon1TestDriver, ITKCommonGTestDriver, or ITKIOImageBaseTestDriver.

Tests should be run with the `ctest --preset PRESET_NAME` command.

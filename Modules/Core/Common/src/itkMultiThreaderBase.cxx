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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#include "itkMultiThreaderBase.h"
#include "itkPlatformMultiThreader.h"

#if defined(ITK_USE_POOL_MULTI_THREADER)
#  include "itkPoolMultiThreader.h"
#endif
#include "itkNumericTraits.h"
#include <mutex>

#include "itksys/SystemTools.hxx"
#include "itksys/SystemInformation.hxx"
#include "itkImageSourceCommon.h"
#include "itkSingleton.h"
#include "itkProcessObject.h"

#include <algorithm> // For clamp.
#include <iostream>
#include <string>
#include <cctype>
#include <utility> // For move.

#if defined(ITK_HAS_SCHED_GETAFFINITY)
#  include <sched.h>
#endif

#if defined(ITK_USE_TBB)
#  include "itkTBBMultiThreader.h"
#endif

#include "itkTotalProgressReporter.h"

namespace itk
{

struct MultiThreaderBaseGlobals
{
  // Initialize static members.
  MultiThreaderBaseGlobals() = default;

  // GlobalDefaultThreaderTypeIsInitialized is used only in this
  // file to ensure that the ITK_GLOBAL_DEFAULT_THREADER or
  // ITK_USE_THREADPOOL environmenal variables are
  // only used as a fall back option.  If the SetGlobalDefaultThreader
  // API is ever used by the developer, the developers choice is
  // respected over the environmental variable.
  bool       GlobalDefaultThreaderTypeIsInitialized{ false };
  std::mutex globalDefaultInitializerMutex;

  // Global value to control which threader to be used by default. First it is initialized with the default preprocessor
  // definition from CMake configuration value, for compile time control of default. This initial value can be
  // overridden by the environmental variable ITK_GLOBAL_DEFAULT_THREADER.
  MultiThreaderBase::ThreaderEnum m_GlobalDefaultThreader{ MultiThreaderBase::ThreaderEnum::ITK_DEFAULT_THREADER };

  // Global variable defining the maximum number of threads that can be used.
  //  The m_GlobalMaximumNumberOfThreads must always be less than or equal to
  //  ITK_MAX_THREADS and greater than zero. */
  ThreadIdType m_GlobalMaximumNumberOfThreads{ ITK_MAX_THREADS };

  //  Global variable defining the default number of threads to set at
  //  construction time of a MultiThreaderBase instance.  The
  //  m_GlobalDefaultNumberOfThreads must always be less than or equal to the
  //  m_GlobalMaximumNumberOfThreads and larger or equal to 1 once it has been
  //  initialized in the constructor of the first MultiThreaderBase instantiation.
  ThreadIdType m_GlobalDefaultNumberOfThreads{ 0 };
};

itkGetGlobalSimpleMacro(MultiThreaderBase, MultiThreaderBaseGlobals, PimplGlobals);


#if !defined(ITK_LEGACY_REMOVE)
void
MultiThreaderBase::SetGlobalDefaultUseThreadPool(const bool GlobalDefaultUseThreadPool)
{
  if (GlobalDefaultUseThreadPool)
  {
    SetGlobalDefaultThreader(ThreaderEnum::Pool);
  }
  else
  {
    SetGlobalDefaultThreader(ThreaderEnum::Platform);
  }
}

bool
MultiThreaderBase::GetGlobalDefaultUseThreadPool()
{
  return (GetGlobalDefaultThreader() == ThreaderEnum::Pool);
}
#endif

void
MultiThreaderBase::SetGlobalDefaultThreaderPrivate(ThreaderEnum threaderType)
{
  // m_PimplGlobals->globalDefaultInitializerMutex must be already held here!

  m_PimplGlobals->m_GlobalDefaultThreader = threaderType;
  m_PimplGlobals->GlobalDefaultThreaderTypeIsInitialized = true;
}

void
MultiThreaderBase::SetGlobalDefaultThreader(ThreaderEnum threaderType)
{
  itkInitGlobalsMacro(PimplGlobals);

  // Acquire mutex then call private method to do the real work.
  const std::lock_guard<std::mutex> lockGuard(m_PimplGlobals->globalDefaultInitializerMutex);

  MultiThreaderBase::SetGlobalDefaultThreaderPrivate(threaderType);
}

MultiThreaderBase::ThreaderEnum
MultiThreaderBase::GetGlobalDefaultThreaderPrivate()
{
  // m_PimplGlobals->globalDefaultInitializerMutex must be already held here!

  if (!m_PimplGlobals->GlobalDefaultThreaderTypeIsInitialized)
  {
    std::string envVar;
    // first check ITK_GLOBAL_DEFAULT_THREADER
    if (itksys::SystemTools::GetEnv("ITK_GLOBAL_DEFAULT_THREADER", envVar))
    {
      envVar = itksys::SystemTools::UpperCase(envVar);
      const ThreaderEnum threaderT = ThreaderTypeFromString(envVar);
      if (threaderT != ThreaderEnum::Unknown)
      {
        MultiThreaderBase::SetGlobalDefaultThreaderPrivate(threaderT);
      }
    }
    // if that was not set check ITK_USE_THREADPOOL (deprecated)
    else if (!m_PimplGlobals->GlobalDefaultThreaderTypeIsInitialized &&
             itksys::SystemTools::GetEnv("ITK_USE_THREADPOOL", envVar))
    {
      envVar = itksys::SystemTools::UpperCase(envVar);
      itkGenericOutputMacro("Warning: ITK_USE_THREADPOOL has been deprecated since ITK v5.0. You should now use "
                            "ITK_GLOBAL_DEFAULT_THREADER\nFor example ITK_GLOBAL_DEFAULT_THREADER=Pool");
      if (envVar != "NO" && envVar != "OFF" && envVar != "FALSE")
      {
#ifdef __EMSCRIPTEN__
        MultiThreaderBase::SetGlobalDefaultThreaderPrivate(ThreaderEnum::Platform);
#else
        MultiThreaderBase::SetGlobalDefaultThreaderPrivate(ThreaderEnum::Pool);
#endif
      }
      else
      {
        MultiThreaderBase::SetGlobalDefaultThreaderPrivate(ThreaderEnum::Platform);
      }
    }

    // always set that we are initialized
    m_PimplGlobals->GlobalDefaultThreaderTypeIsInitialized = true;
  }
  return m_PimplGlobals->m_GlobalDefaultThreader;
}

MultiThreaderBase::ThreaderEnum
MultiThreaderBase::GetGlobalDefaultThreader()
{
  itkInitGlobalsMacro(PimplGlobals);

  // Acquire mutex then call private method to do the real work.
  const std::lock_guard<std::mutex> lockGuard(m_PimplGlobals->globalDefaultInitializerMutex);

  return MultiThreaderBase::GetGlobalDefaultThreaderPrivate();
}

MultiThreaderBase::ThreaderEnum
MultiThreaderBase::ThreaderTypeFromString(std::string threaderString)
{
  threaderString = itksys::SystemTools::UpperCase(threaderString);
  if (threaderString == "PLATFORM")
  {
    return ThreaderEnum::Platform;
  }
  if (threaderString == "POOL")
  {
    return ThreaderEnum::Pool;
  }
  else if (threaderString == "TBB")
  {
    return ThreaderEnum::TBB;
  }
  else
  {
    return ThreaderEnum::Unknown;
  }
}

void
MultiThreaderBase::SetGlobalMaximumNumberOfThreads(ThreadIdType val)
{
  itkInitGlobalsMacro(PimplGlobals);

  m_PimplGlobals->m_GlobalMaximumNumberOfThreads = std::clamp<ThreadIdType>(val, 1, ITK_MAX_THREADS);

  // If necessary reset the default to be used from now on.
  MultiThreaderBase::SetGlobalDefaultNumberOfThreads(
    std::min(Self::GetGlobalDefaultNumberOfThreads(), Self::GetGlobalMaximumNumberOfThreads()));
}

ThreadIdType
MultiThreaderBase::GetGlobalMaximumNumberOfThreads()
{
  itkInitGlobalsMacro(PimplGlobals);
  return m_PimplGlobals->m_GlobalMaximumNumberOfThreads;
}

void
MultiThreaderBase::SetGlobalDefaultNumberOfThreads(ThreadIdType val)
{
  itkInitGlobalsMacro(PimplGlobals);

  const std::lock_guard<std::mutex> lockGuard(m_PimplGlobals->globalDefaultInitializerMutex);

  m_PimplGlobals->m_GlobalDefaultNumberOfThreads =
    std::clamp<ThreadIdType>(val, 1, m_PimplGlobals->m_GlobalMaximumNumberOfThreads);
}

void
MultiThreaderBase::SetMaximumNumberOfThreads(ThreadIdType numberOfThreads)
{
  m_MaximumNumberOfThreads =
    std::clamp<ThreadIdType>(numberOfThreads, 1, m_PimplGlobals->m_GlobalMaximumNumberOfThreads);
}

void
MultiThreaderBase::SetNumberOfWorkUnits(ThreadIdType numberOfWorkUnits)
{
  m_NumberOfWorkUnits = std::clamp<ThreadIdType>(numberOfWorkUnits, 1, m_PimplGlobals->m_GlobalMaximumNumberOfThreads);
}

void
MultiThreaderBase::SetUpdateProgress(bool updates)
{
  this->m_UpdateProgress = updates;
}

ThreadIdType
MultiThreaderBase::GetGlobalDefaultNumberOfThreads()
{
  itkInitGlobalsMacro(PimplGlobals);

  const std::lock_guard<std::mutex> lockGuard(m_PimplGlobals->globalDefaultInitializerMutex);

  if (m_PimplGlobals->m_GlobalDefaultNumberOfThreads == 0) // need to initialize
  {
    ThreadIdType threadCount = 0;
    /* The ITK_NUMBER_OF_THREADS_ENV_LIST contains is an
     * environmental variable that holds a ':' separated
     * list of environmental variables that would be
     * queried in order for setting the m_GlobalMaximumNumberOfThreads.
     *
     * This is intended to be a mechanism suitable to easy
     * runtime modification to ease using the proper number
     * of threads for load balancing batch processing
     * systems where the number of threads
     * authorized for use may be less than the number
     * of physical processors on the computer.
     *
     * This list contains the Sun|Oracle Grid Engine
     * environmental variable "NSLOTS" by default
     */
    std::vector<std::string> ITK_NUMBER_OF_THREADS_ENV_LIST;
    std::string              itkNumberOfThreadsEvnListString = "";
    if (itksys::SystemTools::GetEnv("ITK_NUMBER_OF_THREADS_ENV_LIST", itkNumberOfThreadsEvnListString))
    {
      // NOTE: We always put "ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS" at the end
      // unconditionally.
      itkNumberOfThreadsEvnListString += ":ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS";
    }
    else
    {
      itkNumberOfThreadsEvnListString = "NSLOTS:ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS";
    }
    {
      std::stringstream numberOfThreadsEnvListStream(itkNumberOfThreadsEvnListString);
      std::string       item;
      while (std::getline(numberOfThreadsEnvListStream, item, ':'))
      {
        if (!item.empty()) // Do not add empty items.
        {
          ITK_NUMBER_OF_THREADS_ENV_LIST.push_back(item);
        }
      }
    }
    // first, check for environment variable
    std::string itkGlobalDefaultNumberOfThreadsEnv = "0";
    for (const auto & lit : ITK_NUMBER_OF_THREADS_ENV_LIST)
    {
      if (itksys::SystemTools::GetEnv(lit.c_str(), itkGlobalDefaultNumberOfThreadsEnv))
      {
        threadCount = static_cast<ThreadIdType>(atoi(itkGlobalDefaultNumberOfThreadsEnv.c_str()));
      }
    }

    // otherwise, set number of threads based on system information
    if (threadCount <= 0)
    {
      threadCount = GetGlobalDefaultNumberOfThreadsByPlatform();
    }

    // limit the number of threads to m_GlobalMaximumNumberOfThreads
    // verify that the default number of threads is larger than zero
    threadCount = std::clamp(threadCount, ThreadIdType{ 1 }, ThreadIdType{ ITK_MAX_THREADS });

    m_PimplGlobals->m_GlobalDefaultNumberOfThreads = threadCount;
  }
  return m_PimplGlobals->m_GlobalDefaultNumberOfThreads;
}

ThreadIdType
MultiThreaderBase::GetGlobalDefaultNumberOfThreadsByPlatform()
{
#if defined(ITK_HAS_SCHED_GETAFFINITY)
  cpu_set_t mask;
  if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) != -1)
  {
    return static_cast<ThreadIdType>(CPU_COUNT(&mask));
  }
#endif

#if defined(ITK_LEGACY_REMOVE)
  return std::thread::hardware_concurrency();
#endif

#if defined(ITK_USE_PTHREADS)

// Default the number of threads to be the number of available
// processors if we are using pthreads()
#  ifdef _SC_NPROCESSORS_ONLN
  auto num = static_cast<ThreadIdType>(sysconf(_SC_NPROCESSORS_ONLN));
#  elif defined(_SC_NPROC_ONLN)
  ThreadIdType num = static_cast<ThreadIdType>(sysconf(_SC_NPROC_ONLN));
#  else
  ThreadIdType num = 1;
#  endif

  itksys::SystemInformation mySys;
  mySys.RunCPUCheck();
  const int result = mySys.GetNumberOfPhysicalCPU(); // Avoid using hyperthreading cores.
  if (result == -1)
  {
    num = 1;
  }
  return num;
#elif defined(ITK_USE_WIN32_THREADS)
  SYSTEM_INFO sysInfo;

  GetSystemInfo(&sysInfo);
  ThreadIdType num = sysInfo.dwNumberOfProcessors;
  return num;
#else
  return 1;
#endif
} // namespace itk

MultiThreaderBase::Pointer
MultiThreaderBase::New()
{
  Pointer smartPtr = itk::ObjectFactory<MultiThreaderBase>::Create();
  if (smartPtr == nullptr)
  {
    const ThreaderEnum threaderType = GetGlobalDefaultThreader();
    switch (threaderType)
    {
      case ThreaderEnum::Platform:
        return PlatformMultiThreader::New();
      case ThreaderEnum::Pool:
#if defined(ITK_USE_POOL_MULTI_THREADER)
        return PoolMultiThreader::New();
#else
        itkGenericExceptionMacro("ITK has been built without PoolMultiThreader support!");
#endif
      case ThreaderEnum::TBB:
#if defined(ITK_USE_TBB)
        return TBBMultiThreader::New();
#else
        itkGenericExceptionMacro("ITK has been built without TBB support!");
#endif
      default:
        itkGenericExceptionMacro("MultiThreaderBase::GetGlobalDefaultThreader returned Unknown!");
    }
  }
  smartPtr->UnRegister();
  return smartPtr;
}


MultiThreaderBase::MultiThreaderBase()
  : m_MaximumNumberOfThreads(MultiThreaderBase::GetGlobalDefaultNumberOfThreads())
{
  m_NumberOfWorkUnits = m_MaximumNumberOfThreads;
}

MultiThreaderBase::~MultiThreaderBase() = default;

ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION
MultiThreaderBase::SingleMethodProxy(void * arg)
{
  // grab the WorkUnitInfo originally prescribed
  auto * workUnitInfoStruct = static_cast<MultiThreaderBase::WorkUnitInfo *>(arg);

  // execute the user specified threader callback, catching any exceptions
  try
  {
    (*workUnitInfoStruct->ThreadFunction)(arg);
    workUnitInfoStruct->ThreadExitCode = WorkUnitInfo::ThreadExitCodeEnum::SUCCESS;
  }
  catch (const ProcessAborted &)
  {
    workUnitInfoStruct->ThreadExitCode = WorkUnitInfo::ThreadExitCodeEnum::ITK_PROCESS_ABORTED_EXCEPTION;
  }
  catch (const ExceptionObject &)
  {
    workUnitInfoStruct->ThreadExitCode = WorkUnitInfo::ThreadExitCodeEnum::ITK_EXCEPTION;
  }
  catch (const std::exception &)
  {
    workUnitInfoStruct->ThreadExitCode = WorkUnitInfo::ThreadExitCodeEnum::STD_EXCEPTION;
  }
  catch (...)
  {
    workUnitInfoStruct->ThreadExitCode = WorkUnitInfo::ThreadExitCodeEnum::UNKNOWN;
  }

  return ITK_THREAD_RETURN_DEFAULT_VALUE;
}


void
MultiThreaderBase::SetSingleMethodAndExecute(ThreadFunctionType func, void * data)
{
  this->SetSingleMethod(std::move(func), data);
  this->SingleMethodExecute();
}


void
MultiThreaderBase::ParallelizeArray(SizeValueType             firstIndex,
                                    SizeValueType             lastIndexPlus1,
                                    ArrayThreadingFunctorType aFunc,
                                    ProcessObject *           filter)
{
  // This implementation simply delegates parallelization to the old interface
  // SetSingleMethod+SingleMethodExecute. This method is meant to be overloaded!

  if (!this->GetUpdateProgress())
  {
    filter = nullptr;
  }
  // Upon destruction, progress will be set to 1.0
  const ProgressReporter progress(filter, 0, 1);

  if (firstIndex + 1 < lastIndexPlus1)
  {
    struct ArrayCallback acParams{ aFunc, firstIndex, lastIndexPlus1, filter };
    this->SetSingleMethodAndExecute(&MultiThreaderBase::ParallelizeArrayHelper, &acParams);
  }
  else if (firstIndex + 1 == lastIndexPlus1)
  {
    aFunc(firstIndex);
  }
  // else nothing needs to be executed
}

ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION
MultiThreaderBase::ParallelizeArrayHelper(void * arg)
{
  auto *             workUnitInfo = static_cast<MultiThreaderBase::WorkUnitInfo *>(arg);
  const ThreadIdType workUnitID = workUnitInfo->WorkUnitID;
  const ThreadIdType workUnitCount = workUnitInfo->NumberOfWorkUnits;
  auto *             acParams = static_cast<struct ArrayCallback *>(workUnitInfo->UserData);

  const SizeValueType range = acParams->lastIndexPlus1 - acParams->firstIndex;
  const double        fraction = static_cast<double>(range) / workUnitCount;
  const SizeValueType first = acParams->firstIndex + fraction * workUnitID;
  SizeValueType       afterLast = acParams->firstIndex + fraction * (workUnitID + 1);
  if (workUnitID == workUnitCount - 1) // last thread
  {
    // Avoid possible problems due to floating point arithmetic
    afterLast = acParams->lastIndexPlus1;
  }

  TotalProgressReporter reporter(acParams->filter, range);

  for (SizeValueType i = first; i < afterLast; ++i)
  {
    acParams->functor(i);

    reporter.CompletedPixel();
  }

  return ITK_THREAD_RETURN_DEFAULT_VALUE;
}


void
MultiThreaderBase::ParallelizeImageRegion(unsigned int                            dimension,
                                          const IndexValueType                    index[],
                                          const SizeValueType                     size[],
                                          MultiThreaderBase::ThreadingFunctorType funcP,
                                          ProcessObject *                         filter)
{
  // This implementation simply delegates parallelization to the old interface
  // SetSingleMethod+SingleMethodExecute. This method is meant to be overloaded!
  if (!this->GetUpdateProgress())
  {
    filter = nullptr;
  }
  const ProgressReporter progress(filter, 0, 1);

  struct RegionAndCallback rnc{ funcP, dimension, index, size, filter };
  this->SetSingleMethodAndExecute(&MultiThreaderBase::ParallelizeImageRegionHelper, &rnc);
}

ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION
MultiThreaderBase::ParallelizeImageRegionHelper(void * arg)
{
  auto *             workUnitInfo = static_cast<MultiThreaderBase::WorkUnitInfo *>(arg);
  const ThreadIdType workUnitID = workUnitInfo->WorkUnitID;
  const ThreadIdType workUnitCount = workUnitInfo->NumberOfWorkUnits;
  auto *             rnc = static_cast<struct RegionAndCallback *>(workUnitInfo->UserData);

  const ImageRegionSplitterBase * splitter = ImageSourceCommon::GetGlobalDefaultSplitter();
  ImageIORegion                   region(rnc->dimension);
  for (unsigned int d = 0; d < rnc->dimension; ++d)
  {
    region.SetIndex(d, rnc->index[d]);
    region.SetSize(d, rnc->size[d]);
  }
  const ThreadIdType total = splitter->GetSplit(workUnitID, workUnitCount, region);

  TotalProgressReporter reporter(rnc->filter, 0);

  if (workUnitID < total)
  {
    rnc->functor(&region.GetIndex()[0], &region.GetSize()[0]);

    reporter.Completed(region.GetNumberOfPixels());
  }

  return ITK_THREAD_RETURN_DEFAULT_VALUE;
}

// Print method for the multithreader
void
MultiThreaderBase::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Number of Work Units: " << m_NumberOfWorkUnits << '\n';
  os << indent << "Number of Threads: " << m_MaximumNumberOfThreads << '\n';
  os << indent << "Global Maximum Number Of Threads: " << m_PimplGlobals->m_GlobalMaximumNumberOfThreads << std::endl;
  os << indent << "Global Default Number Of Threads: " << m_PimplGlobals->m_GlobalDefaultNumberOfThreads << std::endl;
  os << indent << "Global Default Threader Type: " << m_PimplGlobals->m_GlobalDefaultThreader << std::endl;
  os << indent << "SingleMethod: " << m_SingleMethod << std::endl;
  os << indent << "SingleData: " << m_SingleData << std::endl;
}

MultiThreaderBaseGlobals * MultiThreaderBase::m_PimplGlobals;

/** Print enum values */
std::ostream &
operator<<(std::ostream & out, const MultiThreaderBaseEnums::Threader value)
{
  return out << [value] {
    switch (value)
    {
      case MultiThreaderBaseEnums::Threader::Platform:
        return "itk::MultiThreaderBaseEnums::Threader::Platform";
        //      TODO    case MultiThreaderBaseEnums::Threader::First:
        //                    return "itk::MultiThreaderBaseEnums::Threader::First";
      case MultiThreaderBaseEnums::Threader::Pool:
        return "itk::MultiThreaderBaseEnums::Threader::Pool";
      case MultiThreaderBaseEnums::Threader::TBB:
        return "itk::MultiThreaderBaseEnums::Threader::TBB";
        //      TODO    case MultiThreaderBaseEnums::Threader::Last:
        //                    return "itk::MultiThreaderBaseEnums::Threader::Last";
      case MultiThreaderBaseEnums::Threader::Unknown:
        return "itk::MultiThreaderBaseEnums::Threader::Unknown";
      default:
        return "INVALID VALUE FOR itk::MultiThreaderBaseEnums::Threader";
    }
  }();
}
/** Print enum values */
std::ostream &
operator<<(std::ostream & out, const MultiThreaderBaseEnums::ThreadExitCode value)
{
  return out << [value] {
    switch (value)
    {
      case MultiThreaderBaseEnums::ThreadExitCode::SUCCESS:
        return "itk::MultiThreaderBaseEnums::ThreadExitCode::SUCCESS";
      case MultiThreaderBaseEnums::ThreadExitCode::ITK_EXCEPTION:
        return "itk::MultiThreaderBaseEnums::ThreadExitCode::ITK_EXCEPTION";
      case MultiThreaderBaseEnums::ThreadExitCode::ITK_PROCESS_ABORTED_EXCEPTION:
        return "itk::MultiThreaderBaseEnums::ThreadExitCode::ITK_PROCESS_ABORTED_EXCEPTION";
      case MultiThreaderBaseEnums::ThreadExitCode::STD_EXCEPTION:
        return "itk::MultiThreaderBaseEnums::ThreadExitCode::STD_EXCEPTION";
      case MultiThreaderBaseEnums::ThreadExitCode::UNKNOWN:
        return "itk::MultiThreaderBaseEnums::ThreadExitCode::UNKNOWN";
      default:
        return "INVALID VALUE FOR itk::MultiThreaderBaseEnums::ThreadExitCode";
    }
  }();
}
} // namespace itk

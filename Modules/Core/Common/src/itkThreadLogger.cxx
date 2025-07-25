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
#include <iostream>
#include "itkThreadLogger.h"
#include "itksys/SystemTools.hxx"

namespace itk
{

ThreadLogger::ThreadLogger()
  : m_TerminationRequested(false) // ms
  , m_Delay(300)
{
  m_Thread = std::thread(&ThreadLogger::ThreadFunction, this);
}

ThreadLogger::~ThreadLogger()
{
  if (m_Thread.joinable())
  {
    m_TerminationRequested = true;
    m_Thread.join(); // waits for it to finish if necessary
  }
}

void
ThreadLogger::SetPriorityLevel(PriorityLevelEnum level)
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  this->m_OperationQ.push(SET_PRIORITY_LEVEL);
  this->m_LevelQ.push(level);
}

Logger::PriorityLevelEnum
ThreadLogger::GetPriorityLevel() const
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  const PriorityLevelEnum           level = this->m_PriorityLevel;
  return level;
}

void
ThreadLogger::SetLevelForFlushing(PriorityLevelEnum level)
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  this->m_LevelForFlushing = level;
  this->m_OperationQ.push(SET_LEVEL_FOR_FLUSHING);
  this->m_LevelQ.push(level);
}

Logger::PriorityLevelEnum
ThreadLogger::GetLevelForFlushing() const
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  const PriorityLevelEnum           level = this->m_LevelForFlushing;
  return level;
}

void
ThreadLogger::SetDelay(DelayType delay)
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  this->m_Delay = delay;
}

ThreadLogger::DelayType
ThreadLogger::GetDelay() const
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  const DelayType                   delay = this->m_Delay;
  return delay;
}

void
ThreadLogger::AddLogOutput(OutputType * output)
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  this->m_OperationQ.push(ADD_LOG_OUTPUT);
  this->m_OutputQ.emplace(output);
}

void
ThreadLogger::Write(PriorityLevelEnum level, const std::string & content)
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  this->m_OperationQ.push(WRITE);
  this->m_MessageQ.push(content);
  this->m_LevelQ.push(level);
  if (this->m_LevelForFlushing >= level)
  {
    this->InternalFlush();
  }
}

void
ThreadLogger::Flush()
{
  const std::lock_guard<std::mutex> lockGuard(m_Mutex);
  this->m_OperationQ.push(FLUSH);
  this->InternalFlush();
}

void
ThreadLogger::InternalFlush()
{
  // m_Mutex must already be held here!

  while (!this->m_OperationQ.empty())
  {
    switch (this->m_OperationQ.front())
    {
      case ThreadLogger::SET_PRIORITY_LEVEL:
        this->m_PriorityLevel = this->m_LevelQ.front();
        this->m_LevelQ.pop();
        break;

      case ThreadLogger::SET_LEVEL_FOR_FLUSHING:
        this->m_LevelForFlushing = this->m_LevelQ.front();
        this->m_LevelQ.pop();
        break;

      case ThreadLogger::ADD_LOG_OUTPUT:
        this->m_Output->AddLogOutput(this->m_OutputQ.front());
        this->m_OutputQ.pop();
        break;

      case ThreadLogger::WRITE:
        this->Logger::Write(this->m_LevelQ.front(), this->m_MessageQ.front());
        this->m_LevelQ.pop();
        this->m_MessageQ.pop();
        break;
      case ThreadLogger::FLUSH:
        this->Logger::Flush();
        break;
    }
    this->m_OperationQ.pop();
  }
  this->m_Output->Flush();
}

void
ThreadLogger::ThreadFunction()
{
  while (!m_TerminationRequested)
  {
    {
      const std::lock_guard<std::mutex> lockGuard(m_Mutex);
      while (!m_OperationQ.empty())
      {
        switch (m_OperationQ.front())
        {
          case ThreadLogger::SET_PRIORITY_LEVEL:
            m_PriorityLevel = m_LevelQ.front();
            m_LevelQ.pop();
            break;

          case ThreadLogger::SET_LEVEL_FOR_FLUSHING:
            m_LevelForFlushing = m_LevelQ.front();
            m_LevelQ.pop();
            break;

          case ThreadLogger::ADD_LOG_OUTPUT:
            m_Output->AddLogOutput(m_OutputQ.front());
            m_OutputQ.pop();
            break;

          case ThreadLogger::WRITE:
            Logger::Write(m_LevelQ.front(), m_MessageQ.front());
            m_LevelQ.pop();
            m_MessageQ.pop();
            break;
          case ThreadLogger::FLUSH:
            Logger::Flush();
            break;
        }
        m_OperationQ.pop();
      }
    } // end of scope of lockGuard (unlocking m_Mutex automatically)

    itksys::SystemTools::Delay(this->GetDelay());
  }
}

void
ThreadLogger::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Thread ID: " << m_Thread.get_id() << std::endl;
  os << indent << "TerminationRequested: " << m_TerminationRequested << std::endl;

  os << indent << "OperationQ size: " << m_OperationQ.size() << std::endl;
  os << indent << "MessageQ size: " << m_MessageQ.size() << std::endl;
  os << indent << "LevelQ size: " << m_LevelQ.size() << std::endl;
  os << indent << "OutputQ size: " << m_OutputQ.size() << std::endl;

  os << indent << "Delay: " << m_Delay << std::endl;
}

} // namespace itk

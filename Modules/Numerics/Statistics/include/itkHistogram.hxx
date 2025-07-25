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
#ifndef itkHistogram_hxx
#define itkHistogram_hxx

#include "itkNumericTraits.h"
#include "itkMath.h"
#include "itkPrintHelper.h"

namespace itk::Statistics
{
template <typename TMeasurement, typename TFrequencyContainer>
Histogram<TMeasurement, TFrequencyContainer>::Histogram()
  : m_Size(0)
  , m_OffsetTable(OffsetTableType(Superclass::GetMeasurementVectorSize() + 1))
  , m_FrequencyContainer(FrequencyContainerType::New())

{
  for (unsigned int i = 0; i < this->GetMeasurementVectorSize() + 1; ++i)
  {
    this->m_OffsetTable[i] = InstanceIdentifier{};
  }
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::Size() const -> InstanceIdentifier
{
  if (this->GetMeasurementVectorSize() == 0)
  {
    return InstanceIdentifier{};
  }
  InstanceIdentifier size = 1;
  for (unsigned int i = 0; i < this->GetMeasurementVectorSize(); ++i)
  {
    size *= m_Size[i];
  }
  return size;
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetSize() const -> const SizeType &
{
  return m_Size;
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetSize(unsigned int dimension) const -> SizeValueType
{
  return m_Size[dimension];
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetBinMin(unsigned int dimension, InstanceIdentifier nbin) const
  -> const MeasurementType &
{
  return m_Min[dimension][nbin];
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetBinMax(unsigned int dimension, InstanceIdentifier nbin) const
  -> const MeasurementType &
{
  return m_Max[dimension][nbin];
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::SetBinMin(unsigned int       dimension,
                                                        InstanceIdentifier nbin,
                                                        MeasurementType    min)
{
  m_Min[dimension][nbin] = min;
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::SetBinMax(unsigned int       dimension,
                                                        InstanceIdentifier nbin,
                                                        MeasurementType    max)
{
  m_Max[dimension][nbin] = max;
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetDimensionMins(unsigned int dimension) const -> const BinMinVectorType &
{
  return m_Min[dimension];
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetDimensionMaxs(unsigned int dimension) const -> const BinMaxVectorType &
{
  return m_Max[dimension];
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetMins() const -> const BinMinContainerType &
{
  return m_Min;
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetMaxs() const -> const BinMaxContainerType &
{
  return m_Max;
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetFrequency(InstanceIdentifier id) const -> AbsoluteFrequencyType
{
  return m_FrequencyContainer->GetFrequency(id);
}

template <typename TMeasurement, typename TFrequencyContainer>
bool
Histogram<TMeasurement, TFrequencyContainer>::SetFrequency(InstanceIdentifier id, AbsoluteFrequencyType value)
{
  return m_FrequencyContainer->SetFrequency(id, value);
}

template <typename TMeasurement, typename TFrequencyContainer>
bool
Histogram<TMeasurement, TFrequencyContainer>::IncreaseFrequency(InstanceIdentifier id, AbsoluteFrequencyType value)
{
  return m_FrequencyContainer->IncreaseFrequency(id, value);
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::Initialize(const SizeType & size)
{
  if (this->GetMeasurementVectorSize() == 0)
  {
    itkExceptionMacro("MeasurementVectorSize is Zero. It should be set to a non-zero value before calling Initialize");
  }

  this->m_Size = size;

  // creates offset table which will be used for generation of
  // instance identifiers.
  InstanceIdentifier num = 1;

  this->m_OffsetTable.resize(this->GetMeasurementVectorSize() + 1);

  this->m_OffsetTable[0] = num;
  for (unsigned int i = 0; i < this->GetMeasurementVectorSize(); ++i)
  {
    num *= m_Size[i];
    this->m_OffsetTable[i + 1] = num;
  }

  m_NumberOfInstances = num;

  // adjust the sizes of min max value containers
  m_Min.resize(this->GetMeasurementVectorSize());
  for (unsigned int dim = 0; dim < this->GetMeasurementVectorSize(); ++dim)
  {
    m_Min[dim].resize(m_Size[dim]);
  }

  m_Max.resize(this->GetMeasurementVectorSize());
  for (unsigned int dim = 0; dim < this->GetMeasurementVectorSize(); ++dim)
  {
    m_Max[dim].resize(m_Size[dim]);
  }

  // initialize auxiliary variables
  this->m_TempIndex.SetSize(this->GetMeasurementVectorSize());
  m_TempIndex.Fill(0);

  this->m_TempMeasurementVector.SetSize(this->GetMeasurementVectorSize());
  m_TempMeasurementVector.Fill(0);

  // initialize the frequency container
  m_FrequencyContainer->Initialize(this->m_OffsetTable[this->GetMeasurementVectorSize()]);
  this->SetToZero();
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::SetToZero()
{
  m_FrequencyContainer->SetToZero();
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::Initialize(const SizeType &        size,
                                                         MeasurementVectorType & lowerBound,
                                                         MeasurementVectorType & upperBound)
{
  this->Initialize(size);


  for (unsigned int i = 0; i < this->GetMeasurementVectorSize(); ++i)
  {
    if (size[i] > 0)
    {
      float interval =
        (static_cast<float>(upperBound[i]) - static_cast<float>(lowerBound[i])) / static_cast<float>(size[i]);

      // Set the min vector and max vector
      for (unsigned int j = 0; j < static_cast<unsigned int>(size[i] - 1); ++j)
      {
        this->SetBinMin(i, j, (MeasurementType)(lowerBound[i] + (static_cast<float>(j) * interval)));
        this->SetBinMax(i, j, (MeasurementType)(lowerBound[i] + ((static_cast<float>(j) + 1) * interval)));
      }
      this->SetBinMin(
        i, size[i] - 1, (MeasurementType)(lowerBound[i] + ((static_cast<float>(size[i]) - 1) * interval)));
      this->SetBinMax(i, size[i] - 1, (MeasurementType)(upperBound[i]));
    }
  }
}

/** */
template <typename TMeasurement, typename TFrequencyContainer>
bool
Histogram<TMeasurement, TFrequencyContainer>::GetIndex(const MeasurementVectorType & measurement,
                                                       IndexType &                   index) const
{
  // now using something similar to binary search to find
  // index.
  const unsigned int measurementVectorSize = this->GetMeasurementVectorSize();
  if (index.Size() != measurementVectorSize)
  {
    index.SetSize(measurementVectorSize);
  }


  for (unsigned int dim = 0; dim < measurementVectorSize; ++dim)
  {
    const MeasurementType tempMeasurement = measurement[dim];
    IndexValueType        begin = 0;
    if (tempMeasurement < m_Min[dim][begin])
    {
      // one of measurement is below the minimum
      // its ok if we extend the bins to infinity.. not ok if we don't
      if (!m_ClipBinsAtEnds)
      {
        index[dim] = static_cast<IndexValueType>(0);
        continue;
      }

      // set an illegal value and return 0
      index[dim] = (IndexValueType)m_Size[dim];
      return false;
    }

    IndexValueType end = static_cast<IndexValueType>(m_Min[dim].size()) - 1;
    if (tempMeasurement >= m_Max[dim][end])
    {
      // one of measurement is above the maximum
      // its ok if we extend the bins to infinity.. not ok if we don't
      // Need to include the last endpoint in the last bin.
      if (!m_ClipBinsAtEnds || Math::AlmostEquals(tempMeasurement, m_Max[dim][end]))
      {
        index[dim] = (IndexValueType)m_Size[dim] - 1;
        continue;
      }

      // set an illegal value and return 0
      index[dim] = (IndexValueType)m_Size[dim];
      return false;
    }

    // Binary search for the bin where this measurement could be
    IndexValueType  mid = (end + 1) / 2;
    MeasurementType median = m_Min[dim][mid];

    while (true)
    {
      if (tempMeasurement < median)
      {
        end = mid - 1;
      }
      else if (tempMeasurement > median)
      {
        // test whether it is inside the current bin by comparing to the max of
        // this bin.
        if (tempMeasurement < m_Max[dim][mid] && tempMeasurement >= m_Min[dim][mid])
        {
          index[dim] = mid;
          break;
        }
        // otherwise, continue binary search
        begin = mid + 1;
      }
      else
      {
        index[dim] = mid;
        break;
      }
      mid = begin + (end - begin) / 2;
      median = m_Min[dim][mid];
    } // end of while
  } // end of for()
  return true;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline const typename Histogram<TMeasurement, TFrequencyContainer>::IndexType &
Histogram<TMeasurement, TFrequencyContainer>::GetIndex(InstanceIdentifier id) const
{
  InstanceIdentifier id2 = id;
  const unsigned int measurementVectorSize = this->GetMeasurementVectorSize();
  for (int i = measurementVectorSize - 1; i > 0; i--)
  {
    m_TempIndex[i] = static_cast<IndexValueType>(id2 / this->m_OffsetTable[i]);
    id2 -= (m_TempIndex[i] * this->m_OffsetTable[i]);
  }
  m_TempIndex[0] = static_cast<IndexValueType>(id2);

  return m_TempIndex;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline bool
Histogram<TMeasurement, TFrequencyContainer>::IsIndexOutOfBounds(const IndexType & index) const
{
  const unsigned int measurementVectorSize = this->GetMeasurementVectorSize();
  for (unsigned int dim = 0; dim < measurementVectorSize; ++dim)
  {
    if (index[dim] < 0 || index[dim] >= static_cast<IndexValueType>(m_Size[dim]))
    {
      return true;
    }
  }
  return false;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline auto
Histogram<TMeasurement, TFrequencyContainer>::GetInstanceIdentifier(const IndexType & index) const -> InstanceIdentifier
{
  InstanceIdentifier instanceId = 0;

  for (int i = this->GetMeasurementVectorSize() - 1; i > 0; i--)
  {
    instanceId += index[i] * this->m_OffsetTable[i];
  }

  instanceId += index[0];

  return instanceId;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline const typename Histogram<TMeasurement, TFrequencyContainer>::MeasurementType &
Histogram<TMeasurement, TFrequencyContainer>::GetBinMinFromValue(const unsigned int dimension, const float value) const
{
  // If the value is lower than any of min value in the Histogram,
  // it returns the lowest min value
  if (value <= this->m_Min[dimension][0])
  {
    return this->m_Min[dimension][0];
  }

  // If the value is higher than any of min value in the Histogram,
  // it returns the highest min value
  if (value >= m_Min[dimension][m_Size[dimension] - 1])
  {
    return m_Min[dimension][this->m_Size[dimension] - 1];
  }

  unsigned int binMinFromValue = 0;

  for (unsigned int i = 0; i < this->m_Size[dimension]; ++i)
  {
    if ((value >= this->m_Min[dimension][i]) && (value < this->m_Max[dimension][i]))
    {
      binMinFromValue = i;
    }
  }

  return this->m_Min[dimension][binMinFromValue];
}

template <typename TMeasurement, typename TFrequencyContainer>
inline const typename Histogram<TMeasurement, TFrequencyContainer>::MeasurementType &
Histogram<TMeasurement, TFrequencyContainer>::GetBinMaxFromValue(const unsigned int dimension, const float value) const
{
  // If the value is lower than any of max value in the Histogram,
  // it returns the lowest max value
  if (value <= this->m_Max[dimension][0])
  {
    return this->m_Max[dimension][0];
  }

  // If the value is higher than any of max value in the Histogram,
  // it returns the highest max value
  if (value >= m_Max[dimension][m_Size[dimension] - 1])
  {
    return m_Max[dimension][this->m_Size[dimension] - 1];
  }

  unsigned int binMaxFromValue = 0;

  for (unsigned int i = 0; i < this->m_Size[dimension]; ++i)
  {
    if ((value >= this->m_Min[dimension][i]) && (value < this->m_Max[dimension][i]))
    {
      binMaxFromValue = i;
    }
  }

  return this->m_Max[dimension][binMaxFromValue];
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetHistogramMinFromIndex(const IndexType & index) const
  -> const MeasurementVectorType &
{
  const unsigned int measurementVectorSize = this->GetMeasurementVectorSize();
  for (unsigned int i = 0; i < measurementVectorSize; ++i)
  {
    m_TempMeasurementVector[i] = this->GetBinMin(i, index[i]);
  }
  return m_TempMeasurementVector;
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetHistogramMaxFromIndex(const IndexType & index) const
  -> const MeasurementVectorType &
{
  const unsigned int measurementVectorSize = this->GetMeasurementVectorSize();
  for (unsigned int i = 0; i < measurementVectorSize; ++i)
  {
    m_TempMeasurementVector[i] = this->GetBinMax(i, index[i]);
  }
  return m_TempMeasurementVector;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline auto
Histogram<TMeasurement, TFrequencyContainer>::GetMeasurementVector(const IndexType & index) const
  -> const MeasurementVectorType &
{
  const unsigned int measurementVectorSize = this->GetMeasurementVectorSize();
  for (unsigned int i = 0; i < measurementVectorSize; ++i)
  {
    const MeasurementType value = (m_Min[i][index[i]] + m_Max[i][index[i]]);
    m_TempMeasurementVector[i] = static_cast<MeasurementType>(value / 2.0);
  }
  return m_TempMeasurementVector;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline auto
Histogram<TMeasurement, TFrequencyContainer>::GetMeasurementVector(InstanceIdentifier id) const
  -> const MeasurementVectorType &
{
  return this->GetMeasurementVector(this->GetIndex(id));
}

template <typename TMeasurement, typename TFrequencyContainer>
inline void
Histogram<TMeasurement, TFrequencyContainer>::SetFrequency(const AbsoluteFrequencyType value)
{
  typename Self::Iterator       iter = this->Begin();
  const typename Self::Iterator end = this->End();

  while (iter != end)
  {
    iter.SetFrequency(value);
    ++iter;
  }
}

template <typename TMeasurement, typename TFrequencyContainer>
inline bool
Histogram<TMeasurement, TFrequencyContainer>::SetFrequencyOfIndex(const IndexType &           index,
                                                                  const AbsoluteFrequencyType value)
{
  return this->SetFrequency(this->GetInstanceIdentifier(index), value);
}

template <typename TMeasurement, typename TFrequencyContainer>
inline bool
Histogram<TMeasurement, TFrequencyContainer>::SetFrequencyOfMeasurement(const MeasurementVectorType & measurement,
                                                                        const AbsoluteFrequencyType   value)
{
  IndexType index;
  this->GetIndex(measurement, index);
  return this->SetFrequencyOfIndex(index, value);
}

template <typename TMeasurement, typename TFrequencyContainer>
inline bool
Histogram<TMeasurement, TFrequencyContainer>::IncreaseFrequencyOfIndex(const IndexType &           index,
                                                                       const AbsoluteFrequencyType value)
{
  const bool result = this->IncreaseFrequency(this->GetInstanceIdentifier(index), value);

  return result;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline bool
Histogram<TMeasurement, TFrequencyContainer>::IncreaseFrequencyOfMeasurement(const MeasurementVectorType & measurement,
                                                                             const AbsoluteFrequencyType   value)
{
  IndexType index;

  this->GetIndex(measurement, index);
  return this->IncreaseFrequency(this->GetInstanceIdentifier(index), value);
}

template <typename TMeasurement, typename TFrequencyContainer>
inline auto
Histogram<TMeasurement, TFrequencyContainer>::GetFrequency(const IndexType & index) const -> AbsoluteFrequencyType
{
  return (this->GetFrequency(this->GetInstanceIdentifier(index)));
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetMeasurement(InstanceIdentifier n, unsigned int dimension) const
  -> MeasurementType
{
  return static_cast<MeasurementType>((m_Min[dimension][n] + m_Max[dimension][n]) / 2);
}

template <typename TMeasurement, typename TFrequencyContainer>
auto
Histogram<TMeasurement, TFrequencyContainer>::GetFrequency(InstanceIdentifier n, unsigned int dimension) const
  -> AbsoluteFrequencyType
{
  const InstanceIdentifier nextOffset = this->m_OffsetTable[dimension + 1];
  InstanceIdentifier       current = this->m_OffsetTable[dimension] * n;
  const InstanceIdentifier includeLength = this->m_OffsetTable[dimension];
  InstanceIdentifier       include;
  InstanceIdentifier       includeEnd;
  const InstanceIdentifier last = this->m_OffsetTable[this->GetMeasurementVectorSize()];

  AbsoluteFrequencyType frequency = 0;

  while (current < last)
  {
    include = current;
    includeEnd = include + includeLength;
    while (include < includeEnd)
    {
      frequency += GetFrequency(include);
      ++include;
    }
    current += nextOffset;
  }
  return frequency;
}

template <typename TMeasurement, typename TFrequencyContainer>
inline auto
Histogram<TMeasurement, TFrequencyContainer>::GetTotalFrequency() const -> TotalAbsoluteFrequencyType
{
  return m_FrequencyContainer->GetTotalFrequency();
}

template <typename TMeasurement, typename TFrequencyContainer>
double
Histogram<TMeasurement, TFrequencyContainer>::Quantile(unsigned int dimension, double p) const
{
  const unsigned int size = this->GetSize(dimension);

  double cumulated = 0;
  auto   totalFrequency = static_cast<double>(this->GetTotalFrequency());


  double f_n = NAN;
  double p_n_prev = NAN;
  if (p < 0.5)
  {
    InstanceIdentifier n = 0;
    double             p_n = 0.0;
    do
    {
      f_n = this->GetFrequency(n, dimension);
      cumulated += f_n;
      p_n_prev = p_n;
      p_n = cumulated / totalFrequency;
      ++n;
    } while (n < size && p_n < p);

    const double binProportion = f_n / totalFrequency;

    const auto   min = static_cast<double>(this->GetBinMin(dimension, n - 1));
    const auto   max = static_cast<double>(this->GetBinMax(dimension, n - 1));
    const double interval = max - min;
    return min + ((p - p_n_prev) / binProportion) * interval;
  }

  InstanceIdentifier n = size - 1;
  InstanceIdentifier m{};
  double             p_n = 1.0;
  do
  {
    f_n = this->GetFrequency(n, dimension);
    cumulated += f_n;
    p_n_prev = p_n;
    p_n = 1.0 - cumulated / totalFrequency;
    --n;
    ++m;
  } while (m < size && p_n > p);

  const double binProportion = f_n / totalFrequency;
  const auto   min = static_cast<double>(this->GetBinMin(dimension, n + 1));
  const auto   max = static_cast<double>(this->GetBinMax(dimension, n + 1));
  const double interval = max - min;
  return max - ((p_n_prev - p) / binProportion) * interval;
}

template <typename TMeasurement, typename TFrequencyContainer>
double
Histogram<TMeasurement, TFrequencyContainer>::Mean(unsigned int dimension) const
{
  const unsigned int size = this->GetSize(dimension);
  const double       totalFrequency = this->GetTotalFrequency();
  double             sum = 0;
  for (unsigned int i = 0; i < size; ++i)
  {
    const double frequency = this->GetFrequency(i, dimension);
    sum += frequency * this->GetMeasurement(i, dimension);
  }
  return sum / totalFrequency;
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::PrintSelf(std::ostream & os, Indent indent) const
{
  using namespace print_helper;

  Superclass::PrintSelf(os, indent);

  os << indent << "Size: " << static_cast<typename NumericTraits<SizeType>::PrintType>(m_Size) << std::endl;
  os << indent << "OffsetTable: " << std::endl;
  for (const auto & elem : m_OffsetTable)
  {
    os << indent.GetNextIndent() << "[" << &elem - &*(m_OffsetTable.begin()) << "]: " << elem << std::endl;
  }

  itkPrintSelfObjectMacro(FrequencyContainer);

  os << indent << "NumberOfInstances: " << m_NumberOfInstances << std::endl;

  os << indent << "Min: " << std::endl;
  for (const auto & elemVec : m_Min)
  {
    for (const auto & elem : elemVec)
    {
      os << indent.GetNextIndent() << "[" << &elem - &*(elemVec.begin())
         << "]: " << static_cast<typename NumericTraits<MeasurementType>::PrintType>(elem) << std::endl;
    }
  }

  os << indent << "Max: " << std::endl;
  for (const auto & elemVec : m_Max)
  {
    for (const auto & elem : elemVec)
    {
      os << indent.GetNextIndent() << "[" << &elem - &*(elemVec.begin())
         << "]: " << static_cast<typename NumericTraits<MeasurementType>::PrintType>(elem) << std::endl;
    }
  }

  os << indent << "TempMeasurementVector: " << m_TempMeasurementVector << std::endl;
  os << indent << "TempIndex: " << static_cast<typename NumericTraits<IndexType>::PrintType>(m_TempIndex) << std::endl;
  itkPrintSelfBooleanMacro(ClipBinsAtEnds);
}

template <typename TMeasurement, typename TFrequencyContainer>
void
Histogram<TMeasurement, TFrequencyContainer>::Graft(const DataObject * thatObject)
{
  this->Superclass::Graft(thatObject);

  const auto * thatConst = dynamic_cast<const Self *>(thatObject);
  if (thatConst)
  {
    auto * that = const_cast<Self *>(thatConst);
    this->m_Size = that->m_Size;
    this->m_OffsetTable = that->m_OffsetTable;
    this->m_FrequencyContainer = that->m_FrequencyContainer;
    this->m_NumberOfInstances = that->m_NumberOfInstances;
    this->m_Min = that->m_Min;
    this->m_Max = that->m_Max;
    this->m_TempMeasurementVector = that->m_TempMeasurementVector;
    this->m_TempIndex = that->m_TempIndex;
    this->m_ClipBinsAtEnds = that->m_ClipBinsAtEnds;
  }
}

} // namespace itk::Statistics

#endif

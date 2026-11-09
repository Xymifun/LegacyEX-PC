#include "nn/pia/transport/NetworkRttManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#ifdef __ghs__
#include <ppc_ghs.h>
#elif defined(__powerpc__) || defined(__PPC__)
static inline float NegativeMultiplySubtract(float a, float c, float b)
{
    float result;
    __asm__("fnmsubs %0, %1, %2, %3" : "=f"(result) : "f"(a), "f"(c), "f"(b));
    return result;
}
#endif

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/transport/TransportSupport.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

static const unsigned long long cTraceCategoryRtt = 0x8000;
static const unsigned long long cTraceCategoryRttDetail = 0x40000;

namespace
{
enum
{
    ResultAlreadyInitialized = 0x10c02,
    ResultAllocationFailed = 0x10c01
};
}

NetworkRttManager* NetworkRttManager::sInstance = 0;
static unsigned char* sStorage = 0;

static inline float FastSqrt(float value)
{
#ifdef __ghs__
    const float guess = __FRSQRTEF(value);
    return __FNMSUBS(guess * guess, value, 3.0F) * (guess * 0.5F) * value;
#elif defined(__powerpc__) || defined(__PPC__)
    float guess;
    __asm__("frsqrte %0, %1" : "=f"(guess) : "f"(value));
    return NegativeMultiplySubtract(guess * guess, value, 3.0F) * (guess * 0.5F) * value;
#else
    return std::sqrt(value);
#endif
}

template <class T>
NetworkRttManager::DataBuffer<T>::DataBuffer() : mData(0),
                                                 mCritical(-1)
{
}

template <class T>
void NetworkRttManager::DataBuffer<T>::Initialize(unsigned int capacity, pead::Heap* heap)
{
    if (capacity == 0)
    {
        mData = 0;
        return;
    }
    mData = common::NewArray<T>(capacity);
    mValues.allocBuffer(static_cast<int>(capacity), heap, 4);
}

template <class T>
void NetworkRttManager::DataBuffer<T>::Finalize()
{
    if (mData)
    {
        mValues.freeBuffer();
        common::DeleteArray(mData);
        mData = 0;
    }
}

template <class T>
T* NetworkRttManager::DataBuffer<T>::Get(const common::StationAddress& address)
{
    if (mValues.isBufferReady())
    {
        const int index = mValues.binarySearch(&address, &CompareFunc);
        if (index >= 0)
        {
            T* value = mValues.at(index);
            return value;
        }
    }
    return 0;
}

template <class T>
T* NetworkRttManager::DataBuffer<T>::Assign(const common::StationAddress& address, unsigned int now)
{
    if (!mValues.isBufferReady())
        return 0;
    const int index = mValues.binarySearch(&address, &CompareFunc);
    if (index >= 0)
    {
        T* value = mValues.at(index);
        Refresh(*value, now);
        return value;
    }
    T* value;
    if (mValues.size() < mValues.maxSize())
    {
        value = &mData[mValues.size()];
    }
    else
    {
        int oldest = 0;
        unsigned int oldestAge = now - mValues.unsafeAt(0)->time;
        for (int i = 1; i < mValues.size(); ++i)
        {
            const unsigned int age = now - mValues.at(i)->time;
            if (age > oldestAge)
            {
                oldest = i;
                oldestAge = age;
            }
        }
        value = mValues.at(oldest);
        mValues.erase(oldest, 1);
    }
    value->address = address;
    value->time = now;
    Reset(*value);
    for (int i = 0; i < mValues.size(); ++i)
    {
        if (address < mValues.at(i)->address)
        {
            mValues.insert(i, value);
            return value;
        }
    }
    mValues.pushBack(value);
    return value;
}

template class NetworkRttManager::DataBuffer<NetworkRttManager::MeasurementData>;
template class NetworkRttManager::DataBuffer<NetworkRttManager::ResponseData>;

NetworkRttManager::StatUnit::StatUnit()
{
    Clear();
}

void NetworkRttManager::StatUnit::Clear()
{
    latest = 0;
    total = 0;
    squares = 0;
    count = 0;
}

void NetworkRttManager::StatUnit::AddValue(unsigned int value)
{
    const unsigned int square = value * value;
    if (squares + square < squares)
        return;
    latest = value;
    total += value;
    squares += square;
    ++count;
}

void NetworkRttManager::MeasurementData::Clear()
{
    for (unsigned int i = 0; i < 16; ++i)
        values[i].Clear();
    updateTime = 0;
}

unsigned int NetworkRttManager::MeasurementData::UpdateTime(unsigned int now)
{
    const unsigned int current = now >> 8;
    const unsigned int previous = updateTime >> 8;
    if (current != previous)
    {
        if (current - previous >= 16)
        {
            Clear();
            updateTime = now;
            return current;
        }
        const unsigned int slot = current & 15;
        const unsigned int previousSlot = previous & 15;
        if (slot < previousSlot)
        {
            for (unsigned int i = 0; i <= slot; ++i)
                values[i].Clear();
            for (unsigned int i = previousSlot + 1; i < 16; ++i)
                values[i].Clear();
        }
        else
        {
            for (unsigned int i = previousSlot + 1; i <= slot; ++i)
                values[i].Clear();
        }
    }
    updateTime = now;
    return current;
}

void NetworkRttManager::MeasurementData::AddValue(unsigned int now, unsigned int value)
{
    const unsigned int slot = UpdateTime(now) & 15;
    values[slot].AddValue(value);
}

unsigned int NetworkRttManager::MeasurementData::GetAverage() const
{
    unsigned int total = 0;
    unsigned int count = 0;
    const StatUnit* unit = values;
    for (unsigned int i = 0; i < 16; ++i, ++unit)
    {
        total += unit->total;
        count += unit->count;
    }
    unsigned int average = 0;
    if (count != 0)
        average = total / count;
    return average;
}

int NetworkRttManager::MeasurementData::GetStdDev() const
{
    unsigned int total = 0;
    unsigned long long squares = 0;
    unsigned int count = 0;
    const StatUnit* unit = values;
    for (unsigned int i = 0; i < 16; ++i, ++unit)
    {
        squares += unit->squares;
        total += unit->total;
        count += unit->count;
    }
    if (count != 0)
    {
        const float n = static_cast<float>(count);
        const float mean = static_cast<float>(total) / n;
        const float variance = static_cast<float>(squares) / n - mean * mean;
        if (variance > 0.0f)
            return static_cast<unsigned int>(FastSqrt(variance));
    }
    return 0;
}

int NetworkRttManager::MeasurementData::GetCount() const
{
    int count = 0;
    for (unsigned int i = 0; i < 16; ++i)
        count += values[i].count;
    return count;
}

unsigned int NetworkRttManager::MeasurementData::GetLatest() const
{
    const unsigned int current = (updateTime >> 8) & 15;
    for (int i = current; i >= 0; --i)
    {
        if (values[i].count != 0)
            return values[i].latest;
    }
    for (int i = 15; i > static_cast<int>(current); --i)
    {
        if (values[i].count != 0)
            return values[i].latest;
    }
    return 0;
}

void NetworkRttManager::MeasurementData::GetValues(StatValues* output) const
{
    unsigned int total = 0;
    unsigned long long squares = 0;
    unsigned int count = 0;
    const StatUnit* unit = values;
    for (unsigned int i = 0; i < 16; ++i, ++unit)
    {
        total += unit->total;
        squares += unit->squares;
        count += unit->count;
    }
    if (count == 0)
    {
        output->standardDeviation = 0;
        output->latest = 0;
        output->average = 0;
    }
    else
    {
        const float n = static_cast<float>(count);
        const float mean = static_cast<float>(total) / n;
        output->average = total / count;
        const float variance = static_cast<float>(squares) / n - mean * mean;
        if (variance <= 0.0f)
            output->standardDeviation = 0;
        else
            output->standardDeviation = static_cast<unsigned int>(FastSqrt(variance));
        output->count = count;
        output->latest = GetLatest();
    }
}

NetworkRttManager::NetworkRttManager()
{
}

NetworkRttManager::~NetworkRttManager()
{
    Finalize();
}

Result NetworkRttManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (sInstance)
        return 0x10c0f;
    pead::Heap* heap = common::HeapManager::GetHeap();
    sStorage = static_cast<unsigned char*>(pead::allocateFromSeadHeap(0x80, heap, 8));
    if (!sStorage)
        return ResultAllocationFailed;
    sInstance = new (sStorage) NetworkRttManager();
    return sInstance ? 0 : ResultAllocationFailed;
}

void NetworkRttManager::Finalize()
{
    mResponses.Finalize();
    mMeasurements.Finalize();
}

void NetworkRttManager::DestroyInstance()
{
    if (sInstance)
    {
        sInstance->~NetworkRttManager();
        sInstance = 0;
    }
    if (sStorage)
    {
        common::DeleteArray(sStorage);
        sStorage = 0;
    }
}

Result NetworkRttManager::InitializeImpl(unsigned int measurementCapacity, unsigned int responseCapacity)
{
    if (mMeasurements.IsInitialized() || mResponses.IsInitialized())
        return ResultAlreadyInitialized;
    pead::Heap* heap = common::HeapManager::GetHeap();
    mMeasurements.Initialize(measurementCapacity, heap);
    mResponses.Initialize(responseCapacity, heap);
    mStartTime.SetNow();
    return 0;
}

void NetworkRttManager::WriteToPacket(common::Packet* packet)
{
    const unsigned int now = GetElapsedMilliseconds();
    packet->m_packetData.m_header.m_rttResponseFlag = mMeasurements.IsInitialized() ? static_cast<unsigned short>(now)
                                                                                    : 0;
    const common::StationAddress& destination = packet->m_field5f4;
    unsigned short measurement = 0;
    if (destination.IsValid())
    {
        common::CriticalSection& lock = mResponses.GetCriticalSection();
        lock.Lock();
        ResponseData* response = mResponses.Get(destination);
        if (response)
        {
            const unsigned int delay = now - response->time;
            if (delay < 10000)
            {
                measurement = static_cast<unsigned short>(response->sequence + delay);
                if (measurement == 0)
                    measurement = 0xffff;
            }
        }
        lock.Unlock();
    }
    packet->m_packetData.m_header.m_rttMeasurement = measurement;
}

void NetworkRttManager::ReadFromPacket(const common::Packet* packet)
{
    const common::StationAddress& source = packet->m_field608;
    if (!source.IsValid())
    {
        source.Trace(cTraceCategoryRttDetail);
        return;
    }
    const unsigned int now = GetElapsedMilliseconds();
    if (packet->m_packetData.m_header.m_rttResponseFlag != 0)
    {
        common::CriticalSection& lock = mResponses.GetCriticalSection();
        lock.Lock();
        ResponseData* response = mResponses.Assign(source, now);
        if (response)
        {
            response->sequence = packet->m_packetData.m_header.m_rttResponseFlag;
        }
        else
        {
            source.Trace(cTraceCategoryRtt);
        }
        lock.Unlock();
    }
    const unsigned short measurement = packet->m_packetData.m_header.m_rttMeasurement;
    if (measurement != 0)
    {
        const unsigned int rtt = (now - measurement) & 0xffff;
        common::CriticalSection& lock = mMeasurements.GetCriticalSection();
        lock.Lock();
        MeasurementData* value = mMeasurements.Assign(source, now);
        source.Trace(cTraceCategoryRttDetail);
        if (value)
            value->AddValue(now, rtt);
        lock.Unlock();
    }
}

unsigned int NetworkRttManager::GetAverage(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    unsigned int result = value->GetAverage();
    mMeasurements.Unlock();
    return result;
}
int NetworkRttManager::GetStdDev(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    int result = value->GetStdDev();
    mMeasurements.Unlock();
    return result;
}
int NetworkRttManager::GetCount(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    int result = value->GetCount();
    mMeasurements.Unlock();
    return result;
}
unsigned int NetworkRttManager::GetLatest(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    unsigned int result = value->GetLatest();
    mMeasurements.Unlock();
    return result;
}
void NetworkRttManager::GetValues(StatValues* values, const common::StationAddress& address)
{
    MeasurementData* value;
    common::CriticalSection& lock = mMeasurements.GetCriticalSection();
    lock.Lock();
    value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        values->average = 0;
        values->standardDeviation = 0;
        values->count = 0;
        values->latest = 0;
        lock.Unlock();
        return;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    value->GetValues(values);
    lock.Unlock();
}

int NetworkRttManager::CompareFunc(const ResponseData* value, const common::StationAddress* address)
{
    return common::StationAddress::Compare(value->address, *address);
}
int NetworkRttManager::CompareFunc(const MeasurementData* value, const common::StationAddress* address)
{
    return common::StationAddress::Compare(value->address, *address);
}

}
}
}

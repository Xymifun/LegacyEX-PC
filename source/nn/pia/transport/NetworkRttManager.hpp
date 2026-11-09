#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_NETWORKRTTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_NETWORKRTTMANAGER_HPP

#include <cmath>
#include <new>

#include "nn/pia.h"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "platform/wiiu/pead.hpp"

namespace pead
{
class Heap;
}

namespace nn
{
namespace pia
{
namespace transport
{

class NetworkRttManager
{
public:
    struct StatValues
    {
        unsigned int average;
        unsigned int standardDeviation;
        unsigned int count;
        unsigned int latest;
    };

    struct StatUnit
    {
        unsigned int latest;
        unsigned int total;
        unsigned int squares;
        unsigned int count;

        StatUnit();
        void Clear();
        void AddValue(unsigned int value);
    };

    struct MeasurementData
    {
        common::StationAddress address;
        unsigned int time;
        StatUnit values[16];
        unsigned int updateTime;

        MeasurementData() : time(0) { Clear(); }
        void Clear();
        unsigned int UpdateTime(unsigned int now);
        void AddValue(unsigned int now, unsigned int value);
        unsigned int GetAverage() const;
        int GetStdDev() const;
        int GetCount() const;
        unsigned int GetLatest() const;
        void GetValues(StatValues* output) const;
    };

    struct ResponseData
    {
        common::StationAddress address;
        unsigned int time;
        unsigned short sequence;

        ResponseData() : time(0), sequence(0) {}
    };

    template <class T>
    class DataBuffer
    {
    public:
        DataBuffer();
        void Initialize(unsigned int capacity, pead::Heap* heap);
        void Finalize();
        T* Get(const common::StationAddress& address);
        T* Assign(const common::StationAddress& address, unsigned int now);
        void Lock() { mCritical.Lock(); }
        void Unlock() { mCritical.Unlock(); }
        common::CriticalSection& GetCriticalSection() { return mCritical; }
        bool IsInitialized() const { return mData != 0; }

    private:
        static void Refresh(MeasurementData& value, unsigned int now) { value.UpdateTime(now); }
        static void Refresh(ResponseData& value, unsigned int now) { value.time = now; }
        static void Reset(MeasurementData& value) { value.Clear(); }
        static void Reset(ResponseData& value) { value.sequence = 0; }
        pead::PtrArray<T> mValues;
        T* mData;
        common::CriticalSection mCritical;
    };

    NetworkRttManager();
    ~NetworkRttManager();
    static Result CreateInstance();
    void Finalize();
    static void DestroyInstance();
    Result InitializeImpl(unsigned int measurementCapacity, unsigned int responseCapacity);
    void WriteToPacket(common::Packet* packet);
    void ReadFromPacket(const common::Packet* packet);
    unsigned int GetAverage(const common::StationAddress& address);
    int GetStdDev(const common::StationAddress& address);
    int GetCount(const common::StationAddress& address);
    unsigned int GetLatest(const common::StationAddress& address);
    void GetValues(StatValues* values, const common::StationAddress& address);
    static int CompareFunc(const ResponseData* value, const common::StationAddress* address);
    static int CompareFunc(const MeasurementData* value, const common::StationAddress* address);
    void Trace(unsigned long long value) const;

    static NetworkRttManager* sInstance;

private:
    friend class Transport;
    unsigned int GetElapsedMilliseconds() const
    {
        common::Time now;
        now.SetNow();
        return static_cast<unsigned int>(static_cast<long long>(now.m_tick - mStartTime.m_tick)
                                         / common::TimeSpan::GetTicksPerMSec().m_tick);
    }
    DataBuffer<ResponseData> mResponses;
    DataBuffer<MeasurementData> mMeasurements;
    common::Time mStartTime;
};

}
}
}

#endif

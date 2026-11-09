#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROFILINGUNIT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROFILINGUNIT_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qChain.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class CriticalSection;

class ProfilingUnit : public RootObject
{
public:
    typedef qChain<ProfilingUnit*, DefaultChainPolicy<ProfilingUnit*> > List;

    ProfilingUnit(const wchar_t* pName, unsigned int uiTraceMask, const wchar_t* pDescription);
    virtual ~ProfilingUnit();

    void Reset(Time time);
    void Log();

    void IncTime(unsigned long long ullTime);
    void IncTime(unsigned long long ullTime, const wchar_t* pActivity);

    uint32_t GetNbOccurences() const;
    const wchar_t* GetName() const;
    uint32_t GetID() const;

    unsigned long long GetTotalTime() const;
    unsigned long long GetMinTime() const;
    unsigned long long GetMaxTime() const;
    unsigned long long GetLastTime() const;
    unsigned long long GetAvgTime() const;
    unsigned long long GetVarTime() const;

    void EnableLogging(bool bEnable);
    void SetLoggingFrequency(unsigned int uiFrequency);
    void SetName(const wchar_t* pName);

    static void Enable();
    static void Disable();
    static bool IsEnabled();
    static void ResetAll();
    static qList<unsigned int> GetAllIDs();
    static ProfilingUnit* FindByID(unsigned int uiID);
    static qList<ProfilingUnit*> FindByName(const String& rName);
    static void Report(const qList<String>& rNames);
    static List& GetList();
    static CriticalSection& CS();

private:
    friend class DefaultChainPolicy<ProfilingUnit*>;

    qChainLink<ProfilingUnit*> m_link;
    unsigned long long m_totalTime;
    unsigned long long m_totalSquaredTime;
    unsigned long long m_minTime;
    unsigned long long m_maxTime;
    unsigned long long m_lastTime;
    Time m_lastResetTime;
    uint32_t m_nbOccurences;
    unsigned int m_traceMask;
    String m_name;
    bool m_bLoggingEnabled;
    uint16_t m_reserved;
    uint32_t m_id;
    uint32_t m_nbLogs;
    unsigned int m_loggingFrequency;
    String m_lastActivity;
    String m_maxTimeActivity;
};

}
}

#endif

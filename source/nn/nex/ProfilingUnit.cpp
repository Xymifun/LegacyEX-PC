#include "nn/nex/ProfilingUnit.hpp"

#include <algorithm>
#include <cwchar>
#include <functional>

#include "nn/nex/CriticalSection.hpp"

namespace nn
{
namespace nex
{

ProfilingUnit::List g_profilingUnits;
CriticalSection g_profilingUnitCriticalSection(0);
unsigned int g_nextProfilingUnitID = 0;

ProfilingUnit::ProfilingUnit(const wchar_t* pName, unsigned int uiTraceMask, const wchar_t*)
    : m_link(),
      m_lastResetTime(),
      m_name(pName),
      m_lastActivity(),
      m_maxTimeActivity()
{
    Reset(Time::GetTime());

    m_reserved = 0;
    m_traceMask = uiTraceMask;
    m_bLoggingEnabled = false;
    m_nbLogs = 0;
    m_loggingFrequency = 0;

    CS().Enter();

    GetList().push_back(this);
    m_id = g_nextProfilingUnitID++;

    CS().Leave();
}

ProfilingUnit::~ProfilingUnit()
{
    CS().Enter();

    GetList().remove(this);

    if (m_bLoggingEnabled)
        Log();

    CS().Leave();
}

void ProfilingUnit::Reset(Time time)
{
    m_nbOccurences = 0;
    m_totalTime = 0;
    m_totalSquaredTime = 0;
    m_minTime = 0xffffffffffffffffull;
    m_maxTime = 0;
    m_lastTime = 0;
    m_lastResetTime = time;
    m_nbLogs = 0;
}

void ProfilingUnit::Log()
{
    if (GetNbOccurences() == 0)
        return;

    Time::GetTime();
}

uint32_t ProfilingUnit::GetNbOccurences() const
{
    return m_nbOccurences;
}

const wchar_t* ProfilingUnit::GetName() const
{
    return m_name;
}

uint32_t ProfilingUnit::GetID() const
{
    return m_id;
}

void ProfilingUnit::ResetAll()
{
    Time time(Time::GetTime());

    CS().Enter();

    std::for_each(GetList().begin(), GetList().end(), std::bind2nd(std::mem_fun(&ProfilingUnit::Reset), time));

    CS().Leave();
}

qList<unsigned int> ProfilingUnit::GetAllIDs()
{
    CS().Enter();

    qList<unsigned int> ids;

    List::iterator unit = GetList().begin();
    List::iterator last = GetList().end();

    for (; unit != last; ++unit)
        ids.push_back((*unit)->GetID());

    CS().Leave();
    return ids;
}

ProfilingUnit* ProfilingUnit::FindByID(unsigned int uiID)
{
    CS().Enter();

    for (List::iterator unit = GetList().begin(); unit != GetList().end(); ++unit)
    {
        if ((*unit)->GetID() == uiID)
        {
            ProfilingUnit* pFound = *unit;
            CS().Leave();
            return pFound;
        }
    }

    CS().Leave();
    return 0;
}

qList<ProfilingUnit*> ProfilingUnit::FindByName(const String& rName)
{
    CS().Enter();

    qList<ProfilingUnit*> units;

    List::iterator unit = GetList().begin();
    List::iterator last = GetList().end();

    for (; unit != last; ++unit)
    {
        if (String::IsEqual(rName, (*unit)->GetName()))
            units.push_back(*unit);
    }

    CS().Leave();
    return units;
}

void ProfilingUnit::Report(const qList<String>& rNames)
{
    CS().Enter();

    for (List::iterator unit = GetList().begin(); unit != GetList().end(); ++unit)
    {
        for (qList<String>::const_iterator name = rNames.begin(); name != rNames.end(); ++name)
        {
            if (std::wcscmp(*name, (*unit)->GetName()) == 0)
                (*unit)->Log();
        }
    }

    CS().Leave();
}

ProfilingUnit::List& ProfilingUnit::GetList()
{
    return g_profilingUnits;
}

CriticalSection& ProfilingUnit::CS()
{
    return g_profilingUnitCriticalSection;
}

}
}

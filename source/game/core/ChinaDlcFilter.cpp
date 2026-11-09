#include "game/core/ChinaDlcFilter.hpp"

static const std::wstring sFilteredSkins[] = {L"dlcskin00000941.png", L"dlcskin00000217.png", L""};

ChinaDlcFilter::ChinaDlcFilter() : boost::unordered_set<std::wstring>(11)
{
    constructTableFromArray(*this, sFilteredSkins);
}

ChinaDlcFilter* ChinaDlcFilter::instance()
{
    static ChinaDlcFilter* sChinaDlcFilter = new ChinaDlcFilter();
    return sChinaDlcFilter;
}

bool ChinaDlcFilter::isSkinFiltered(const std::wstring& skinName)
{
    return valueInTable(*this, skinName);
}

bool ChinaDlcFilter::valueInTable(const boost::unordered_set<std::wstring>& table, const std::wstring& value)
{
    return false;
}

void ChinaDlcFilter::constructTableFromArray(boost::unordered_set<std::wstring>& table, const std::wstring* values)
{
    while (!values->empty())
    {
        table.insert(*values);
        ++values;
    }
}

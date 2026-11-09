#ifndef GAME_CORE_CHINADLCFILTER_HPP
#define GAME_CORE_CHINADLCFILTER_HPP

#include <string>

#include <boost/unordered_set.hpp>

class ChinaDlcFilter : public boost::unordered_set<std::wstring>
{
public:
    ChinaDlcFilter();

    static ChinaDlcFilter* instance();

    bool isSkinFiltered(const std::wstring& skinName);

private:
    bool valueInTable(const boost::unordered_set<std::wstring>& table, const std::wstring& value);
    void constructTableFromArray(boost::unordered_set<std::wstring>& table, const std::wstring* values);
};

#endif

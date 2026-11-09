#ifndef GAME_CORE_WSTRINGLOOKUP_HPP
#define GAME_CORE_WSTRINGLOOKUP_HPP

#include <string>

class WstringLookup
{
public:
    WstringLookup();
    std::wstring lookup(unsigned int index);
    unsigned int lookup(std::wstring value);
    void getTable(std::wstring** table, unsigned int* count);
};

#endif

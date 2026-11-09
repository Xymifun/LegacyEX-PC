#ifndef GAME_CORE_DLCGAMERULES_HPP
#define GAME_CORE_DLCGAMERULES_HPP

#include <string>

#include "game/core/DLCFile.hpp"

class DLCGameRules : public DLCFile
{
public:
    DLCGameRules(DLCManager::EDLCType type, const std::wstring& path) : DLCFile(type, path) {}
    virtual ~DLCGameRules() {}
};

#endif

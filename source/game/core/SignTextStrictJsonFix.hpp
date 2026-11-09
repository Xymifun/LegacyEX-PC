#ifndef GAME_CORE_SIGNTEXTSTRICTJSONFIX_HPP
#define GAME_CORE_SIGNTEXTSTRICTJSONFIX_HPP

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class SignTextStrictJsonFix : public DataFix
{
public:
    virtual ~SignTextStrictJsonFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);

private:
    void updateLine(CompoundTag* tag, const std::wstring& key);
};

#endif

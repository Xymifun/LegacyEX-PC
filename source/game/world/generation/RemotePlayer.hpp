#ifndef GAME_WORLD_GENERATION_REMOTEPLAYER_HPP
#define GAME_WORLD_GENERATION_REMOTEPLAYER_HPP

#include <string>

#include "game/world/generation/AbstractClientPlayer.hpp"

class BlockPos;
class DamageSource;
class GameType;
class Level;

class RemotePlayer : public AbstractClientPlayer
{
public:
    RemotePlayer(Level* level, const std::wstring& name);
    virtual bool hasPermission(EGameCommand command);
    virtual BlockPos getCommandSenderBlockPosition();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void tick();
    virtual void aiStep();
    virtual void animateRespawn();
    virtual void SetGameType(const GameType* gameType);
    virtual const GameType* GetGameType();
    virtual unsigned long long GetType();

private:
    unsigned char mField0x878;
    int mField0x87c;
    float mField0x880;
    const GameType* mGameType;
};

#endif

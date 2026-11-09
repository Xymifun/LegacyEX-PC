#ifndef GAME_WORLD_GENERATION_ABSTRACTCLIENTPLAYER_HPP
#define GAME_WORLD_GENERATION_ABSTRACTCLIENTPLAYER_HPP

#include <string>

#include "game/world/generation/Player.hpp"

class GameType;
class Level;

class AbstractClientPlayer : public Player
{
public:
    AbstractClientPlayer(Level* level, const std::wstring& name);
    virtual ~AbstractClientPlayer() {}

    virtual void tick();
    virtual bool isPickable();
    virtual bool isSpectator();
    virtual bool isCreative();
    float getFieldOfViewModifier();
    virtual const GameType* GetGameType() = 0;
    virtual void SetGameType(const GameType* gameType) = 0;

    float mElytraRotX;
    float mElytraRotY;
    float mElytraRotZ;
};

#endif

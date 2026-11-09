#ifndef GAME_SERVER_SERVERITEMCOOLDOWNS_HPP
#define GAME_SERVER_SERVERITEMCOOLDOWNS_HPP

#include "game/item/ItemCooldowns.hpp"

class Item;
class ServerPlayer;

class ServerItemCooldowns : public ItemCooldowns
{
public:
    ServerItemCooldowns(ServerPlayer* player);

    virtual void onCooldownStarted(Item* item, int duration);
    virtual void onCooldownEnded(Item* item);

private:
    ServerPlayer* mPlayer;
};

#endif

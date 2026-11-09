#ifndef GAME_CORE_MINECARTRIDEABLE_HPP
#define GAME_CORE_MINECARTRIDEABLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/Minecart.hpp"

class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class Player;

class MinecartRideable : public Minecart
{
public:
    MinecartRideable(Level* level);
    MinecartRideable(Level* level, double x, double y, double z);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void activateMinecart(int x, int y, int z, bool powered);
    virtual const Minecart::Type* getType();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned long long GetType();
};

#endif

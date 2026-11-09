#include "game/core/MinecartRideable.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

MinecartRideable::MinecartRideable(Level* level) : Minecart(level)
{
    fjDerivedCtorCalls();
    mCategories |= EntityCategory::eEntityCategory_Rideable;
}

MinecartRideable::MinecartRideable(Level* level, double x, double y, double z) : Minecart(level, x, y, z)
{
    fjDerivedCtorCalls();
    mCategories |= EntityCategory::eEntityCategory_Rideable;
}

Entity* MinecartRideable::create(Level* level)
{
    return new MinecartRideable(level);
}

void MinecartRideable::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
}

bool MinecartRideable::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (player->isSneaking())
    {
        return false;
    }

    if (!isVehicle() && !mLevel->mIsClientSide)
    {
        player->startRiding(shared_from_this());
    }

    return true;
}

void MinecartRideable::activateMinecart(int x, int y, int z, bool powered)
{
    if (powered)
    {
        if (isVehicle())
        {
            ejectPassengers();
        }

        if (getHurtTime() == 0)
        {
            setHurtDir(-getHurtDir());
            setHurtTime(10);
            setDamage(50.0f);
            markHurt();
        }
    }
}

const Minecart::Type* MinecartRideable::getType()
{
    return Minecart::Type::RIDEABLE;
}

unsigned int MinecartRideable::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_pPlayer->isSneaking() && getPassengers()->empty())
    {
        return IDS_TOOLTIPS_RIDE;
    }

    return IDS_NONE;
}

unsigned long long MinecartRideable::GetType()
{
    return EntityType::MINECART_RIDEABLE;
}

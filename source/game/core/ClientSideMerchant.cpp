#include "game/core/ClientSideMerchant.hpp"

#include <stdint.h>

#include "game/item/MerchantContainer.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Villager.hpp"

ClientSideMerchant::ClientSideMerchant(boost::shared_ptr<Player> player, const std::wstring& name)
{
    mTradingPlayer = player;
    mContainer = NULL;
    mOffers = NULL;
    mName = name;
    mVillager = boost::weak_ptr<Villager>();
}

ClientSideMerchant::~ClientSideMerchant()
{
    delete mContainer;
    delete mOffers;
}

void ClientSideMerchant::createContainer()
{
    mContainer = new MerchantContainer(mTradingPlayer, shared_from_this());
}

boost::shared_ptr<Player> ClientSideMerchant::getTradingPlayer()
{
    return mTradingPlayer;
}

void ClientSideMerchant::setTradingPlayer(const boost::shared_ptr<Player>& player)
{
}

MerchantRecipeList* ClientSideMerchant::getOffers(const boost::shared_ptr<Player>& player)
{
    return mOffers;
}

void ClientSideMerchant::overrideOffers(MerchantRecipeList* offers)
{
    delete mOffers;
    mOffers = offers;
}

void ClientSideMerchant::notifyTrade(MerchantRecipe* recipe)
{
    recipe->increaseUses();
}

void ClientSideMerchant::notifyTradeUpdated(const not_null_ptr<ItemInstance>& item)
{
}

std::wstring ClientSideMerchant::getDisplayName()
{
    return mName;
}

void ClientSideMerchant::setVillager(boost::shared_ptr<Villager> villager)
{
    mVillager = villager;
}

bool ClientSideMerchant::isAvailableToTrade()
{
    boost::shared_ptr<Villager> villager = mVillager.lock();
    if (villager && villager->mRemoved)
        return false;
    return true;
}

Level* ClientSideMerchant::getLevel()
{
    return mTradingPlayer->mLevel;
}

BlockPos ClientSideMerchant::getPosition()
{
    return BlockPos(mTradingPlayer);
}

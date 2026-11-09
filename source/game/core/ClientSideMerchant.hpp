#ifndef GAME_CORE_CLIENTSIDEMERCHANT_HPP
#define GAME_CORE_CLIENTSIDEMERCHANT_HPP

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/Merchant.hpp"
#include "game/world/block/BlockPos.hpp"

class MerchantContainer;
class MerchantRecipe;
class MerchantRecipeList;
class ItemInstance;
class Player;
class Villager;

class ClientSideMerchant : public Merchant, public boost::enable_shared_from_this<ClientSideMerchant>
{
public:
    ClientSideMerchant(boost::shared_ptr<Player> player, const std::wstring& name);
    virtual ~ClientSideMerchant();

    void createContainer();
    boost::shared_ptr<Player> getTradingPlayer();
    virtual void setTradingPlayer(const boost::shared_ptr<Player>& player);
    virtual MerchantRecipeList* getOffers(const boost::shared_ptr<Player>& player);
    virtual void overrideOffers(MerchantRecipeList* offers);
    virtual void notifyTrade(MerchantRecipe* recipe);
    virtual void notifyTradeUpdated(const not_null_ptr<ItemInstance>& item);
    virtual std::wstring getDisplayName();
    void setVillager(boost::shared_ptr<Villager> villager);
    virtual bool isAvailableToTrade();
    virtual Level* getLevel();
    virtual BlockPos getPosition();
    MerchantContainer* getContainer();

private:
    MerchantContainer* mContainer;
    boost::shared_ptr<Player> mTradingPlayer;
    MerchantRecipeList* mOffers;
    std::wstring mName;
    boost::weak_ptr<Villager> mVillager;
};

#endif

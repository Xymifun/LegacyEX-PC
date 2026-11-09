#ifndef GAME_CORE_MERCHANT_HPP
#define GAME_CORE_MERCHANT_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class BlockPos;
class Level;
class MerchantRecipe;
class MerchantRecipeList;
class Player;

class Merchant
{
public:
    virtual ~Merchant() {}
    virtual void setTradingPlayer(const boost::shared_ptr<Player>& player) = 0;
    virtual boost::shared_ptr<Player> getTradingPlayer() = 0;
    virtual MerchantRecipeList* getOffers(const boost::shared_ptr<Player>& player) = 0;
    virtual void overrideOffers(MerchantRecipeList* offers) = 0;
    virtual void notifyTrade(MerchantRecipe* recipe) = 0;
    virtual void notifyTradeUpdated(const not_null_ptr<ItemInstance>& item) = 0;
    virtual std::wstring getDisplayName() = 0;
    virtual bool isAvailableToTrade() = 0;
    virtual Level* getLevel() = 0;
    virtual BlockPos getPosition() = 0;
};

#endif

#ifndef GAME_WORLD_BLOCK_LOCALBLOCKINTERACTION_HPP
#define GAME_WORLD_BLOCK_LOCALBLOCKINTERACTION_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/InteractionObject.hpp"

class Inventory;
class Player;

class LocalBlockInteraction : public InteractionObject
{
public:
    LocalBlockInteraction(int typeId, const std::wstring& name);

    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    virtual int getTypeId(bool large);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();

private:
    int mTypeId;
    std::wstring mName;
};

#endif

#ifndef GAME_CLIENT_RENDER_PLAYERENDERCHESTCONTAINER_HPP
#define GAME_CLIENT_RENDER_PLAYERENDERCHESTCONTAINER_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionObject.hpp"
#include "game/item/SimpleContainer.hpp"

class CompoundTag;
class EnderChestBlockEntity;
class ListTag;
class Player;

class PlayerEnderChestContainer : public SimpleContainer
{
public:
    PlayerEnderChestContainer();
    virtual ~PlayerEnderChestContainer();

    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);

    virtual unsigned int getContainerType();
    void setActiveChest(boost::shared_ptr<EnderChestBlockEntity> chest);
    void setItemsByTag(ListTag* tag);
    ListTag* createTag();
    InteractionObject::NameData GetNetworkNameData();

private:
    boost::shared_ptr<EnderChestBlockEntity> m_activeChest;
};

#endif

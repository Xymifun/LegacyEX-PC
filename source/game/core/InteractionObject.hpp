#ifndef GAME_CORE_INTERACTIONOBJECT_HPP
#define GAME_CORE_INTERACTIONOBJECT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Nameable.hpp"

class AbstractContainerMenu;
class Inventory;
class Player;

class InteractionObject : public virtual Nameable
{
public:
    class NameData
    {
    public:
        NameData(std::wstring name, std::wstring ownerUuid, bool createdOnHost, bool createdByRestrictedPlayer)
        {
            this->name = name;
            this->ownerUuid = ownerUuid;
            this->createdOnHost = createdOnHost;
            this->createdByRestrictedPlayer = createdByRestrictedPlayer;
        }

        std::wstring name;
        std::wstring ownerUuid;
        bool createdOnHost;
        bool createdByRestrictedPlayer;
    };

    InteractionObject() {}

    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory,
                                              boost::shared_ptr<Player> player) = 0;
    virtual int getTypeId(bool large) = 0;
    virtual NameData GetNetworkNameData() { return NameData(getDisplayName(), L"", true, false); }
};

#endif

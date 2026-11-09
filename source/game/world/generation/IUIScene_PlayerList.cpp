#include "game/world/generation/IUIScene_PlayerList.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/KickPlayerPacket.hpp"
#include "game/ui/UIControl_ButtonList.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

IUIScene_PlayerList::PlayerChange::PlayerChange(unsigned char smallIdValue, bool addedValue)
    : smallId(smallIdValue),
      added(addedValue)
{
}

IUIScene_PlayerList::IUIScene_PlayerList()
{
    InitializeCriticalSection(&mCriticalSection);
}

IUIScene_PlayerList::~IUIScene_PlayerList()
{
    std::vector<PlayerChange*>::iterator it;
    for (it = mPlayerChanges.begin(); it != mPlayerChanges.end(); ++it)
        delete *it;
    mPlayerChanges.clear();
    DeleteCriticalSection(&mCriticalSection);
}

int IUIScene_PlayerList::KickPlayerReturned(void* context, int playerIndex, C4JStorage::EMessageResult result)
{
    unsigned char smallId = *static_cast<unsigned char*>(context);
    free(context);
    if (result == C4JStorage::EMessageResult_Accept)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        if (player->mPacketListener != NULL)
            player->mPacketListener->send(boost::shared_ptr<KickPlayerPacket>(new KickPlayerPacket(smallId)));
    }
    return 0;
}

void IUIScene_PlayerList::OnPlayerChanged(void* context, INetworkPlayer* player, bool added)
{
    IUIScene_PlayerList* list = static_cast<IUIScene_PlayerList*>(context);
    EnterCriticalSection(&list->mCriticalSection);
    list->mPlayerChanges.push_back(new PlayerChange(player->GetSmallId(), added));
    LeaveCriticalSection(&list->mCriticalSection);
}

void IUIScene_PlayerList::tick()
{
    EnterCriticalSection(&mCriticalSection);
    unsigned int removed = 0;
    for (int index = 0; index < mPlayerChanges.size(); ++index)
    {
        PlayerChange* change = mPlayerChanges[index];
        UIControl_ButtonList* list = GetPlayerList();
        bool wasRemoved = RemovePlayer(change->smallId);
        if (!change->added)
            AddPlayer(change->smallId);
        if (!change->added)
        {
            if (!wasRemoved)
                list->addItem(std::wstring(L""), list->getItemCount());
        }
        else if (wasRemoved)
        {
            ++removed;
            if (removed < static_cast<unsigned int>(list->getItemCount()))
                list->removeItem(list->getItemCount() - (index + 1), false);
        }
        delete change;
    }
    LeaveCriticalSection(&mCriticalSection);

    for (unsigned int item = 0; item < static_cast<unsigned int>(GetPlayerCount()); ++item)
    {
        if (item >= static_cast<unsigned int>(GetPlayerList()->getItemCount()))
            break;
        PlayerInfo* currentInfo = GetPlayer(item);
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(currentInfo->smallId);
        if (player != NULL)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(player, info);
            UpdatePlayer(item, currentInfo, info);
            delete info;
        }
    }

    EnterCriticalSection(&mCriticalSection);
    mPlayerChanges.clear();
    LeaveCriticalSection(&mCriticalSection);
}

bool IUIScene_PlayerList::hasAPlayerChanged()
{
    return !mPlayerChanges.empty();
}

void IUIScene_PlayerList::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    info->smallId = player->GetSmallId();

    std::wstring displayName(L"");
    displayName = player->GetDisplayName();

    char voiceStatus = 0;
    if (player->HasVoice())
    {
        if (player->IsMutedByLocalUser(GetPad()))
        {
            voiceStatus = 3;
        }
        else
        {
            voiceStatus = 1;
            if (player->IsTalking())
            {
                voiceStatus = 2;
            }
        }
    }

    info->voiceStatus = voiceStatus;
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
    info->name = displayName;
}

PlayerInfo* IUIScene_PlayerList::CreatePlayerInfo()
{
    return new PlayerInfo();
}

int IUIScene_PlayerList::FindPlayer(unsigned char smallId)
{
    int result = -1;
    for (int index = 0; index < GetPlayerCount(); ++index)
    {
        if (GetPlayer(index)->smallId == smallId)
        {
            if (GetPlayerList()->getCurrentSelection() == GetPlayerList()->getItemCount() - 1)
            {
                GetPlayerList()->setCurrentSelection(std::max(0, GetPlayerList()->getItemCount() - 2), false, false);
            }
            result = index;
            break;
        }
    }
    return result;
}

void IUIScene_PlayerList::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    int comparison = newInfo->name.compare(0, newInfo->name.size(), oldInfo->name.c_str(), oldInfo->name.size());

    if (comparison != 0 || hasAPlayerChanged())
    {
        oldInfo->name.assign(newInfo->name);
        GetPlayerList()->setButtonLabel(index, newInfo->name);
    }

    if (newInfo->smallId != oldInfo->smallId || hasAPlayerChanged())
    {
        oldInfo->smallId = newInfo->smallId;
        GetPlayerList()->setButtonLabel(index, newInfo->name);
    }
}

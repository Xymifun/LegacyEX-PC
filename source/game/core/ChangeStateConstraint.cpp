#include "game/core/ChangeStateConstraint.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ChangeStateConstraint::ChangeStateConstraint(Tutorial* tutorial, eTutorial_State targetState, eTutorial_State* states,
                                             unsigned int numStates, double x0, double y0, double z0, double x1,
                                             double y1, double z1, bool insideArea, bool changeGameType,
                                             const GameType* gameType)
    : TutorialConstraint(-1)
{
    area = AABB::newPermanent(x0, y0, z0, x1, y1, z1);
    this->insideArea = insideArea;
    this->changeGameType = changeGameType;
    this->gameType = gameType;
    previousGameType = NULL;
    this->tutorial = tutorial;
    this->targetState = targetState;
    this->numStates = numStates;
    stateChanged = false;
    previousState = eTutorial_State_Invalid;
    gameTypeChanged = false;
    this->states = new eTutorial_State[numStates];

    for (unsigned int i = 0; i < this->numStates; i++)
    {
        this->states[i] = states[i];
    }
}

ChangeStateConstraint::~ChangeStateConstraint()
{
    delete area;

    if (numStates != 0)
    {
        delete[] states;
    }
}

int ChangeStateConstraint::getType()
{
    return 4;
}

void ChangeStateConstraint::tick(int playerId)
{
    bool inMatchingState = false;
    Minecraft* minecraft = Minecraft::GetInstance();

    for (unsigned int i = 0; i < numStates; i++)
    {
        if (states[i] == tutorial->currentState)
        {
            inMatchingState = true;
            break;
        }
    }

    if (!stateChanged && inMatchingState
        && area->contains(minecraft->mLocalPlayers[playerId]->getPos(1.0f)) == insideArea)
    {
        stateChanged = true;
        previousState = tutorial->currentState;
        tutorial->changeTutorialState(targetState, NULL);

        if (changeGameType && minecraft->mGameModes[playerId] != NULL)
        {
            previousGameType = minecraft->mLocalPlayers[playerId]->mAbilities.mInstabuild ? GameType::CREATIVE
                                                                                          : GameType::SURVIVAL;

            unsigned int privileges = minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(
                Player::ePlayerGamePrivilege_All);
            Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CreativeMode,
                                           gameType == GameType::CREATIVE);

            if (minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All)
                != privileges)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[playerId];

                if (player && player->mPacketListener != NULL && player->mPacketListener->getNetworkPlayer() != NULL)
                {
                    player->mPacketListener->send(boost::shared_ptr<PlayerInfoPacket>(
                        new PlayerInfoPacket(player->mPacketListener->getNetworkPlayer()->GetSmallId(), -1, privileges,
                                             player->GetGameType()->getId())));
                }
            }
        }
    }
    else if (stateChanged && area->contains(minecraft->mLocalPlayers[playerId]->getPos(1.0f)) != insideArea)
    {
        stateChanged = false;
        tutorial->changeTutorialState(previousState, NULL);

        if (changeGameType)
        {
            unsigned int privileges = minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(
                Player::ePlayerGamePrivilege_All);
            Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CreativeMode,
                                           previousGameType == GameType::CREATIVE);

            if (minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All)
                != privileges)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[playerId];

                if (player && player->mPacketListener != NULL && player->mPacketListener->getNetworkPlayer() != NULL)
                {
                    player->mPacketListener->send(boost::shared_ptr<PlayerInfoPacket>(
                        new PlayerInfoPacket(player->mPacketListener->getNetworkPlayer()->GetSmallId(), -1, privileges,
                                             player->GetGameType()->getId())));
                }
            }
        }
    }
}

bool ChangeStateConstraint::isTargetState(int state)
{
    return state == targetState;
}

bool ChangeStateConstraint::withinTargetStateArea(int playerId)
{
    return area->contains(Minecraft::GetInstance()->mLocalPlayers[playerId]->getPos(1.0f));
}

bool ChangeStateConstraint::withinTargetStateArea(Vec3* pos)
{
    return area->contains(pos);
}

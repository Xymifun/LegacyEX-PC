#include "game/command/CheckpointCommand.hpp"

#include <vector>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerStats.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGlide.hpp"
#endif

EGameCommand CheckpointCommand::getId()
{
    return eGameCommand_Checkpoint;
}

int CheckpointCommand::getPermissionLevel()
{
    return 0;
}

void CheckpointCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    MinecraftServer* server = gMinecraftServer;

    StatsUID uid(stream.readPlayerUID());

    PlayerList* playerList = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;
    boost::shared_ptr<ServerPlayer> player = playerList->getPlayerFromStatsUID(uid);

    int checkpoint = stream.readInt();
    byteStream.reset();

    std::vector<CheckpointRuleDefinition*>* checkpoints =
        gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpoints();

    MasterGameMode* gameMode = server->mMasterGameMode;
    unsigned int round = gameMode->mGameStats.mCurrentRound;

    if (player && player->isAlive() && !gameMode->mGameStats.GetPlayerStats(uid)->getReachedEnd(round))
    {
        ServerPlayerGameMode* playerGameMode = static_cast<ServerPlayerGameMode*>(player->GetGameMode());

        if (checkpoint == 0)
        {
            gameMode->mGameStats.RemovePlayerStats(player);
            gameMode->mGameStats.AddPlayerStats(player);
            playerGameMode->RecordLastCheckpoint(0, 0, true, true, true);
            gameMode->mGameStats.OnRoundStart();

            player->mGameMode->SetDeaths(0);
            player->setHealth(3.0f);
            player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(uid, 0);

            gameMode->ResetRoundTime();
        }
#ifdef MINECRAFT_PC
        else if (checkpoint == kPcCheckpointStartKeepTime && !checkpoints->empty())
        {
            playerGameMode->RecordLastCheckpoint(0, (*checkpoints)[0]->getSequenceID(), true, true, true);
        }
#endif
        else if (checkpoint > 0)
        {
            if ((unsigned int)checkpoint < checkpoints->size())
            {
                playerGameMode->RecordLastCheckpoint(checkpoint, (*checkpoints)[checkpoint]->getSequenceID(), true,
                                                     true, true);
            }
        }

        for (int lastCheckpoint = playerGameMode->getLastCheckpoint(); lastCheckpoint >= 0; --lastCheckpoint)
        {
            if ((*checkpoints)[lastCheckpoint]->hasSpawnPoint())
            {
                playerGameMode->ResetTempPoints();
                playerGameMode->RecordLastCheckpoint(lastCheckpoint, (*checkpoints)[lastCheckpoint]->getSequenceID(),
                                                     true, true, true);

                UpdatePlayerRuleDefinition* spawnPoint = (*checkpoints)[lastCheckpoint]->GetSpawnPoint();
                int spawnX = spawnPoint->getX();
                int spawnY = spawnPoint->getY();
                int spawnZ = spawnPoint->getZ();

                PlayerList* players = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;
                std::vector<boost::shared_ptr<ServerPlayer> >* allPlayers = players->getPlayers();

                std::vector<boost::shared_ptr<ServerPlayer> >::iterator end = allPlayers->end();
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = allPlayers->begin(); it != end; ++it)
                {
                    if (*it && (*it)->GetCamera() == player)
                    {
                        (*it)->mLoadedChunks.clear();
                        (*it)->mLastSentX = (double)spawnX;
                        (*it)->mLastSentZ = (double)spawnZ;

                        (*it)->mConnection->teleport((double)spawnX, (double)spawnY, (double)spawnZ,
                                                     spawnPoint->getYRot(), spawnPoint->getXRot(), true, true);

                        if (checkpoint == 0)
                        {
                            (*it)->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_UpdateStartTime,
                                                                            GameModePacket::UpdateStartTimeData(0)));
                        }
                    }
                }

                player->mXOld = (double)spawnX;
                player->mYOld = (double)spawnY;
                player->mZOld = (double)spawnZ;

                if (CommonMasterGameMode::GetMiniGame()->GetId() == 3)
                {
                    player->startFallFlying();
                }
                break;
            }
        }

        gameMode->ResetPlayer(player);

        int deaths = gMinecraftServer->mMasterGameMode->mGameStats.GetDeathCount(uid);

        if (gameMode->mGameStats.GetPlayerStats(uid) != NULL)
        {
            player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(uid, deaths);
        }

        player->ResetThermalVariables();
        gameMode->ResetLastKnownPosition(player->mPlayerIndex);
    }
}

boost::shared_ptr<GameCommandPacket> CheckpointCommand::preparePacket(const StatsUID& uid, int checkpoint)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writePlayerUID(uid.GetID());
    stream.writeInt(checkpoint);

    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_Checkpoint, byteStream.toByteArray()));
}

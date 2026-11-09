#include "game/core/GhostGameMode.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Minecraft.hpp"
#include "game/world/generation/GhostPlayer.hpp"

GhostGameMode::GhostGameMode(GhostPlayer* player) : GameMode(), mGhostPlayer(player)
{
}

GhostGameMode::~GhostGameMode()
{
}

bool GhostGameMode::startDestroyBlock(const BlockPos&, const Direction*)
{
    return false;
}

void GhostGameMode::tick()
{
}

InteractionResult::EInteractionResult GhostGameMode::useItem(boost::shared_ptr<Player>, Level*,
                                                             InteractionHand::EInteractionHand, bool)
{
    return InteractionResult::eInteractionResult_Fail;
}

void GhostGameMode::SetGameType(const GameType*)
{
}

const GameType* GhostGameMode::GetGameType()
{
    return NULL;
}

const MiniGameDef* GhostGameMode::GetMiniGame()
{
    return Minecraft::GetInstance()->GetMiniGame();
}

boost::shared_ptr<Player> GhostGameMode::GetPlayer()
{
    return boost::dynamic_pointer_cast<Player>(mGhostPlayer->shared_from_this());
}

Level* GhostGameMode::GetLevel()
{
    return NULL;
}

bool GhostGameMode::RecordLastCheckpoint(int checkpoint, int, bool, bool, bool)
{
    mGhostPlayer->setLastCheckpointId(checkpoint);
    return true;
}

bool GhostGameMode::RecordLapCompleted()
{
    mGhostPlayer->setLapsCompleted(mGhostPlayer->getLapsCompleted() + 1);
    return true;
}

bool GhostGameMode::RecordProgress(double, double, double progress, double)
{
    mGhostPlayer->setProgress(progress);
    return true;
}

#include <cfloat>

#include "game/world/generation/GhostPlayer.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/GhostGameMode.hpp"

GhostPlayer::GhostPlayer(Level* level, const std::wstring& name)
    : RemotePlayer(level, name),
      mLastCheckpointId(0),
      mLapsCompleted(0),
      mProgress(0.0)
{
    mX = DBL_MAX;
    mY = DBL_MAX;
    mZ = DBL_MAX;
    mXOld = DBL_MAX;
    mYOld = DBL_MAX;
    mZOld = DBL_MAX;
    mGameMode = new GhostGameMode(this);
}

GhostPlayer::~GhostPlayer()
{
    delete mGameMode;
}

int GhostPlayer::getLastCheckpointId()
{
    return mLastCheckpointId;
}

void GhostPlayer::setLastCheckpointId(int checkpointId)
{
    mLastCheckpointId = checkpointId;
}

int GhostPlayer::getLapsCompleted()
{
    return mLapsCompleted;
}

void GhostPlayer::setLapsCompleted(int lapsCompleted)
{
    mLapsCompleted = lapsCompleted;
}

double GhostPlayer::getProgress()
{
    return mProgress;
}

void GhostPlayer::setProgress(double progress)
{
    mProgress = progress;
}

GameMode* GhostPlayer::GetGameMode()
{
    return mGameMode;
}

unsigned long long GhostPlayer::GetType()
{
    return EntityType::GHOST_PLAYER;
}

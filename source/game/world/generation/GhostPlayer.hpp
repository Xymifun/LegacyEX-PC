#ifndef GAME_WORLD_GENERATION_GHOSTPLAYER_HPP
#define GAME_WORLD_GENERATION_GHOSTPLAYER_HPP

#include <string>

#include "game/world/generation/RemotePlayer.hpp"
#include "game/core/GhostGameMode.hpp"

class GhostPlayer : public RemotePlayer
{
public:
    GhostPlayer(Level* level, const std::wstring& name);
    virtual ~GhostPlayer();
    int getLastCheckpointId();
    void setLastCheckpointId(int checkpointId);
    int getLapsCompleted();
    void setLapsCompleted(int lapsCompleted);
    double getProgress();
    void setProgress(double progress);
    GameMode* GetGameMode();
    virtual unsigned long long GetType();

private:
    int mLastCheckpointId;
    int mLapsCompleted;
    double mProgress;
    GhostGameMode* mGameMode;
};

#endif

#ifndef GAME_CORE_GAMEMODETRANSITIONPARAMS_HPP
#define GAME_CORE_GAMEMODETRANSITIONPARAMS_HPP

class GameModeTransitionParams
{
public:
    GameModeTransitionParams(int texturePackId, bool returningToLobby, bool waitForRoundStart, int minChunkX,
                             int minChunkZ, int maxChunkX, int maxChunkZ, int themeId);

    int mTexturePackId;
    bool mReturningToLobby;
    bool mWaitForRoundStart;
    int mMinChunkX;
    int mMinChunkZ;
    int mMaxChunkX;
    int mMaxChunkZ;
    int mThemeId;
};

#endif

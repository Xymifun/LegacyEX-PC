#include "game/core/GameModeTransitionParams.hpp"

GameModeTransitionParams::GameModeTransitionParams(int texturePackId, bool returningToLobby, bool waitForRoundStart,
                                                   int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ,
                                                   int themeId)
    : mTexturePackId(texturePackId),
      mReturningToLobby(returningToLobby),
      mWaitForRoundStart(waitForRoundStart),
      mMinChunkX(minChunkX),
      mMinChunkZ(minChunkZ),
      mMaxChunkX(maxChunkX),
      mMaxChunkZ(maxChunkZ),
      mThemeId(themeId)
{
}

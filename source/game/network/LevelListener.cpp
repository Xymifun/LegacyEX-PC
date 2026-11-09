#include "game/network/LevelListener.hpp"

#include "game/client/audio/SoundEvent.hpp"

void LevelListener::playSoundToAllPlayers(const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y,
                                          double z, float volume, float pitch, float range)
{
}

void LevelListener::invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ)
{
}

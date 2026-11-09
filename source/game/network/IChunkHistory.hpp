#ifndef GAME_NETWORK_ICHUNKHISTORY_HPP
#define GAME_NETWORK_ICHUNKHISTORY_HPP

#include <vector>

class IChunkHistory
{
public:
    virtual std::vector<short>* GetChangesForChunk(int x, int z) const = 0;
    virtual bool ShouldLoadOnClient(int x, int z) const = 0;
};

#endif

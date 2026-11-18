#ifndef PLATFORM_PC_PCTERRAINARENA_HPP
#define PLATFORM_PC_PCTERRAINARENA_HPP

#include <map>
#include <vector>

struct PcTerrainSlot
{
    int arena = -1;
    int firstVertex = 0;
    int blocks = 0;
};

class PcTerrainArenas
{
public:
    static const int kBlockVertices = 128;
    static const int kVertexBytes = 16;
    static const int kArenaVertices = 1 << 21;
    static const int kArenaBlocks = kArenaVertices / kBlockVertices;
    static const int kMaxArenas = 64;

    static PcTerrainArenas& Get();

    bool Allocate(int vertexCount, unsigned int quadIndexBuffer, PcTerrainSlot& slot);
    void Upload(const PcTerrainSlot& slot, const void* vertices, int vertexCount, const float* chunkOffset,
                const unsigned short* quadElements, int quadElementCount);
    void Release(const PcTerrainSlot& slot);
    void FlushReleases();

    int ArenaCount() const { return static_cast<int>(mArenas.size()); }
    unsigned int VertexArray(int arena) const { return mArenas[arena].vertexArray; }
    unsigned int VertexBuffer(int arena) const { return mArenas[arena].vertexBuffer; }
    unsigned int QuadBuffer(int arena) const { return mArenas[arena].quadBuffer; }
    unsigned int OffsetTexture(int arena) const { return mArenas[arena].offsetTexture; }

private:
    struct Arena
    {
        unsigned int vertexArray = 0;
        unsigned int vertexBuffer = 0;
        unsigned int quadBuffer = 0;
        unsigned int offsetBuffer = 0;
        unsigned int offsetTexture = 0;
        std::map<int, int> freeRuns;
    };

    bool CreateArena(unsigned int quadIndexBuffer);
    static void Free(Arena& arena, int firstBlock, int blocks);

    std::vector<Arena> mArenas;
    std::vector<PcTerrainSlot> mPendingReleases;
    std::vector<float> mOffsetScratch;
    std::vector<unsigned short> mQuadScratch;
};

#endif

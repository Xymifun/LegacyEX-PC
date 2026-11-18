#include "platform/pc/PcTerrainArena.hpp"

#include <cstdio>

#include "platform/pc/PcGl.hpp"

PcTerrainArenas& PcTerrainArenas::Get()
{
    static PcTerrainArenas arenas;
    return arenas;
}

bool PcTerrainArenas::CreateArena(unsigned int quadIndexBuffer)
{
    if (static_cast<int>(mArenas.size()) >= kMaxArenas)
        return false;
    Arena arena;
    PcGl::GenBuffers(1, &arena.vertexBuffer);
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, arena.vertexBuffer);
    PcGl::BufferData(GL_COPY_WRITE_BUFFER, static_cast<GLsizeiptr>(kArenaVertices) * kVertexBytes, nullptr,
                     GL_STATIC_DRAW);
    PcGl::GenBuffers(1, &arena.quadBuffer);
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, arena.quadBuffer);
    PcGl::BufferData(GL_COPY_WRITE_BUFFER, static_cast<GLsizeiptr>(kArenaVertices / 4) * sizeof(unsigned short),
                     nullptr, GL_STATIC_DRAW);
    PcGl::GenBuffers(1, &arena.offsetBuffer);
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, arena.offsetBuffer);
    PcGl::BufferData(GL_COPY_WRITE_BUFFER, static_cast<GLsizeiptr>(kArenaBlocks) * 4 * sizeof(float), nullptr,
                     GL_STATIC_DRAW);
    PcGl::GenTextures(1, &arena.offsetTexture);
    PcGl::BindTexture(GL_TEXTURE_BUFFER, arena.offsetTexture);
    PcGl::TexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, arena.offsetBuffer);
    PcGl::BindTexture(GL_TEXTURE_BUFFER, 0);

    PcGl::GenVertexArrays(1, &arena.vertexArray);
    PcGl::BindVertexArray(arena.vertexArray);
    PcGl::BindBuffer(GL_ARRAY_BUFFER, arena.vertexBuffer);
    PcGl::EnableVertexAttribArray(0);
    PcGl::VertexAttribIPointer(0, 4, GL_SHORT, kVertexBytes, nullptr);
    PcGl::EnableVertexAttribArray(1);
    PcGl::VertexAttribIPointer(1, 4, GL_SHORT, kVertexBytes, reinterpret_cast<const void*>(8));
    PcGl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
    PcGl::BindVertexArray(0);
    PcGl::BindBuffer(GL_ARRAY_BUFFER, 0);

    arena.freeRuns[0] = kArenaBlocks;
    mArenas.push_back(arena);
    std::printf("[pc] gl: terrain arena %d made (%d MB of vertices)\n", static_cast<int>(mArenas.size()),
                kArenaVertices * kVertexBytes / (1024 * 1024));
    std::fflush(stdout);
    return true;
}

bool PcTerrainArenas::Allocate(int vertexCount, unsigned int quadIndexBuffer, PcTerrainSlot& slot)
{
    const int blocks = (vertexCount + kBlockVertices - 1) / kBlockVertices;
    if (blocks <= 0 || blocks > kArenaBlocks)
        return false;
    for (int pass = 0; pass < 2; ++pass)
    {
        for (size_t index = 0; index < mArenas.size(); ++index)
        {
            std::map<int, int>& runs = mArenas[index].freeRuns;
            for (std::map<int, int>::iterator run = runs.begin(); run != runs.end(); ++run)
            {
                if (run->second < blocks)
                    continue;
                const int first = run->first;
                const int left = run->second - blocks;
                runs.erase(run);
                if (left > 0)
                    runs[first + blocks] = left;
                slot.arena = static_cast<int>(index);
                slot.firstVertex = first * kBlockVertices;
                slot.blocks = blocks;
                return true;
            }
        }
        if (pass == 0 && !CreateArena(quadIndexBuffer))
            return false;
    }
    return false;
}

void PcTerrainArenas::Upload(const PcTerrainSlot& slot, const void* vertices, int vertexCount,
                             const float* chunkOffset, const unsigned short* quadElements, int quadElementCount)
{
    const Arena& arena = mArenas[slot.arena];
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, arena.vertexBuffer);
    PcGl::BufferSubData(GL_COPY_WRITE_BUFFER, static_cast<GLintptr>(slot.firstVertex) * kVertexBytes,
                        static_cast<GLsizeiptr>(vertexCount) * kVertexBytes, vertices);

    mOffsetScratch.resize(static_cast<size_t>(slot.blocks) * 4);
    for (int block = 0; block < slot.blocks; ++block)
    {
        for (int component = 0; component < 4; ++component)
            mOffsetScratch[static_cast<size_t>(block) * 4 + component] = chunkOffset[component];
    }
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, arena.offsetBuffer);
    PcGl::BufferSubData(GL_COPY_WRITE_BUFFER,
                        static_cast<GLintptr>(slot.firstVertex / kBlockVertices) * 4 * sizeof(float),
                        static_cast<GLsizeiptr>(mOffsetScratch.size() * sizeof(float)), &mOffsetScratch[0]);

    const int quads = slot.blocks * kBlockVertices / 4;
    mQuadScratch.assign(static_cast<size_t>(quads), 0);
    for (int quad = 0; quad < quadElementCount && quad < quads; ++quad)
        mQuadScratch[static_cast<size_t>(quad)] = quadElements[quad];
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, arena.quadBuffer);
    PcGl::BufferSubData(GL_COPY_WRITE_BUFFER, static_cast<GLintptr>(slot.firstVertex / 4) * sizeof(unsigned short),
                        static_cast<GLsizeiptr>(quads) * sizeof(unsigned short), &mQuadScratch[0]);
    PcGl::BindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

void PcTerrainArenas::Release(const PcTerrainSlot& slot)
{
    if (slot.arena >= 0)
        mPendingReleases.push_back(slot);
}

void PcTerrainArenas::FlushReleases()
{
    for (const PcTerrainSlot& slot : mPendingReleases)
    {
        if (slot.arena >= 0 && slot.arena < static_cast<int>(mArenas.size()))
            Free(mArenas[slot.arena], slot.firstVertex / kBlockVertices, slot.blocks);
    }
    mPendingReleases.clear();
}

void PcTerrainArenas::Free(Arena& arena, int firstBlock, int blocks)
{
    std::map<int, int>& runs = arena.freeRuns;
    std::map<int, int>::iterator next = runs.lower_bound(firstBlock);
    if (next != runs.begin())
    {
        std::map<int, int>::iterator previous = next;
        --previous;
        if (previous->first + previous->second == firstBlock)
        {
            firstBlock = previous->first;
            blocks += previous->second;
            runs.erase(previous);
        }
    }
    if (next != runs.end() && firstBlock + blocks == next->first)
    {
        blocks += next->second;
        runs.erase(next);
    }
    runs[firstBlock] = blocks;
}

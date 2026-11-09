#ifndef GAME_WORLD_CHUNK_CHUNKINFOMAPMANAGER_HPP
#define GAME_WORLD_CHUNK_CHUNKINFOMAPMANAGER_HPP

class BufferBuilder;

class ChunkInfoMapManager
{
public:
    static void Render(float width, float height);

    static void RenderQuad(float x, float y, float width, float height, unsigned int colour,
                           BufferBuilder* bufferBuilder);

private:
    static void renderStatusMap(BufferBuilder* bufferBuilder, float width, float height);
    static void renderHeatMap(BufferBuilder* bufferBuilder, float width, float height);
};

#endif

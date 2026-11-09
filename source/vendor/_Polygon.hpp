#ifndef SOURCE_VENDOR_POLYGON_HPP
#define SOURCE_VENDOR_POLYGON_HPP

#include "game/core/Vertex.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BufferBuilder;

class _Polygon
{
public:
    _Polygon(arrayWithLength<Vertex*> vertices);
    _Polygon(arrayWithLength<Vertex*> vertices, int u0, int v0, int u1, int v1, float width, float height);
    _Polygon(arrayWithLength<Vertex*> vertices, float u0, float v0, float u1, float v1);
    ~_Polygon();

    void _init(arrayWithLength<Vertex*> vertices);
    void mirror();
    void render(BufferBuilder* buffer, float scale);
    void renderSkinned(BufferBuilder* buffer, float scale, int boneIndex);
    void flipNormal();

private:
    arrayWithLength<Vertex*> mVertices;
    int mVertexCount;
    bool mFlipNormal;
};

#endif

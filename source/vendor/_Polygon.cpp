#include "vendor/_Polygon.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/util/Vec3.hpp"

_Polygon::_Polygon(arrayWithLength<Vertex*> vertices)
{
    _init(vertices);
}

_Polygon::_Polygon(arrayWithLength<Vertex*> vertices, int u0, int v0, int u1, int v1, float width, float height)
{
    _init(vertices);

    float uInset = u1 > u0 ? 0.01f / width : -0.01f / width;
    float vInset = v1 > v0 ? 0.01f / height : -0.01f / height;

    vertices[0] = vertices[0]->remap((float)u1 / width - uInset, (float)v0 / height + vInset);
    vertices[1] = vertices[1]->remap((float)u0 / width + uInset, (float)v0 / height + vInset);
    vertices[2] = vertices[2]->remap((float)u0 / width + uInset, (float)v1 / height - vInset);
    vertices[3] = vertices[3]->remap((float)u1 / width - uInset, (float)v1 / height - vInset);
}

_Polygon::_Polygon(arrayWithLength<Vertex*> vertices, float u0, float v0, float u1, float v1)
{
    _init(vertices);

    vertices[0] = vertices[0]->remap(u1, v0);
    vertices[1] = vertices[1]->remap(u0, v0);
    vertices[2] = vertices[2]->remap(u0, v1);
    vertices[3] = vertices[3]->remap(u1, v1);
}

_Polygon::~_Polygon()
{
    delete[] mVertices.data;
}

void _Polygon::_init(arrayWithLength<Vertex*> vertices)
{
    mFlipNormal = false;
    mVertices = vertices;
    mVertexCount = vertices.length;
}

void _Polygon::mirror()
{
    arrayWithLength<Vertex*> mirrored(mVertices.length, true);
    for (unsigned int i = 0; i < mVertices.length; ++i)
    {
        mirrored[i] = mVertices[mVertices.length - i - 1];
    }
    delete[] mVertices.data;
    mVertices = mirrored;
}

void _Polygon::render(BufferBuilder* buffer, float scale)
{
    Vec3* edge0 = mVertices[1]->position->vectorTo(mVertices[0]->position);
    Vec3* edge1 = mVertices[1]->position->vectorTo(mVertices[2]->position);
    Vec3* normalVector = edge1->cross(edge0)->normalize();
    float nx = (float)normalVector->x;
    float ny = (float)normalVector->y;
    float nz = (float)normalVector->z;
    if (mFlipNormal)
    {
        nx = -nx;
        ny = -ny;
        nz = -nz;
    }
    buffer->normal(nx, ny, nz);

    for (int i = 0; i < 4; ++i)
    {
        Vertex* vertex = mVertices[i];
        buffer->vertexUV((float)(vertex->position->x * scale), (float)(vertex->position->y * scale),
                         (float)(vertex->position->z * scale), vertex->textureU, vertex->textureV);
    }
}

void _Polygon::renderSkinned(BufferBuilder* buffer, float scale, int boneIndex)
{
    Vec3* edge0 = mVertices[1]->position->vectorTo(mVertices[0]->position);
    Vec3* edge1 = mVertices[1]->position->vectorTo(mVertices[2]->position);
    Vec3* normalVector = edge1->cross(edge0)->normalize();
    float nx = (float)normalVector->x;
    float ny = (float)normalVector->y;
    float nz = (float)normalVector->z;
    if (mFlipNormal)
    {
        nx = -nx;
        ny = -ny;
        nz = -nz;
    }
    buffer->normal(nx, ny, nz);

    for (int i = 0; i < 4; ++i)
    {
        Vertex* vertex = mVertices[i];
        buffer->vertexUVSkinned((float)(vertex->position->x * scale), (float)(vertex->position->y * scale),
                                (float)(vertex->position->z * scale), vertex->textureU, vertex->textureV, boneIndex);
    }
}

void _Polygon::flipNormal()
{
    mFlipNormal = !mFlipNormal;
}

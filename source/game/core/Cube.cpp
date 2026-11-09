#include "game/core/Cube.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/Mth.hpp"
#include "vendor/_Polygon.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/util/Vec3.hpp"

const int Cube::FACE_RIGHT = 1;
const int Cube::FACE_LEFT = 2;
const int Cube::FACE_TOP = 4;
const int Cube::FACE_BOTTOM = 8;
const int Cube::FACE_FRONT = 16;
const int Cube::FACE_BACK = 32;
const int Cube::FACE_ALL = 63;

Cube::Cube(ModelPart* parent, int texU, int texV, float x, float y, float z, int width, int height, int depth,
           float inflate, bool mirror, int faceMask, bool flip, float xRot, float yRot, float zRot)
    : mX0(x),
      mY0(y),
      mZ0(z),
      mX1(x + (float)width),
      mY1(y + (float)height),
      mZ1(z + (float)depth),
      mTexU(texU),
      mTexV(texV),
      mId()
{
    _init(parent, texU, texV, x, y, z, width, height, depth, inflate, mirror, faceMask, flip);
    rotate(xRot, yRot, zRot);
}

arrayWithLength<Vertex*> Cube::VertexArray4(Vertex* a, Vertex* b, Vertex* c, Vertex* d)
{
    Vertex** vertices = new Vertex*[4];
    vertices[0] = a;
    vertices[2] = c;
    vertices[3] = d;
    vertices[1] = b;
    return arrayWithLength<Vertex*>(vertices, 4);
}

void Cube::_init(ModelPart* parent, int texU, int texV, float x, float y, float z, int width, int height, int depth,
                 float inflate, bool mirror, int faceMask, bool flip)
{
    mVertices = arrayWithLength<Vertex*>(8, true);
    mPolygons = arrayWithLength<_Polygon*>(6, true);

    float x1 = x + width;
    float y1 = y + height;
    float z1 = z + depth;

    x -= inflate;
    y -= inflate;
    z -= inflate;
    x1 += inflate;
    y1 += inflate;
    z1 += inflate;

    const float epsilon = 0.0009765625f;
    if (x == x1)
        x1 += epsilon;
    if (y == y1)
        y1 += epsilon;
    if (z == z1)
        z1 += epsilon;

    if (mirror)
    {
        float t = x1;
        x1 = x;
        x = t;
    }

    Vertex* u0 = new Vertex(x, y, z, 0.0f, 0.0f);
    Vertex* u1 = new Vertex(x1, y, z, 0.0f, 8.0f);
    Vertex* u2 = new Vertex(x1, y1, z, 8.0f, 8.0f);
    Vertex* u3 = new Vertex(x, y1, z, 8.0f, 0.0f);
    Vertex* l0 = new Vertex(x, y, z1, 0.0f, 0.0f);
    Vertex* l1 = new Vertex(x1, y, z1, 0.0f, 8.0f);
    Vertex* l2 = new Vertex(x1, y1, z1, 8.0f, 8.0f);
    Vertex* l3 = new Vertex(x, y1, z1, 8.0f, 0.0f);

    mVertices[0] = u0;
    mVertices[1] = u1;
    mVertices[2] = u2;
    mVertices[3] = u3;
    mVertices[4] = l0;
    mVertices[5] = l1;
    mVertices[6] = l2;
    mVertices[7] = l3;

    unsigned int polygonCount = 0;
    if (faceMask & 1)
        mPolygons[polygonCount++] = new _Polygon(VertexArray4(l1, u1, u2, l2), texU + depth + width, texV + depth,
                                                 texU + depth + width + depth, texV + depth + height, parent->mTexWidth,
                                                 parent->mTexHeight);
    if (faceMask & 2)
        mPolygons[polygonCount++] = new _Polygon(VertexArray4(u0, l0, l3, u3), texU, texV + depth, texU + depth,
                                                 texV + depth + height, parent->mTexWidth, parent->mTexHeight);
    if (faceMask & 4)
        mPolygons[polygonCount++] = new _Polygon(VertexArray4(l1, l0, u0, u1), texU + depth, texV, texU + depth + width,
                                                 texV + depth, parent->mTexWidth, parent->mTexHeight);
    if (flip)
    {
        if (faceMask & 8)
            mPolygons[polygonCount++] = new _Polygon(VertexArray4(u2, u3, l3, l2), texU + depth + width, texV,
                                                     texU + depth + width + width, texV + depth, parent->mTexWidth,
                                                     parent->mTexHeight);
    }
    else
    {
        if (faceMask & 8)
            mPolygons[polygonCount++] = new _Polygon(VertexArray4(u2, u3, l3, l2), texU + depth + width, texV + depth,
                                                     texU + depth + width + width, texV, parent->mTexWidth,
                                                     parent->mTexHeight);
    }
    if (faceMask & 16)
        mPolygons[polygonCount++] = new _Polygon(VertexArray4(u1, u0, u3, u2), texU + depth, texV + depth,
                                                 texU + depth + width, texV + depth + height, parent->mTexWidth,
                                                 parent->mTexHeight);
    if (faceMask & 32)
        mPolygons[polygonCount++] = new _Polygon(VertexArray4(l0, l1, l2, l3), texU + depth + width + depth,
                                                 texV + depth, texU + depth + width + depth + width,
                                                 texV + depth + height, parent->mTexWidth, parent->mTexHeight);
    mPolygons.length = polygonCount;

    if (mirror)
    {
        for (unsigned int i = 0; i < mPolygons.length; ++i)
        {
            mPolygons[i]->mirror();
        }
    }
}

void Cube::rotate_axis(float angle, double& x, double& y) const
{
    double oldX = x;
    double oldY = y;
    double c = Mth::cos(angle);
    double s = Mth::sin(angle);
    x = oldX * c - oldY * s;
    y = oldX * s + oldY * c;
}

void Cube::rotate(float x, float y, float z)
{
    if (x > 0.0f || y > 0.0f)
    {
        for (unsigned int i = 0; i < mVertices.length; ++i)
        {
            if (x != 0.0f)
                rotate_axis(x, mVertices.data[i]->position->y, mVertices.data[i]->position->z);
            if (y != 0.0f)
                rotate_axis(y, mVertices.data[i]->position->y, mVertices.data[i]->position->z);
            if (z != 0.0f)
                rotate_axis(z, mVertices.data[i]->position->y, mVertices.data[i]->position->z);
        }
    }
}

void Cube::render(BufferBuilder* buffer, float scale)
{
    buffer->begin();
    for (unsigned int i = 0; i < mPolygons.length; ++i)
    {
        mPolygons.data[i]->render(buffer, scale);
    }
    buffer->end();
}

void Cube::renderSkinned(BufferBuilder* buffer, float scale, int boneIndex)
{
    for (unsigned int i = 0; i < mPolygons.length; ++i)
    {
        mPolygons.data[i]->renderSkinned(buffer, scale, boneIndex);
    }
}

Cube* Cube::setId(const std::wstring& id)
{
    mId.assign(id, 0, 0xffffffffu);
    return this;
}

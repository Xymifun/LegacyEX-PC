#ifndef GAME_CORE_CUBE_HPP
#define GAME_CORE_CUBE_HPP

#include <string>

#include "game/core/Vertex.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BufferBuilder;
class ModelPart;
class _Polygon;

class Cube
{
public:
    static const int FACE_RIGHT;
    static const int FACE_LEFT;
    static const int FACE_TOP;
    static const int FACE_BOTTOM;
    static const int FACE_FRONT;
    static const int FACE_BACK;
    static const int FACE_ALL;

    Cube(ModelPart* parent, int texU, int texV, float x, float y, float z, int width, int height, int depth,
         float inflate, bool mirror, int faceMask, bool flip, float xRot, float yRot, float zRot);

    arrayWithLength<Vertex*> VertexArray4(Vertex* a, Vertex* b, Vertex* c, Vertex* d);
    void _init(ModelPart* parent, int texU, int texV, float x, float y, float z, int width, int height, int depth,
               float inflate, bool mirror, int faceMask, bool flip);
    void rotate_axis(float angle, double& x, double& y) const;
    void rotate(float x, float y, float z);
    void render(BufferBuilder* buffer, float scale);
    void renderSkinned(BufferBuilder* buffer, float scale, int boneIndex);
    Cube* setId(const std::wstring& id);

private:
    arrayWithLength<Vertex*> mVertices;
    arrayWithLength<_Polygon*> mPolygons;

public:
    float mX0;
    float mY0;
    float mZ0;
    float mX1;
    float mY1;
    float mZ1;

private:
    int mTexU;
    int mTexV;
    std::wstring mId;
};

#endif

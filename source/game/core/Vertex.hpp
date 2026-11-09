#ifndef GAME_CORE_VERTEX_HPP
#define GAME_CORE_VERTEX_HPP

class Vec3;

class Vertex
{
public:
    Vertex(float x, float y, float z, float u, float v);
    Vertex(Vertex* source, float u, float v);
    Vertex(Vec3* source, float u, float v);
    ~Vertex();
    Vertex* remap(float u, float v);

    Vec3* position;
    float textureU;
    float textureV;
};

#endif

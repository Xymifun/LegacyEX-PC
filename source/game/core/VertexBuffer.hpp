#ifndef GAME_CORE_VERTEXBUFFER_HPP
#define GAME_CORE_VERTEXBUFFER_HPP

class ByteBuffer;
class VertexFormat;

class VertexBuffer
{
public:
    VertexBuffer(VertexFormat* vertexFormat);

    void bind();
    void unbind();
    void upload(ByteBuffer* buffer);
    void draw(int mode);
    void _delete();
    int getVertexCount();
};

#endif

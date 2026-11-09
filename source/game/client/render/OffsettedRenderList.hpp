#ifndef GAME_CLIENT_RENDER_OFFSETTEDRENDERLIST_HPP
#define GAME_CLIENT_RENDER_OFFSETTEDRENDERLIST_HPP

class Buffer;

class OffsettedRenderList
{
public:
    OffsettedRenderList();

    void init(int x, int y, int z, double cameraX, double cameraY, double cameraZ);
    bool isAt(int x, int y, int z);
    void add(int list);
    void render();
    void clear();

private:
    int x;
    int y;
    int z;
    float cameraX;
    float cameraY;
    float cameraZ;
    Buffer* glListBuffer;
    bool valid;
    bool bufferUploaded;
};

#endif

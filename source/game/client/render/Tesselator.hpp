#ifndef GAME_CLIENT_RENDER_TESSELATOR_HPP
#define GAME_CLIENT_RENDER_TESSELATOR_HPP

class BufferBuilder;

class Tesselator
{
public:
    Tesselator(int bufferSize);

    static Tesselator* getInstance();
    static void CreateNewThreadStorage(int bufferSizeInBytes);
    static void ReleaseThreadStorage();

    BufferBuilder* getBuilder();
    void end();

    BufferBuilder* mBuilder;
    bool* mIsDrawing;
};

#endif

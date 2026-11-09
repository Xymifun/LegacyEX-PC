#ifndef GAME_CLIENT_RENDER_RENDERTARGET_HPP
#define GAME_CLIENT_RENDER_RENDERTARGET_HPP

class RenderTarget
{
public:
    int renderWidth;
    int renderHeight;
    int width;
    int height;

    void resize(int width, int height);
    RenderTarget(int width, int height, bool useDepth);
    void setClearColor(float red, float green, float blue, float alpha);
    void destroyBuffers();
    void unbindWrite();
    void clear();
    void bindWrite(unsigned char setViewport);
    void unbindRead() {}
    void createBuffers(int width, int height);
    void setFilterMode(int filterMode);
    void checkStatus();
    void bindRead();
    void bindRead(int attachment);
    void blitToScreen(int width, int height, bool disableBlend);
    void blitToScreen(int width, int height);
};

#endif

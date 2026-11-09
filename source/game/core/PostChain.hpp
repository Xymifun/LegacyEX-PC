#ifndef GAME_CORE_POSTCHAIN_HPP
#define GAME_CORE_POSTCHAIN_HPP

#include <map>
#include <string>
#include <vector>

class Matrix4f;
class PostPass;
class RenderTarget;
class ResourceLocation;
class ResourceManager;
class TextureManager;

class PostChain
{
public:
    PostChain(TextureManager* textureManager, ResourceManager* resourceManager, RenderTarget* screenTarget,
              ResourceLocation* name);

    void updateOrthoMatrix();
    void destroy();
    RenderTarget* getTempTarget(const std::wstring& name);
    void addTempTarget(const std::wstring& name, int width, int height);
    void addPass(const std::wstring& name, RenderTarget* input, RenderTarget* output);
    void resize(int width, int height);
    void process(float time);
    std::wstring getName();
    RenderTarget* getRenderTarget(const std::wstring& name);

private:
    RenderTarget* mScreenTarget;
    ResourceManager* mResourceManager;
    std::wstring mName;
    std::vector<PostPass*> mPostPasses;
    std::map<std::wstring, RenderTarget*> mCustomRenderTargets;
    std::vector<RenderTarget*> mRenderTargets;
    Matrix4f* mOrthoMatrix;
    int mWidth;
    int mHeight;
    float mTime;
    float mLastStamp;
};

#endif

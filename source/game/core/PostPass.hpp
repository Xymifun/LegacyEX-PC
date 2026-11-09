#ifndef GAME_CORE_POSTPASS_HPP
#define GAME_CORE_POSTPASS_HPP

#include <string>
#include <vector>

class ResourceManager;
class RenderTarget;
class Matrix4f;
class Effect;

class PostPass
{
public:
    PostPass(ResourceManager* resourceManager, std::wstring name, RenderTarget* input, RenderTarget* output);
    ~PostPass();

    void destroy();
    void prepareState();
    void process(float time);
    void setOrthoMatrix(Matrix4f* matrix);
    std::wstring getName();
    void addAuxAsset(std::wstring name, RenderTarget* target, int width, int height);
    Effect* getEffect();

private:
    RenderTarget* mInput;
    RenderTarget* mOutput;
    Effect* mEffect;
    std::vector<RenderTarget*> mAuxAssets;
    std::vector<std::wstring> mAuxAssetNames;
    std::vector<int> mAuxWidths;
    std::vector<int> mAuxHeights;
    Matrix4f* mOrthoMatrix;
};

#endif

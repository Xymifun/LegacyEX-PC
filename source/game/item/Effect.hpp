#ifndef GAME_ITEM_EFFECT_HPP
#define GAME_ITEM_EFFECT_HPP

#include <string>
#include <vector>
#include <stdint.h>
#include <boost/unordered_map.hpp>

class ResourceManager;
class RenderTarget;
class Uniform;
class Program;

class Effect
{
public:
    Effect(ResourceManager* resourceManager, const std::wstring& name);

    void destroy();
    void clear();
    void apply();
    void markDirty();
    Uniform* getUniform(const std::wstring& name);
    Uniform* safeGetUniform(const std::wstring& name);
    void updateLocations();
    void setSampler(std::wstring name, RenderTarget* target);
    Program* getVertexProgram();
    Program* getFragmentProgram();
    std::wstring getName();
    int getId();

private:
    boost::unordered_map<std::wstring, RenderTarget*> mSamplers;
    std::vector<std::wstring> mSamplerNames;
    std::vector<int> mSamplerLocations;
    unsigned char mReservedA[32];
    boost::unordered_map<std::wstring, Uniform*> mUniforms;
    unsigned char mReservedB[36];
    bool mDirty;
};

#endif

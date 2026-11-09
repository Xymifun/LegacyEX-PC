#include "game/item/Effect.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/core/ProgramManager.hpp"
#include "game/core/DummyUniform.hpp"
#include "game/core/Uniform.hpp"

static Effect* sCurrentEffect = 0;
static int sCurrentProgram = -1;
static bool sLastCull = true;
static DummyUniform* sDummyUniform = new DummyUniform();

void Effect::destroy()
{
    ProgramManager* manager = ProgramManager::getInstance();
    manager->releaseProgram(this);
}

void Effect::clear()
{
    sCurrentProgram = -1;
    sCurrentEffect = 0;
    sLastCull = true;
    for (unsigned int i = 0; i < mSamplerLocations.size(); ++i)
    {
        RenderTarget* target = mSamplers[mSamplerNames[i]];
        if (target != 0)
        {
            GlStateManager::activeTexture(i);
            GlStateManager::bindTexture(0);
        }
    }
}

void Effect::apply()
{
    static_cast<void>(this);
}

void Effect::markDirty()
{
    mDirty = true;
}

Uniform* Effect::getUniform(const std::wstring& name)
{
    boost::unordered_map<std::wstring, Uniform*>::iterator it = mUniforms.find(name);
    if (it != mUniforms.end())
        return it->second;
    return 0;
}

Uniform* Effect::safeGetUniform(const std::wstring& name)
{
    Uniform* uniform = getUniform(name);
    if (uniform != 0)
        return uniform;
    return reinterpret_cast<Uniform*>(&sDummyUniform);
}

void Effect::setSampler(std::wstring name, RenderTarget* target)
{
    boost::unordered_map<std::wstring, RenderTarget*>::iterator it = mSamplers.find(name);
    if (it != mSamplers.end())
        mSamplers.erase(name);
    mSamplers[name] = target;
    markDirty();
}

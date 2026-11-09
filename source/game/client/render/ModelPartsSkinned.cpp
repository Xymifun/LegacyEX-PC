#include "game/client/render/ModelPartsSkinned.hpp"

#include "game/core/Cube.hpp"

#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"

ModelPartsSkinned::ModelPartsSkinned() : mDisplayList(-1), mCompiled(false), mParts()
{
}

ModelPartsSkinned::~ModelPartsSkinned()
{
    removeAllModels();
}

void ModelPartsSkinned::addModelPart(ModelPartSkin* part)
{
    part->mBoneIndex = mParts.size();
    mParts.push_back(part);
    mCompiled = false;
}

void ModelPartsSkinned::removeModelPart(ModelPartSkin* part)
{
    int index = -1;

    for (unsigned int i = 0; i < mParts.size(); ++i)
    {
        if (mParts[i] == part)
        {
            index = i;
        }
    }

    if (index == -1)
    {
        return;
    }

    ModelPartSkin* last = mParts.back();
    last->mBoneIndex = index;
    mParts[index] = last;
    mParts.pop_back();

    mCompiled = false;
}

void ModelPartsSkinned::removeAllModels()
{
    mParts.clear();

    if (mDisplayList != -1)
    {
        MemoryTracker::releaseLists(mDisplayList);
    }

    mDisplayList = -1;
}

void ModelPartsSkinned::compile(float scale)
{
    if (mParts.size() == 0)
    {
        return;
    }

    if (mDisplayList != -1)
    {
        MemoryTracker::releaseLists(mDisplayList);
        mDisplayList = -1;
    }

    mDisplayList = MemoryTracker::genLists(1);
    GlStateManager::newList(mDisplayList, 0);

    GlStateManager::enableDepthTest();
    GlStateManager::enableLighting();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();

    for (int i = 0; i < mParts.size(); ++i)
    {
        ModelPartSkin* part = mParts[i];

        for (unsigned int j = 0; j < part->mCubes.size(); ++j)
        {
            part->mCubes.at(j)->renderSkinned(builder, scale, i);
        }
    }

    builder->end();
    GlStateManager::endList(0);

    mCompiled = true;
}

void ModelPartsSkinned::render()
{
    if (mParts.size() == 0)
    {
        return;
    }

    GlStateManager::callList(mDisplayList);
}

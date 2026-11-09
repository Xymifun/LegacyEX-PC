#include "game/client/render/ModelPart.hpp"

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Cube.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Mth.hpp"
#include "game/core/TexOffs.hpp"
#include "vendor/SKIN_BOX.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <cmath>
#include <cstdio>

const float ModelPart::RAD = Mth::RAD_TO_DEG;

ModelPart::ModelPart(Model* model)
{
    construct(model);
}

ModelPart::ModelPart()
{
    _init();
    mModel = NULL;
}

ModelPart::ModelPart(Model* model, int xTexOffs, int yTexOffs)
{
    construct(model, xTexOffs, yTexOffs);
}

ModelPart::ModelPart(Model* model, int xTexOffs, int yTexOffs, int texWidth, int texHeight)
{
    construct(model, xTexOffs, yTexOffs, texWidth, texHeight);
}

ModelPart::ModelPart(Model* model, const std::wstring& name)
{
    construct(model, name);
}

ModelPart::ModelPart(Model* model, const std::wstring& name, const ModelDefinition& definition, ModelPart* parent,
                     float scale)
{
    construct(model);
    setTexSize(definition.getTextureWidth(), definition.getTextureHeight());
    definition[name].SetupModelPart(this, parent, scale);
}

void ModelPart::_init()
{
    mTexWidth = 64.0f;
    mTexHeight = 32.0f;
    mX = 0.0f;
    mY = 0.0f;
    mZ = 0.0f;
    mWorldX = 0.0f;
    mWorldY = 0.0f;
    mWorldZ = 0.0f;
    mXRot = 0.0f;
    mYRot = 0.0f;
    mZRot = 0.0f;
    mCubeXRot = 0.0f;
    mCubeYRot = 0.0f;
    mCubeZRot = 0.0f;
    mMirror = false;
    mShow = true;
    mHidden = false;
    mXOffset = 0.0f;
    mYOffset = 0.0f;
    mZOffset = 0.0f;
    mArmourFlags = 0;
    mSkinId = 0;
    mXRotOffset = 0.0f;
    mYRotOffset = 0.0f;
    mZRotOffset = 0.0f;
    mSkinBoxType = 0;
    mCompiled = false;
    mDisplayList = 0;
    mOriginalY = 0.0f;
    mYBeenOffset = false;
}

void ModelPart::construct(Model* model, const std::wstring& name)
{
    _init();
    mModel = model;
    model->mParts.push_back(this);
    mName = name;
    setTexSize(model->mTexWidth, model->mTexHeight);
}

void ModelPart::construct(Model* model)
{
    _init();
    construct(model, L"");
}

void ModelPart::construct(Model* model, int xTexOffs, int yTexOffs)
{
    _init();
    construct(model);
    texOffs(xTexOffs, yTexOffs);
}

void ModelPart::construct(Model* model, int xTexOffs, int yTexOffs, int texWidth, int texHeight)
{
    _init();
    construct(model);
    texOffs(xTexOffs, yTexOffs);
    setTexSize(texWidth, texHeight);
}

ModelPart* ModelPart::setTexSize(int texWidth, int texHeight)
{
    mTexWidth = (float)texWidth;
    mTexHeight = (float)texHeight;

    return this;
}

ModelPart* ModelPart::texOffs(int xTexOffs, int yTexOffs)
{
    mXTexOffs = xTexOffs;
    mYTexOffs = yTexOffs;

    return this;
}

int ModelPart::getfU()
{
    return mXTexOffs;
}

int ModelPart::getfV()
{
    return mYTexOffs;
}

void ModelPart::setPos(float x, float y, float z)
{
    mX = x;
    mY = y;
    mZ = z;
    mOriginalY = y;
}

void ModelPart::setWorldPos(float x, float y, float z)
{
    mWorldX = x;
    mWorldY = y;
    mWorldZ = z;
}

void ModelPart::mirror()
{
    mMirror = !mMirror;
}

void ModelPart::setYOffset(float offset)
{
    if (mYBeenOffset)
    {
        mY = mOriginalY + offset;
    }
    else
    {
        mOriginalY = mY;
        mY = mY + offset;
    }

    mYBeenOffset = mY != mOriginalY;
}

void ModelPart::clearAllOffsets()
{
    mY = mOriginalY;
    mYBeenOffset = false;
    mXRotOffset = 0.0f;
    mYRotOffset = 0.0f;
    mZRotOffset = 0.0f;
}

void ModelPart::addChild(ModelPart* child)
{
    mChildren.push_back(child);
}

void ModelPart::addChildAndAdjustPivots(ModelPart* child)
{
    child->mX -= mX;
    child->mY -= mY;
    child->mZ -= mZ;
    addChild(child);
}

void ModelPart::PrintCtorValues(const std::string& tag)
{
    std::printf("%s %f %f %f %f %f %f %d %d %d\n", tag.c_str(), mX, mY, mZ, mXRot, mYRot, mZRot, mXTexOffs, mYTexOffs,
                mCubes.size());
}

bool ModelPart::hasYBeenOffset()
{
    return mYBeenOffset;
}

ModelPart* ModelPart::retrieveChild(unsigned int skinId, SKIN_BOX* skinBox, float delta)
{
    ModelPart* child = retrieveChild(skinBox);
    if (child != NULL)
    {
        child->mSkinId = skinId;
        child->mOriginalY = delta;
    }
    return child;
}

ModelPart* ModelPart::retrieveChild(SKIN_BOX* skinBox)
{
    std::vector<ModelPart*>::iterator child;
    for (child = mChildren.begin(); child != mChildren.end(); ++child)
    {
        ModelPart* part = *child;
        std::vector<Cube*>::iterator cube;
        for (cube = part->mCubes.begin(); cube != part->mCubes.end(); ++cube)
        {
            Cube* box = *cube;
            if (box->mX0 == skinBox->x && box->mY0 == skinBox->y && box->mZ0 == skinBox->z
                && box->mX1 == skinBox->x + skinBox->width && box->mY1 == skinBox->y + skinBox->height
                && box->mZ1 == skinBox->z + skinBox->depth)
            {
                return part;
            }
        }
    }
    return NULL;
}

ModelPart* ModelPart::retrieveChildWithSkinIdAndArmourFlags(unsigned int skinId, unsigned int armourFlags)
{
    std::vector<ModelPart*>::iterator child;

    for (child = mChildren.begin(); child != mChildren.end(); ++child)
    {
        if ((*child)->mSkinId == skinId && (*child)->mArmourFlags == armourFlags)
        {
            return *child;
        }
    }

    return NULL;
}

ModelPart* ModelPart::addBox(float x, float y, float z, int width, int height, int depth)
{
    mCubes.push_back(new Cube(this, mXTexOffs, mYTexOffs, x, y, z, width, height, depth, 0.0f, mMirror, Cube::FACE_ALL,
                              false, mCubeXRot, mCubeYRot, mCubeZRot));
    return this;
}

ModelPart* ModelPart::addBox(float x, float y, float z, int width, int height, int depth, bool mirror)
{
    mCubes.push_back(new Cube(this, mXTexOffs, mYTexOffs, x, y, z, width, height, depth, 0.0f, mirror, Cube::FACE_ALL,
                              false, mCubeXRot, mCubeYRot, mCubeZRot));
    return this;
}

void ModelPart::addBox(float x, float y, float z, int width, int height, int depth, float delta)
{
    mCubes.push_back(new Cube(this, mXTexOffs, mYTexOffs, x, y, z, width, height, depth, delta, mMirror, Cube::FACE_ALL,
                              false, mCubeXRot, mCubeYRot, mCubeZRot));
}

ModelPart* ModelPart::addBox(std::wstring id, float x, float y, float z, int width, int height, int depth)
{
    id = mName + L"." + id;

    TexOffs* mapTex = mModel->getMapTex(id);
    texOffs(mapTex->mXTexOffs, mapTex->mYTexOffs);

    mCubes.push_back((new Cube(this, mXTexOffs, mYTexOffs, x, y, z, width, height, depth, 0.0f, mMirror, Cube::FACE_ALL,
                               false, mCubeXRot, mCubeYRot, mCubeZRot))
                         ->setId(id));
    return this;
}

ModelPart* ModelPart::addBoxWithMask(float x, float y, float z, int width, int height, int depth, int faceMask)
{
    mCubes.push_back(new Cube(this, mXTexOffs, mYTexOffs, x, y, z, width, height, depth, 0.0f, mMirror, faceMask, false,
                              mCubeXRot, mCubeYRot, mCubeZRot));
    return this;
}

void ModelPart::addBoxMapTex(float x, float y, float z, int width, int height, int depth, int xTexOffs, int yTexOffs,
                             float delta)
{
    mCubes.push_back(new Cube(this, xTexOffs, yTexOffs, x, y, z, width, height, depth, delta, mMirror, Cube::FACE_ALL,
                              false, mCubeXRot, mCubeYRot, mCubeZRot));
}

void ModelPart::addBoxMapTex(float x, float y, float z, int width, int height, int depth, int xTexOffs, int yTexOffs,
                             bool mirror, float delta)
{
    mCubes.push_back(new Cube(this, xTexOffs, yTexOffs, x, y, z, width, height, depth, delta, mirror, Cube::FACE_ALL,
                              false, mCubeXRot, mCubeYRot, mCubeZRot));
}

void ModelPart::addHumanoidBox(float x, float y, float z, int width, int height, int depth, float delta)
{
    mCubes.push_back(new Cube(this, mXTexOffs, mYTexOffs, x, y, z, width, height, depth, delta, mMirror, Cube::FACE_ALL,
                              true, mCubeXRot, mCubeYRot, mCubeZRot));
}

void ModelPart::render(float scale, bool useDisplayList, bool childrenOnly)
{
    unsigned int i;

    if (mHidden || !mShow)
    {
        return;
    }

    if (!mCompiled)
    {
        compile(scale);
    }

    GlStateManager::translatef(mXOffset, mYOffset, mZOffset);

    if (mXRot != 0.0f || mYRot != 0.0f || mZRot != 0.0f || mYRotOffset != 0.0f)
    {
        PIXBeginNamedEvent(0.0f, "ModelPart::render A");
        GlStateManager::pushMatrix();
        GlStateManager::translatef(mX * scale, mY * scale, mZ * scale);

        if (mZRot != 0.0f)
        {
            GlStateManager::rotatef(mZRot * RAD, 0.0f, 0.0f, 1.0f);
        }

        if (mYRot != 0.0f)
        {
            GlStateManager::rotatef(mYRot * RAD, 0.0f, 1.0f, 0.0f);
        }

        if (mXRot != 0.0f)
        {
            GlStateManager::rotatef(mXRot * RAD, 1.0f, 0.0f, 0.0f);
        }

        GlStateManager::translatef(mXOffset * scale, mYRotOffset * scale, mZOffset * scale);

        if (!childrenOnly)
        {
            if (useDisplayList)
            {
                GlStateManager::callList(mDisplayList);
            }
            else
            {
                BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

                for (i = 0; i < mCubes.size(); i++)
                {
                    mCubes[i]->render(builder, scale);
                }
            }
        }

        for (i = 0; i < mChildren.size(); i++)
        {
            mChildren.at(i)->render(scale, useDisplayList, false);
        }

        GlStateManager::popMatrix();
        PIXEndNamedEvent();
    }
    else if (mX != 0.0f || mY != 0.0f || mZ != 0.0f)
    {
        PIXBeginNamedEvent(0.0f, "ModelPart::render B");
        GlStateManager::translatef(mX * scale, mY * scale, mZ * scale);

        if (!childrenOnly)
        {
            if (useDisplayList)
            {
                GlStateManager::callList(mDisplayList);
            }
            else
            {
                BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

                for (i = 0; i < mCubes.size(); i++)
                {
                    mCubes[i]->render(builder, scale);
                }
            }
        }

        for (i = 0; i < mChildren.size(); i++)
        {
            mChildren.at(i)->render(scale, useDisplayList, false);
        }

        GlStateManager::translatef(-(mX * scale), -(mY * scale), -(mZ * scale));
        PIXEndNamedEvent();
    }
    else
    {
        PIXBeginNamedEvent(0.0f, "ModelPart::render C");

        if (!childrenOnly)
        {
            if (useDisplayList)
            {
                GlStateManager::callList(mDisplayList);
            }
            else
            {
                BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

                for (i = 0; i < mCubes.size(); i++)
                {
                    mCubes[i]->render(builder, scale);
                }
            }
        }

        for (i = 0; i < mChildren.size(); i++)
        {
            mChildren.at(i)->render(scale, useDisplayList, false);
        }

        PIXEndNamedEvent();
    }

    GlStateManager::translatef(-mXOffset, -mYOffset, -mZOffset);
}

void ModelPart::renderRollable(float scale, bool useDisplayList)
{
    if (mHidden || !mShow)
    {
        return;
    }

    if (!mCompiled)
    {
        compile(scale);
    }

    GlStateManager::pushMatrix();
    GlStateManager::translatef(mX * scale, mY * scale, mZ * scale);

    if (mYRot != 0.0f)
    {
        GlStateManager::rotatef(mYRot * RAD, 0.0f, 1.0f, 0.0f);
    }

    if (mXRot != 0.0f)
    {
        GlStateManager::rotatef(mXRot * RAD, 1.0f, 0.0f, 0.0f);
    }

    if (mZRot != 0.0f)
    {
        GlStateManager::rotatef(mZRot * RAD, 0.0f, 0.0f, 1.0f);
    }

    GlStateManager::callList(mDisplayList);
    GlStateManager::popMatrix();
}

void ModelPart::translateTo(float scale)
{
    if (mHidden || !mShow)
    {
        return;
    }

    if (!mCompiled)
    {
        compile(scale);
    }

    if (mXRot != 0.0f || mYRot != 0.0f || mZRot != 0.0f)
    {
        GlStateManager::translatef(mX * scale, mY * scale, mZ * scale);

        if (mZRot != 0.0f)
        {
            GlStateManager::rotatef(mZRot * RAD, 0.0f, 0.0f, 1.0f);
        }

        if (mYRot != 0.0f)
        {
            GlStateManager::rotatef(mYRot * RAD, 0.0f, 1.0f, 0.0f);
        }

        if (mXRot != 0.0f)
        {
            GlStateManager::rotatef(mXRot * RAD, 1.0f, 0.0f, 0.0f);
        }
    }
    else if (mX != 0.0f || mY != 0.0f || mZ != 0.0f)
    {
        GlStateManager::translatef(mX * scale, mY * scale, mZ * scale);
    }
}

void ModelPart::compile(float scale)
{
    unsigned int i;

    mDisplayList = MemoryTracker::genLists(1);
    GlStateManager::newList(mDisplayList, 0);
    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    for (i = 0; i < mCubes.size(); i++)
    {
        mCubes.at(i)->render(builder, scale);
    }

    GlStateManager::endList(0);
    mCompiled = true;
}

#include "game/client/render/ModelPartSkin.hpp"

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPartsSkinned.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Matrix4f.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"

ModelPartSkin::ModelPartSkin() : ModelPart(), mBoneIndex(-1), mSkinnedParts(NULL)
{
    mCompiled = true;
}

ModelPartSkin::ModelPartSkin(Model* model) : ModelPart(model)
{
    mBoneIndex = -1;
    mCompiled = true;
    mSkinnedParts = &model->mSkinnedParts;
    mSkinnedParts->addModelPart(this);
}

ModelPartSkin::ModelPartSkin(Model* model, const std::wstring& name) : ModelPart(model, name)
{
    mBoneIndex = -1;
    mCompiled = true;
    mSkinnedParts = &model->mSkinnedParts;
    mSkinnedParts->addModelPart(this);
}

ModelPartSkin::ModelPartSkin(Model* model, int xTexOffs, int yTexOffs, ModelPartsSkinned* skinnedParts)
    : ModelPart(model, xTexOffs, yTexOffs)
{
    mBoneIndex = -1;

    if (skinnedParts == NULL)
    {
        skinnedParts = &model->mSkinnedParts;
    }

    mCompiled = true;
    mSkinnedParts = skinnedParts;
    mSkinnedParts->addModelPart(this);
}

ModelPartSkin::ModelPartSkin(Model* model, int xTexOffs, int yTexOffs, int texWidth, int texHeight)
    : ModelPart(model, xTexOffs, yTexOffs, texWidth, texHeight)
{
    mBoneIndex = -1;
    mCompiled = true;
    mSkinnedParts = &model->mSkinnedParts;
    mSkinnedParts->addModelPart(this);
}

ModelPartSkin::ModelPartSkin(Model* model, const std::wstring& name, const ModelDefinition& definition,
                             ModelPart* parent, float scale, ModelPartsSkinned* skinnedParts)
    : ModelPart(model, name, definition, parent, scale)
{
    mBoneIndex = -1;

    if (skinnedParts == NULL)
    {
        mCompiled = true;
        mSkinnedParts = &model->mSkinnedParts;
        mSkinnedParts->addModelPart(this);
    }
    else
    {
        mCompiled = true;
        mSkinnedParts = skinnedParts;
        mSkinnedParts->addModelPart(this);
    }
}

ModelPartSkin::~ModelPartSkin()
{
    mSkinnedParts->removeModelPart(this);
}

void ModelPartSkin::setNullBoneMatricesRecursive()
{
    gRenderer->StateSetBoneMatrixNull(mBoneIndex);

    for (unsigned int i = 0; i < mChildren.size(); i++)
    {
        ModelPartSkin* child = dynamic_cast<ModelPartSkin*>(mChildren.at(i));

        if (child != NULL)
        {
            child->setNullBoneMatricesRecursive();
        }
    }
}

void ModelPartSkin::render(float scale, bool useDisplayList, bool childrenOnly)
{
    unsigned int i;

    if (!useDisplayList)
    {
        ModelPart::render(scale, false, childrenOnly);
        return;
    }

    if (!mShow || mHidden)
    {
        setNullBoneMatricesRecursive();
        return;
    }

    GlStateManager::translatef(mXOffset, mYOffset, mZOffset);

    if (mXRot != 0.0f || mYRot != 0.0f || mZRot != 0.0f || mYRotOffset != 0.0f)
    {
        PIXBeginNamedEvent(0.0f, "ModelPart::render A");
        GlStateManager::pushMatrix();

        Matrix4f matrix;
        matrix.setZYXRot(mZRot, mYRot, mXRot);
        matrix.m[12] = mX * scale;
        matrix.m[13] = mY * scale;
        matrix.m[14] = mZ * scale;
        matrix.m[15] = 1.0f;
        GlStateManager::multMatrix(&matrix);

        GlStateManager::translatef(mXOffset * scale, mYRotOffset * scale, mZOffset * scale);

        if (!childrenOnly)
        {
            gRenderer->StateSetBoneMatrixData(mBoneIndex);
        }
        else
        {
            gRenderer->StateSetBoneMatrixNull(mBoneIndex);
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
            gRenderer->StateSetBoneMatrixData(mBoneIndex);
        }
        else
        {
            gRenderer->StateSetBoneMatrixNull(mBoneIndex);
        }

        for (i = 0; i < mChildren.size(); i++)
        {
            if (dynamic_cast<ModelPartSkin*>(mChildren.at(i)) != NULL)
            {
                mChildren.at(i)->render(scale, useDisplayList, false);
            }
        }

        GlStateManager::translatef(-(mX * scale), -(mY * scale), -(mZ * scale));
        PIXEndNamedEvent();
    }
    else
    {
        PIXBeginNamedEvent(0.0f, "ModelPart::render C");

        if (!childrenOnly)
        {
            gRenderer->StateSetBoneMatrixData(mBoneIndex);
        }

        for (i = 0; i < mChildren.size(); i++)
        {
            if (dynamic_cast<ModelPartSkin*>(mChildren.at(i)) != NULL)
            {
                mChildren.at(i)->render(scale, useDisplayList, false);
            }
        }

        PIXEndNamedEvent();
    }

    GlStateManager::translatef(-mXOffset, -mYOffset, -mZOffset);
}

void ModelPartSkin::renderRollable(float scale, bool useDisplayList)
{
    if (mHidden)
    {
        gRenderer->StateSetBoneMatrixNull(mBoneIndex);
        return;
    }

    if (!mShow)
    {
        gRenderer->StateSetBoneMatrixNull(mBoneIndex);
        return;
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

    gRenderer->StateSetBoneMatrixData(mBoneIndex);
    GlStateManager::popMatrix();
}

void ModelPartSkin::compile(float scale)
{
}

#include "game/client/render/PigModel.hpp"

#include "game/client/render/ModelDefinition.hpp"

PigModel::PigModel(const ModelDefinition& modelDefinition) : QuadrupedModel(6, 0.0f, modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        mYHeadOffs = -8.0f;
    }
    else
    {
        mHead->texOffs(16, 16)->addBox(-2.0f, 0.0f, -9.0f, 4, 3, 1, 0.0f);
        mYHeadOffs = 4.0f;
        compileSkinnedDrawlist();
    }
}

#ifdef MINECRAFT_PC
PigModel::PigModel(const ModelDefinition& modelDefinition, bool flatFace)
    : QuadrupedModel(6, 0.0f, modelDefinition)
{
    if (!flatFace)
        mHead->texOffs(16, 16)->addBox(-2.0f, 0.0f, -9.0f, 4, 3, 1, 0.0f);
    mYHeadOffs = 4.0f;
    compileSkinnedDrawlist();
}
#endif

PigModel::PigModel(float scale, const ModelDefinition& modelDefinition) : QuadrupedModel(6, scale, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mHead->texOffs(16, 16)->addBox(-2.0f, 0.0f, -9.0f, 4, 3, 1, scale);
        mYHeadOffs = 4.0f;
        compileSkinnedDrawlist();
    }
    else
    {
        mHasOverriddenParts = true;
    }
}

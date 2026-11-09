#include "game/client/render/Model.hpp"

#include "game/client/render/ModelPart.hpp"
#include "game/util/Random.hpp"

Model::Model()
{
    mRiding = false;
    mYoung = true;
    mRidingDolphin = false;
    mTexWidth = 64;
    mTexHeight = 32;
    mHasOverriddenParts = false;
}

Model::~Model()
{
}

void Model::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                   float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
}

void Model::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                      float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
}

void Model::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks)
{
}

ModelPart* Model::getRandomModelPart(Random random)
{
    return mParts.at(random.nextInt(mParts.size()));
}

ModelPart* Model::AddOrRetrievePart(SKIN_BOX* skinBox, unsigned int armourFlags)
{
    return NULL;
}

void Model::setMapTex(std::wstring name, int xTexOffs, int yTexOffs)
{
    mModelTextureMap[name] = new TexOffs(xTexOffs, yTexOffs);
}

TexOffs* Model::getMapTex(std::wstring name)
{
    return mModelTextureMap[name];
}

void Model::copyModelPart(ModelPart* source, ModelPart* destination)
{
    destination->mXRot = source->mXRot;
    destination->mYRot = source->mYRot;
    destination->mZRot = source->mZRot;
    destination->mX = source->mX;
    destination->mY = source->mY;
    destination->mZ = source->mZ;
}

void Model::copyPropertiesFrom(Model* model)
{
    mRiding = model->mRiding;
    mYoung = model->mYoung;
    mRidingDolphin = model->mRidingDolphin;
    mAttackTime = model->mAttackTime;
    mHasOverriddenParts = model->mHasOverriddenParts;
}

void Model::clearAllOffsets()
{
}

void Model::SetSkinAdjustments(_SkinAdjustments skinAdjustments)
{
}

std::wstring Model::getName()
{
    return L"";
}

void Model::compileSkinnedDrawlist()
{
    mSkinnedParts.compile(0.0625f);
}

void Model::callSkinnedDrawlist()
{
    mSkinnedParts.render();
}

bool Model::hasOverriddenParts()
{
    return mHasOverriddenParts;
}

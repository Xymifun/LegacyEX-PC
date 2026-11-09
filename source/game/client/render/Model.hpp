#ifndef GAME_CLIENT_RENDER_MODEL_HPP
#define GAME_CLIENT_RENDER_MODEL_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/client/render/ModelPartsSkinned.hpp"
#include "game/core/TexOffs.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelPart;
class Random;
struct SKIN_BOX;

class Model
{
public:
    Model();

    virtual ~Model();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual ModelPart* getRandomModelPart(Random random);
    virtual ModelPart* AddOrRetrievePart(SKIN_BOX* skinBox, unsigned int armourFlags);
    virtual void copyPropertiesFrom(Model* model);
    virtual void clearAllOffsets();
    virtual void SetSkinAdjustments(_SkinAdjustments skinAdjustments);
    virtual std::wstring getName();

    void setMapTex(std::wstring name, int xTexOffs, int yTexOffs);
    TexOffs* getMapTex(std::wstring name);

    static void copyModelPart(ModelPart* source, ModelPart* destination);

    void compileSkinnedDrawlist();
    void callSkinnedDrawlist();
    bool hasOverriddenParts();

    float mAttackTime;
    bool mRiding;
    std::vector<ModelPart*> mParts;
    bool mYoung;
    bool mRidingDolphin;
    boost::unordered_map<std::wstring, TexOffs*> mModelTextureMap;
    int mTexWidth;
    int mTexHeight;
    ModelPartsSkinned mSkinnedParts;
    float mYHeadOffs;
    float mZHeadOffs;
    bool mHasOverriddenParts;
};

#endif

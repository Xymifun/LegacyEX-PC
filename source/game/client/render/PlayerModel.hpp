#ifndef GAME_CLIENT_RENDER_PLAYERMODEL_HPP
#define GAME_CLIENT_RENDER_PLAYERMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class HumanoidArm;
class ModelPart;
struct SKIN_BOX;

class PlayerModel : public HumanoidModel
{
public:
    PlayerModel(float scale, bool slimArms, bool is64x64);


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual ModelPart* AddOrRetrievePart(SKIN_BOX* skinBox, unsigned int skinId);
    virtual void clearAllOffsets();
    virtual void SetSkinAdjustments(_SkinAdjustments skinAdjustments);
    virtual void render(HumanoidModel* model, float scale, bool useDisplayList);
    virtual void setAllVisible(bool visible);
    virtual void translateToHandItem(float scale, const HumanoidArm* arm);

    void renderEars(float scale);
    void renderCloak(float scale);

    void copyModelPartsExceptHead();

    ModelPart* mLeftSleeve;
    ModelPart* mRightSleeve;
    ModelPart* mLeftPants;
    ModelPart* mRightPants;
    ModelPart* mJacket;
    ModelPart* mLeftShoulder;
    ModelPart* mRightShoulder;
    ModelPart* mLeftLegging;
    ModelPart* mRightLegging;
    ModelPart* mChest;
    ModelPart* mWaist;
    ModelPart* mLeftShin;
    ModelPart* mRightShin;
    ModelPart* mLeftBoot;
    ModelPart* mRightBoot;
    ModelPart* mCloak;
    ModelPart* mEar;
    bool mSlimArms;
    bool mIs64x64;
};

#endif

#ifndef GAME_CLIENT_RENDER_HUMANOIDMODEL_HPP
#define GAME_CLIENT_RENDER_HUMANOIDMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Model.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class EquipmentSlot;
class HumanoidArm;
class ModelDefinition;
class ModelPart;
struct SKIN_BOX;

class HumanoidModel : public Model
{
public:
    enum eArmPose
    {
        eArmPose_Empty = 0,
        eArmPose_Item = 1,
        eArmPose_Block = 2,
        eArmPose_BowAndArrow = 3,
        eArmPose_ThrowSpear = 4
    };

    enum animbits
    {
        eAnimBit_ArmsDown = 0,
        eAnimBit_ArmsOut = 1,
        eAnimBit_LegsStill = 2,
        eAnimBit_LegsInSync = 5,
        eAnimBit_ArmsInSync = 6,
        eAnimBit_RightArmOverride = 7,
        eAnimBit_Head = 10,
        eAnimBit_RightArm = 11,
        eAnimBit_LeftArm = 12,
        eAnimBit_Body = 13,
        eAnimBit_RightLeg = 14,
        eAnimBit_LeftLeg = 15,
        eAnimBit_Hat = 16,
        eAnimBit_Seated = 17,
        eAnimBit_LeftSleeve = 20,
        eAnimBit_RightSleeve = 21,
        eAnimBit_LeftPants = 22,
        eAnimBit_RightPants = 23,
        eAnimBit_Jacket = 24,
        eAnimBit_ArmourHead = 25,
        eAnimBit_ArmourRightArm = 26,
        eAnimBit_ArmourLeftArm = 27,
        eAnimBit_ArmourBody = 28,
        eAnimBit_ArmourRightLeg = 29,
        eAnimBit_ArmourLeftLeg = 30
    };

    static const float HAT_INFLATE;
    static const float OVERLAY_INFLATE;

    HumanoidModel(const ModelDefinition& definition, bool armourModel);
    HumanoidModel(const ModelDefinition& definition, float scale, bool armourModel);
    HumanoidModel(const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight,
                  bool armourModel, bool createParts, bool skinned);
    HumanoidModel(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                  const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight,
                  bool armourModel, bool createParts, bool skinned);

    virtual ~HumanoidModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual ModelPart* AddOrRetrievePart(SKIN_BOX* skinBox, unsigned int armourFlags);
    virtual void copyPropertiesFrom(Model* model);
    virtual void render(HumanoidModel* model, float scale, bool useDisplayList);
    virtual void setAllVisible(bool visible);
    virtual void translateToHandItem(float scale, const HumanoidArm* arm);
    virtual void setArmPose(const HumanoidArm* arm, eArmPose pose);
    virtual void setArmPose(boost::shared_ptr<Entity> entity, eArmPose pose, bool useAttackArm);

    void _init(float scale, float yOffset, int texWidth, int texHeight, bool armourModel, bool createParts,
               const ModelDefinition& definition, bool skinned);

    ModelPart* createModelPart(bool skinned, const std::wstring& name, const ModelDefinition& definition,
                               ModelPart* parent, float scale);
    ModelPart* createModelPart(bool skinned, int xTexOffs, int yTexOffs);

    void overrideAnim();
    void overrideRender(float scale, bool useDisplayList);

    bool IsBodyPartDisabled(animbits bodyPart);

    void setupBoneMatrices(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    void setupBoneMatrices(HumanoidModel* model, float scale, bool useDisplayList);

    const HumanoidArm* getAttackArm(boost::shared_ptr<Entity> entity);
    ModelPart* getArm(const HumanoidArm* arm);

    float quadraticArmUpdate(float value);

    ModelPart* mHead;
    ModelPart* mHat;
    ModelPart* mBody;
    ModelPart* mRightArm;
    ModelPart* mLeftArm;
    ModelPart* mRightLeg;
    ModelPart* mLeftLeg;
    ModelPart* mRightItem;
    ModelPart* mLeftItem;
    eArmPose mLeftArmPose;
    eArmPose mRightArmPose;
    bool mSitting;
    bool mCrouching;
    bool mEating;
    bool mSwimming;
    float mSpearThrowProgress;
    float mUseItemRemainingTicks;
    float mUseItemProgress;
    _SkinAdjustments mSkinAdjustments;
    float mYOffset;
    bool mIsArmourModel;
    const EquipmentSlot* mArmourSlot;
    float mSwimAmount;
};

#endif

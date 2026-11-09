#ifndef GAME_CLIENT_RENDER_ENDERMANMODEL_HPP
#define GAME_CLIENT_RENDER_ENDERMANMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartsSkinned.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class EndermanModel : public HumanoidModel
{
public:
    enum eModelPart
    {
        eModelPart_Head = 0,
        eModelPart_Hat = 1,
        eModelPart_Body = 2,
        eModelPart_RightArm = 3,
        eModelPart_LeftArm = 4,
        eModelPart_RightLeg = 5,
        eModelPart_LeftLeg = 6,
        eModelPart_RightItem = 7,
        eModelPart_LeftItem = 8,
        eModelPart_InvisibleHead = 9,
        eModelPart_InvisibleHat = 10,
        eModelPart_InvisibleBody = 11,
        eModelPart_InvisibleRightArm = 12,
        eModelPart_InvisibleLeftArm = 13,
        eModelPart_InvisibleRightLeg = 14,
        eModelPart_InvisibleLeftLeg = 15,
        eModelPart_InvisibleRightItem = 16,
        eModelPart_InvisibleLeftItem = 17,
        eModelPart_Count = 18
    };

    EndermanModel(float scale, const ModelDefinition& definition = ModelDefinition());

    virtual ~EndermanModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"enderman"; }

    void createModelParts(float scale, const ModelDefinition& definition);
    void createInvisibleModelParts(float scale, const ModelDefinition& definition);

    bool mCarrying;
    bool mCreepy;
    ModelPart** mModelParts;
    int mNumModelParts;
    ModelPartsSkinned mInvisibleParts;
};

#endif

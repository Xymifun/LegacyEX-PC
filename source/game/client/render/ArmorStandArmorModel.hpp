#ifndef GAME_CLIENT_RENDER_ARMORSTANDARMORMODEL_HPP
#define GAME_CLIENT_RENDER_ARMORSTANDARMORMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;

class ArmorStandArmorModel : public HumanoidModel
{
public:
    ArmorStandArmorModel(float scale, int texWidth, int texHeight,
                         const ModelDefinition& definition = ModelDefinition());

    virtual ~ArmorStandArmorModel();

    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
};

#endif

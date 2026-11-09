#ifndef GAME_CLIENT_RENDER_ENDERMITEMODEL_HPP
#define GAME_CLIENT_RENDER_ENDERMITEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class EndermiteModel : public Model
{
public:
    EndermiteModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~EndermiteModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"endermite"; }

    static const int BODY_PART_COUNT = 4;
    static const int BODY_PART_SIZES[BODY_PART_COUNT][3];
    static const int BODY_PART_TEX_OFFS[BODY_PART_COUNT][2];

    ModelPart* mBodyParts[BODY_PART_COUNT];
};

#endif

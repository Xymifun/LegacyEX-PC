#ifndef GAME_CLIENT_RENDER_SILVERFISHMODEL_HPP
#define GAME_CLIENT_RENDER_SILVERFISHMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class SilverfishModel : public Model
{
public:
    SilverfishModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~SilverfishModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"silverfish"; }

    static const int BODY_PART_COUNT = 7;
    static const int BODY_LAYER_COUNT = 3;
    static const int BODY_PART_TEX_OFFS[BODY_PART_COUNT][2];
    static const int BODY_PART_SIZES[BODY_PART_COUNT][3];

    arrayWithLength<ModelPart*> mBodyParts;
    arrayWithLength<ModelPart*> mBodyLayers;
    float mBodyPartZ[BODY_PART_COUNT];
};

#endif

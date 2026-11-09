#ifndef GAME_CLIENT_RENDER_SKIMODEL_HPP
#define GAME_CLIENT_RENDER_SKIMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "vendor/_SkinAdjustments.hpp"

class ModelDefinition;
class ModelPart;

class SkiModel : public Model
{
public:
    SkiModel(bool mirrored);
    SkiModel(const ModelDefinition& modelDefinition);


    void _init(bool mirrored);

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);

    arrayWithLength<ModelPart*> mSkiParts;
    bool mMirrored;
};

#endif

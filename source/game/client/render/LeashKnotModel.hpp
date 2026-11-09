#ifndef GAME_CLIENT_RENDER_LEASHKNOTMODEL_HPP
#define GAME_CLIENT_RENDER_LEASHKNOTMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"


class LeashKnotModel : public Model
{
public:
    LeashKnotModel(const ModelDefinition& modelDefinition = ModelDefinition());
    LeashKnotModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight);


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);

    void _init(int xTexOffs, int yTexOffs, int texWidth, int texHeight);

    ModelPart* mKnot;
};

#endif

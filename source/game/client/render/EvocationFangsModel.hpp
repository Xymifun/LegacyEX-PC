#ifndef GAME_CLIENT_RENDER_EVOCATIONFANGSMODEL_HPP
#define GAME_CLIENT_RENDER_EVOCATIONFANGSMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class ModelPartSkin;

class EvocationFangsModel : public Model
{
public:
    EvocationFangsModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~EvocationFangsModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);

    ModelPartSkin* mBase;
    ModelPartSkin* mUpperJaw;
    ModelPartSkin* mLowerJaw;
};

#endif

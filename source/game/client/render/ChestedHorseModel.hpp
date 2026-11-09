#ifndef GAME_CLIENT_RENDER_CHESTEDHORSEMODEL_HPP
#define GAME_CLIENT_RENDER_CHESTEDHORSEMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/AbstractHorseModel.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;

class ChestedHorseModel : public AbstractHorseModel
{
public:
    ChestedHorseModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
};

#endif

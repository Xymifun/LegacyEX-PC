#ifndef GAME_CLIENT_RENDER_SPINATTACKMODEL_HPP
#define GAME_CLIENT_RENDER_SPINATTACKMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class ModelPartSkin;

class SpinAttackModel : public Model
{
public:
    SpinAttackModel(const ModelDefinition& modelDefinition = ModelDefinition());


    void renderSpin(boost::shared_ptr<Entity> entity, float ageInTicks, float scale, bool useDisplayList);

    ModelPartSkin* mBox;
};

#endif

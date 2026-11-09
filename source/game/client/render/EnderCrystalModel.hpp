#ifndef GAME_CLIENT_RENDER_ENDERCRYSTALMODEL_HPP
#define GAME_CLIENT_RENDER_ENDERCRYSTALMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class ModelPartSkin;

class EnderCrystalModel : public Model
{
public:
    EnderCrystalModel(float scale, bool renderBase, const ModelDefinition& modelDefinition = ModelDefinition());

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);

    ModelPartSkin* mCube;
    ModelPartSkin* mGlass;
    ModelPartSkin* mBase;
};

#endif

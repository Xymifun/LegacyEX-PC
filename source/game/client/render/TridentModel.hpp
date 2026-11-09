#ifndef GAME_CLIENT_RENDER_TRIDENTMODEL_HPP
#define GAME_CLIENT_RENDER_TRIDENTMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class ModelPartSkin;

class TridentModel : public Model
{
public:
    TridentModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName() { return L"trident"; }

    ModelPartSkin* mPole;
};

#endif

#ifndef GAME_CLIENT_RENDER_SHIELDMODEL_HPP
#define GAME_CLIENT_RENDER_SHIELDMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelDefinition;
class ModelPart;

class ShieldModel : public Model
{
public:
    ShieldModel(const ModelDefinition& modelDefinition);


    void render(bool useDisplayList);

    ModelPart* mPlate;
    ModelPart* mHandle;
    ModelPart* mPlateFront;
};

#endif

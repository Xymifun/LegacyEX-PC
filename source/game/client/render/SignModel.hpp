#ifndef GAME_CLIENT_RENDER_SIGNMODEL_HPP
#define GAME_CLIENT_RENDER_SIGNMODEL_HPP

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class ModelPartSkin;

class SignModel : public Model
{
public:
    SignModel(const ModelDefinition& modelDefinition = ModelDefinition());


    void render(bool useDisplayList);

    ModelPartSkin* mSignBoard;
    ModelPartSkin* mSignStick;
};

#endif

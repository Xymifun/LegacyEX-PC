#ifndef GAME_CLIENT_RENDER_BANNERMODEL_HPP
#define GAME_CLIENT_RENDER_BANNERMODEL_HPP

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class ModelPartSkin;

class BannerModel : public Model
{
public:
    BannerModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~BannerModel();

    void render();

    ModelPartSkin* mFlag;
    ModelPartSkin* mPole;
    ModelPartSkin* mBar;
};

#endif

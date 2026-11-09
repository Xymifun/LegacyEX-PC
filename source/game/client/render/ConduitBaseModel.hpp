#ifndef GAME_CLIENT_RENDER_CONDUITBASEMODEL_HPP
#define GAME_CLIENT_RENDER_CONDUITBASEMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelPartSkin;

class ConduitBaseModel : public Model
{
public:
    ConduitBaseModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight);


    void renderBase(bool useDisplayList);

    ModelPartSkin* mBase;
};

#endif

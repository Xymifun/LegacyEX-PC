#ifndef GAME_CLIENT_RENDER_CONDUITCAGEMODEL_HPP
#define GAME_CLIENT_RENDER_CONDUITCAGEMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelPartSkin;

class ConduitCageModel : public Model
{
public:
    ConduitCageModel(int texOffsX, int texOffsY, int texWidth, int texHeight);


    void renderCage(bool useDisplayList);

    ModelPartSkin* mCage;
};

#endif

#ifndef GAME_CLIENT_RENDER_COWMODEL_HPP
#define GAME_CLIENT_RENDER_COWMODEL_HPP

#include <string>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/QuadrupedModel.hpp"


class CowModel : public QuadrupedModel
{
public:
    CowModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual std::wstring getName() { return L"cow"; }
};

#endif

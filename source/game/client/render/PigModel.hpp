#ifndef GAME_CLIENT_RENDER_PIGMODEL_HPP
#define GAME_CLIENT_RENDER_PIGMODEL_HPP

#include <string>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/QuadrupedModel.hpp"


class PigModel : public QuadrupedModel
{
public:
    PigModel(const ModelDefinition& modelDefinition = ModelDefinition());
    PigModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());
#ifdef MINECRAFT_PC
    PigModel(const ModelDefinition& modelDefinition, bool flatFace);
#endif


    virtual std::wstring getName() { return L"pig"; }
};

#endif

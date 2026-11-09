#ifndef GAME_CLIENT_RENDER_BEDMODEL_HPP
#define GAME_CLIENT_RENDER_BEDMODEL_HPP

#include <stdint.h>
#include <string>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class ModelPartSkin;

class BedModel : public Model
{
public:
    BedModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~BedModel() {}

    virtual std::wstring getName() { return L"bed"; }

    uint32_t modelVersion();

    void render(bool useDisplayList);

    ModelPartSkin* mBed;
    ModelPartSkin* mLeg0;
    ModelPartSkin* mLeg1;
    ModelPartSkin* mLeg2;
    ModelPartSkin* mLeg3;
};

#endif

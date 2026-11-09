#ifndef GAME_CLIENT_RENDER_BLAZEMODEL_HPP
#define GAME_CLIENT_RENDER_BLAZEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class BlazeModel : public Model
{
public:
    BlazeModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"blaze"; }

    arrayWithLength<ModelPartSkin*> mUpperBodyParts;
    ModelPartSkin* mHead;
};
#endif

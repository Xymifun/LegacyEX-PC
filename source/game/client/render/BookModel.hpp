#ifndef GAME_CLIENT_RENDER_BOOKMODEL_HPP
#define GAME_CLIENT_RENDER_BOOKMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPartSkin;

class BookModel : public Model
{
public:
    BookModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);

    ModelPartSkin* mLeftLid;
    ModelPartSkin* mRightLid;
    ModelPartSkin* mLeftPages;
    ModelPartSkin* mRightPages;
    ModelPartSkin* mFlipPage1;
    ModelPartSkin* mFlipPage2;
    ModelPartSkin* mSeam;
};

#endif

#ifndef GAME_CLIENT_RENDER_SKELETONHEADMODEL_HPP
#define GAME_CLIENT_RENDER_SKELETONHEADMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"


class SkeletonHeadModel : public Model
{
public:
    SkeletonHeadModel(const ModelDefinition& modelDefinition = ModelDefinition());
    SkeletonHeadModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight,
                      const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~SkeletonHeadModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"mob_head"; }

    void _init(int xTexOffs, int yTexOffs, int texWidth, int texHeight, const ModelDefinition& modelDefinition);

    ModelPart* mHead;
};

#endif

#include "game/client/render/VindicationIllagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/IllagerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"

static ResourceLocation vindicationIllagerLocation(TN_MOB_ILLAGER_VINDICATOR);

VindicationIllagerRenderer::VindicationIllagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new IllagerModel(0.0f, 0.0f, 64, 64), 0.5f)
{
    class AnonymousItemInHandLayer : public ItemInHandLayer
    {
    public:
        explicit AnonymousItemInHandLayer(VindicationIllagerRenderer* renderer) : ItemInHandLayer(renderer) {}
        virtual ~AnonymousItemInHandLayer() {}
        virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                            bool bUseDisplayList)
        {
            boost::shared_ptr<VindicationIllager> vindicationIllager = boost::static_pointer_cast<VindicationIllager>(
                entity);
            if (vindicationIllager->isAgressive())
            {
                ItemInHandLayer::render(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw,
                                        headPitch, scale, bUseDisplayList);
            }
        }
        virtual void translateToHand(const HumanoidArm* arm)
        {
            IllagerModel* model = static_cast<IllagerModel*>(mLivingEntityRenderer->getModel());
            ModelPart* part = model->getArm(arm);
            part->translateTo(0.0625f);
        }
    };

    addLayer(new AnonymousItemInHandLayer(this));
}

void VindicationIllagerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                        float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* VindicationIllagerRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &vindicationIllagerLocation;
}

void VindicationIllagerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

Model* VindicationIllagerRenderer::createModel(const ModelDefinition& definition)
{
    return new IllagerModel(0.0f, 0.0f, 64, 64, definition);
}

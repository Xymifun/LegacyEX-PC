#include "game/client/render/EndermanRenderer.hpp"

#include <algorithm>

#include "game/client/render/EndermanModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/CarriedBlockLayer.hpp"
#include "game/world/generation/EnderEyesLayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static ResourceLocation ENDERMAN_LOCATION(TN_MOB_ENDERMAN);

EndermanRenderer::EndermanRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new EndermanModel(0.0F), 0.5F)
{
    mEyesLayer = new EnderEyesLayer(this);
    addLayer(mEyesLayer);
    addLayer(new CarriedBlockLayer(this));
}

void EndermanRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                              float partialTick)
{
    boost::shared_ptr<EnderMan> enderman = boost::static_pointer_cast<EnderMan>(entity);
    const BlockState* carried = enderman->getCarryingBlock();
    EndermanModel* model = static_cast<EndermanModel*>(getModel());
    model->mCarrying = carried != NULL;
    bool creepy = enderman->isCreepy();
#ifdef MINECRAFT_PC
    if (PcGoldenAgeOn(kPcGoldenAgeModels))
        creepy = false;
#endif
    model->mCreepy = creepy;

    if (creepy)
    {
        const double offset = 0.02;
        x += mRandom.nextGaussian() * offset;
        z += mRandom.nextGaussian() * offset;
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* EndermanRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDERMAN_LOCATION;
}

void EndermanRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mEyesLayer);
        return;
    }

    if (std::find(mLayers.begin(), mLayers.end(), mEyesLayer) == mLayers.end())
        addLayer(mEyesLayer);
}

void EndermanRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                   float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    bool translucent = !isVisible(entity) && !entity->isInvisibleTo(Minecraft::GetInstance()->mPlayer);

    boost::shared_ptr<Entity> renderEntity = entity;
    if (!bindTexture(renderEntity))
    {
        return;
    }

    if (translucent)
    {
        GlStateManager::setProfile();
    }
    getModel(entity)->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
    if (translucent)
    {
        GlStateManager::unsetProfile();
    }
}

Model* EndermanRenderer::createModel(const ModelDefinition& definition)
{
    return new EndermanModel(0.0F, definition);
}

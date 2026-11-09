#include "game/client/render/StrayRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonModel.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StrayClothingLayer.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation STRAY_LOCATION(TN_MOB_SKELETON_STRAY);

StrayRenderer::StrayRenderer(EntityRenderDispatcher* dispatcher)
    : SkeletonRenderer(dispatcher, new SkeletonModel(SkeletonModel::eSkeletonType_Stray))
{
    mStrayClothingLayer = new StrayClothingLayer(this);
    addLayer(mStrayClothingLayer);
}

std::wstring StrayRenderer::getModelName()
{
    return L"skeleton.stray";
}

const ResourceLocation* StrayRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &STRAY_LOCATION;
}

void StrayRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                 bool overlay)
{
    bool visible = isVisible(entity);
    bool invisibleToPlayer = !visible && !entity->isInvisibleTo(Minecraft::GetInstance()->mPlayer);

    for (std::vector<RenderLayer*>::iterator it = mLayers.begin(); it != mLayers.end(); ++it)
    {
        RenderLayer* layer = *it;
        if ((!visible || invisibleToPlayer) && *it == mStrayClothingLayer)
        {
            continue;
        }

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            && boost::static_pointer_cast<Player>(entity)->isSpectator()
            && boost::static_pointer_cast<Player>(entity)->SpectateAsFloatingHead()
            && (layer == NULL || dynamic_cast<CustomHeadLayer*>(layer) == NULL))
        {
            continue;
        }

        bool damageOverlay = setupOverlayColor(entity, partialTick, layer->colorsOnDamage());
        layer->render(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw, headPitch, scale,
                      overlay);
        if (damageOverlay)
            teardownOverlayColor();
    }
}

Model* StrayRenderer::createModel(const ModelDefinition& definition)
{
    return new SkeletonModel(SkeletonModel::eSkeletonType_Stray, definition);
}

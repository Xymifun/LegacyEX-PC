#include "game/client/render/HorseRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/AbstractHorseModel.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static std::map<std::wstring, ResourceLocation*> LAYERED_LOCATION_CACHE;

HorseRenderer::HorseRenderer(EntityRenderDispatcher* dispatcher)
    : AbstractHorseRenderer(dispatcher, new AbstractHorseModel(), 1.1F)
{
}

void HorseRenderer::bindTexture(const ResourceLocation* location)
{
    mDispatcher->mTextures->bindTextureLayers(location);
}

const ResourceLocation* HorseRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Horse> horse = boost::static_pointer_cast<Horse>(entity);
    std::wstring hashName = horse->getLayeredTextureHashName();
    std::map<std::wstring, ResourceLocation*>::iterator location = LAYERED_LOCATION_CACHE.find(hashName);

    if (location == LAYERED_LOCATION_CACHE.end())
    {
        LAYERED_LOCATION_CACHE[hashName] = new ResourceLocation(horse->getLayeredTextureLayers());
        location = LAYERED_LOCATION_CACHE.find(hashName);
    }

    return location->second;
}

Model* HorseRenderer::createModel(const ModelDefinition& definition)
{
    return new AbstractHorseModel(true, definition);
}

void HorseRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    if (entity->isInvisible())
    {
        mModel->setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                          _SkinAdjustments());
    }
    else
    {
        boost::shared_ptr<Entity> renderEntity(entity);
        const ResourceLocation* location = getTextureLocation(renderEntity);
        int textureCount = location == NULL ? 0 : location->getTextureCount();
        bool dyedArmor = false;
        boost::shared_ptr<Horse> horse;

        if (eTYPE_DERIVED_FROM(EntityType::HORSE, entity->GetType()))
        {
            horse = boost::static_pointer_cast<Horse>(entity);
            if (horse->isWearingDyedArmor())
            {
                unsigned int color = horse->getDyedArmorColor();
                float red = (float)(int)((color >> 16) & 0xff) / 255.0F;
                float green = (float)(int)((color >> 8) & 0xff) / 255.0F;
                float blue = (float)(int)(color & 0xff) / 255.0F;
                GlStateManager::texLayerColor3f(horse->getDyedArmorLayer(), red, green, blue);
                dyedArmor = true;
            }
        }

        EntityRenderer::bindTexture(renderEntity);
        mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);

        for (int layer = 1; layer < textureCount; ++layer)
        {
            gRenderer->TextureBind(layer, -1);
        }

        if (dyedArmor)
        {
            GlStateManager::texLayerColor3f(horse->getDyedArmorLayer(), 1.0F, 1.0F, 1.0F);
        }
    }
}

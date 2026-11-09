#include "game/client/render/TropicalFishRenderer.hpp"

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/FishModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/TropicalFish.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static std::map<std::wstring, ResourceLocation*> LAYERED_LOCATION_CACHE;

TropicalFishRenderer::TropicalFishRenderer(EntityRenderDispatcher* dispatcher)
    : FishRenderer(dispatcher, FISH_TYPE_TROPICAL_FISH, 0.1F)
{
    mModel2 = new FishModel(ModelDefinition(), FISH_TYPE_TROPICAL_FISH, 1);
}

void TropicalFishRenderer::getRGB(int color, float& red, float& green, float& blue)
{
    const arrayWithLength<const float> colors = DyeColor::byBlockData(color)->getTextureDiffuseColors();
    red = colors.data[0] * 255.0F / 255.0F;
    green = colors.data[1] * 255.0F / 255.0F;
    blue = colors.data[2] * 255.0F / 255.0F;
}

void TropicalFishRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                       float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    boost::shared_ptr<TropicalFish> fish = boost::static_pointer_cast<TropicalFish>(entity);

    if (entity->isInvisible())
    {
        if (fish->getBaseVariant() == 0)
        {
            mModel->setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                              _SkinAdjustments());
        }
        else
        {
            mModel2->setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                               _SkinAdjustments());
        }
    }
    else
    {
        boost::shared_ptr<Entity> renderEntity = entity;
        const ResourceLocation* location = getTextureLocation(renderEntity);
        int textureCount = location == NULL ? 0 : location->getTextureCount();
        int color1 = fish->getColor1();
        int color2 = fish->getColor2();
        float red1;
        float green1;
        float blue1 = 1.0F;
        float red2;
        float green2;
        float blue2 = 1.0F;
        getRGB(color1, red1, green1, blue1);
        getRGB(color2, red2, green2, blue2);
        GlStateManager::texLayerColor3f(0, red1, green1, blue1);
        GlStateManager::texLayerColor3f(1, red2, green2, blue2);

        EntityRenderer::bindTexture(renderEntity);
        if (fish->getBaseVariant() == 0)
        {
            mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
        }
        else
        {
            mModel2->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
        }

        for (int layer = 1; layer < textureCount; ++layer)
            gRenderer->TextureBind(layer, -1);

        GlStateManager::texLayerColor3f(0, 1.0F, 1.0F, 1.0F);
        GlStateManager::texLayerColor3f(1, 1.0F, 1.0F, 1.0F);
    }
}

void TropicalFishRenderer::bindTexture(const ResourceLocation* location)
{
    mDispatcher->mTextures->bindTextureLayers(location);
}

const ResourceLocation* TropicalFishRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<TropicalFish> fish = boost::static_pointer_cast<TropicalFish>(entity);
    std::wstring hashName = fish->getLayeredTextureHashName();
    std::map<std::wstring, ResourceLocation*>::iterator location = LAYERED_LOCATION_CACHE.find(hashName);

    if (location == LAYERED_LOCATION_CACHE.end())
    {
        LAYERED_LOCATION_CACHE[hashName] = new ResourceLocation(fish->getLayeredTextureLayers());
        location = LAYERED_LOCATION_CACHE.find(hashName);
    }

    return location->second;
}

Model* TropicalFishRenderer::getModel(boost::shared_ptr<LivingEntity>& entity)
{
    boost::shared_ptr<TropicalFish> fish = boost::static_pointer_cast<TropicalFish>(entity);
    if ((fish->getVariant() & 1) == 0)
        return mModel;
    return mModel2;
}

Model* TropicalFishRenderer::createModel(const ModelDefinition& definition)
{
    TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    if (selected != NULL && selected->HasCustomModel(mModel2->getName()))
        mModel2 = new FishModel(*selected->GetCustomModel(mModel2->getName()), FISH_TYPE_TROPICAL_FISH, 1);
    return new FishModel(definition, FISH_TYPE_TROPICAL_FISH, 0);
}

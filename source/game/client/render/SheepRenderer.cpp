#include "game/client/render/SheepRenderer.hpp"

#include <string>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SheepFurModel.hpp"
#include "game/client/render/SheepModel.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Sheep.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/SheepFurLayer.hpp"

static ResourceLocation SHEEP_LOCATION(TN_MOB_SHEEP);

SheepRenderer::SheepRenderer(EntityRenderDispatcher* dispatcher, Model*, Model*, float)
    : MobRenderer(dispatcher, new SheepModel(), 0.7F)
{
    mFurModel = new SheepFurModel();
    mOriginalFurModel = mFurModel;
    addLayer(new SheepFurLayer(this));
}

Model* SheepRenderer::getModel(boost::shared_ptr<LivingEntity>& entity)
{
    if (mFurModel->hasOverriddenParts())
    {
        boost::shared_ptr<Sheep> sheep = boost::static_pointer_cast<Sheep>(entity);
        if (sheep->isSheared())
            return mModel;
        return mFurModel;
    }
    return mModel;
}

const ResourceLocation* SheepRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SHEEP_LOCATION;
}

void SheepRenderer::setColourState(boost::shared_ptr<Sheep> sheep, float partialTicks)
{
    if (sheep->hasCustomName() && sheep->getCustomName() == L"jeb_")
    {
        int colourIndex = sheep->mTickCount / 25 + sheep->getId();
        int colourCount = static_cast<int>(DyeColor::COLORS.size());
        int firstIndex = colourIndex % colourCount;
        int secondIndex = (colourIndex + 1) % colourCount;
        float blend = (static_cast<float>(sheep->mTickCount % 25) + partialTicks) / 25.0F;
        arrayWithLength<const float> first = Sheep::getColorArray(DyeColor::byBlockData(firstIndex));
        arrayWithLength<const float> second = Sheep::getColorArray(DyeColor::byBlockData(secondIndex));
        GlStateManager::color3f(first.data[0] * (1.0F - blend) + second.data[0] * blend,
                                first.data[1] * (1.0F - blend) + second.data[1] * blend,
                                first.data[2] * (1.0F - blend) + second.data[2] * blend);
    }
    else
    {
        arrayWithLength<const float> colour = Sheep::getColorArray(sheep->getColor());
        GlStateManager::color3f(colour.data[0], colour.data[1], colour.data[2]);
    }
}

void SheepRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                           float partialTick)
{
    if (mFurModel->hasOverriddenParts())
    {
        boost::shared_ptr<Sheep> sheep = boost::static_pointer_cast<Sheep>(entity);
        setColourState(sheep, partialTick);
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

void SheepRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                 bool overlay)
{
    if (!mFurModel->hasOverriddenParts())
    {
        LivingEntityRenderer::renderLayers(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw,
                                           headPitch, scale, overlay);
    }
}

void SheepRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);

    if (mFurModel != mOriginalFurModel && mFurModel != NULL)
    {
        delete mFurModel;
    }

    if (pack->HasCustomModel(L"sheep"))
    {
        mFurModel = new SheepFurModel(*pack->GetCustomModel(L"sheep"));
    }
    else
    {
        mFurModel = mOriginalFurModel;
    }
}

Model* SheepRenderer::createModel(const ModelDefinition& definition)
{
    return new SheepModel(definition);
}

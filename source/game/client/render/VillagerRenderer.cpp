#include "game/client/render/VillagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/VillagerModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/Villager.hpp"

static ResourceLocation VILLAGER_LOCATION(TN_MOB_VILLAGER_VILLAGER);
static ResourceLocation FARMER_LOCATION(TN_MOB_VILLAGER_FARMER);
static ResourceLocation LIBRARIAN_LOCATION(TN_MOB_VILLAGER_LIBRARIAN);
static ResourceLocation PRIEST_LOCATION(TN_MOB_VILLAGER_PRIEST);
static ResourceLocation SMITH_LOCATION(TN_MOB_VILLAGER_SMITH);
static ResourceLocation BUTCHER_LOCATION(TN_MOB_VILLAGER_BUTCHER);

VillagerRenderer::VillagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new VillagerModel(0.0F), 0.5F)
{
    addLayer(new CustomHeadLayer(static_cast<VillagerModel*>(getModel())->mHead));
}

Model* VillagerRenderer::getModel()
{
    return LivingEntityRenderer::getModel();
}

void VillagerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                              float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);

    boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
    if (villager->getTrading() != 0)
    {
        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        item = not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald, 1));
        Item* itemType = item->getItem();
        TextureAtlasSprite* sprite = itemType->getIcon(item);

        mDispatcher = Minecraft::GetInstance()->getEntityRenderDispatcher();

        const float height = entity->mBbHeight + 0.5F;
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x, (float)y + height, (float)z);
        GlStateManager::normal3f(0.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
        const float pitchDirection = mDispatcher->mOptions->mThirdPersonView == 2 ? -1.0F : 1.0F;
        GlStateManager::rotatef(-(pitchDirection * mDispatcher->mCameraXRot), 1.0F, 0.0F, 0.0F);
        GlStateManager::scalef(0.5F, 0.5F, 0.5F);
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);

        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        bindTexture(&ITEM_LOCATION);
        const float u0 = (float)sprite->getU0(false);
        const float u1 = (float)sprite->getU1(false);
        const float v0 = (float)sprite->getV0(false);
        const float v1 = (float)sprite->getV1(false);

        builder->begin();
        builder->color(0xFFFFFF, 0x80);
        builder->normal(0.0F, 1.0F, 0.0F);
        builder->vertexUV(-0.5F, -0.25F, 0.0F, u0, v1);
        builder->vertexUV(0.5F, -0.25F, 0.0F, u1, v1);
        builder->vertexUV(0.5F, 0.75F, 0.0F, u1, v0);
        builder->vertexUV(-0.5F, 0.75F, 0.0F, u0, v0);
        builder->end();

        GlStateManager::disableBlend();
        GlStateManager::popMatrix();
    }
}

const ResourceLocation* VillagerRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
    switch (villager->getProfession())
    {
    case 0:
        return &FARMER_LOCATION;
    case 1:
        return &LIBRARIAN_LOCATION;
    case 2:
        return &PRIEST_LOCATION;
    case 3:
        return &SMITH_LOCATION;
    case 4:
        return &BUTCHER_LOCATION;
    }
    return &VILLAGER_LOCATION;
}

void VillagerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float)
{
    boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
    float scale = 0.9375F;
    if (villager->getAge() < 0)
    {
        scale = (float)(scale * 0.5);
        mShadowRadius = 0.25F;
    }
    else
    {
        mShadowRadius = 0.5F;
    }
    GlStateManager::scalef(scale, scale, scale);
}

Model* VillagerRenderer::createModel(const ModelDefinition& definition)
{
    return new VillagerModel(0.0F, definition);
}

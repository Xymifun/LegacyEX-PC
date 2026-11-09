#include "game/ui/UIControl_EnchantmentBook.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BookModel.hpp"
#include "game/client/render/EnchantTableRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/client/render/Textures.hpp"

UIControl_EnchantmentBook::UIControl_EnchantmentBook()
{
    mBookModel = NULL;
    m_controlType = UIControl::eUIControlType_EnchantmentBook;
    mItemInstance = ItemInstance::EMPTY;
    mBookTick = 0;
    mBookRotation = mBookRotationPrev = mBookOpen = mBookOpenPrev = mBookOpenTarget = mBookOpenSpeed = 0.0F;
}

void UIControl_EnchantmentBook::tickBook()
{
    UIScene_EnchantingMenu* enchantingMenu = dynamic_cast<UIScene_EnchantingMenu*>(m_scene);
    EnchantmentMenu* menu = NULL;
    not_null_ptr<ItemInstance> current;

    if (enchantingMenu != NULL)
    {
        menu = enchantingMenu->getMenu();
        current = menu->getSlot(0)->getItem();
    }
    else
    {
        current = not_null_ptr<ItemInstance>(new ItemInstance(Items::bow, 1, 0));
    }

    if (!ItemInstance::matches(current, mItemInstance, true))
    {
        mItemInstance = current;

        do
        {
            mBookOpenTarget += mRandom.nextInt(4) - mRandom.nextInt(4);
        } while (mBookOpen <= mBookOpenTarget + 1.0F && mBookOpen >= mBookOpenTarget - 1.0F);
    }

    ++mBookTick;
    mBookOpenPrev = mBookOpen;
    mBookRotationPrev = mBookRotation;

    bool shouldBeOpen = false;
    for (int i = 0; i < 3; ++i)
    {
        if (menu == NULL || menu->mCosts[i] != 0)
        {
            shouldBeOpen = true;
        }
    }

    if (shouldBeOpen)
    {
        mBookRotation += 0.2F;
    }
    else
    {
        mBookRotation -= 0.2F;
    }

    if (mBookRotation < 0.0F)
    {
        mBookRotation = 0.0F;
    }
    if (mBookRotation > 1.0F)
    {
        mBookRotation = 1.0F;
    }

    float openSpeed = (mBookOpenTarget - mBookOpen) * 0.4F;
    const float maxSpeed = 0.2F;
    if (openSpeed < -maxSpeed)
    {
        openSpeed = -maxSpeed;
    }
    if (openSpeed > maxSpeed)
    {
        openSpeed = maxSpeed;
    }
    mBookOpenSpeed += (openSpeed - mBookOpenSpeed) * 0.9F;
    mBookOpen = mBookOpen + mBookOpenSpeed;
}

void UIControl_EnchantmentBook::render(fuiRect* rect)
{
    const float partialTicks = 1.0F;

    GlStateManager::pushMatrix();
    float width = rect->xMax - rect->xMin;
    float height = rect->yMax - rect->yMin;
    float widthScale = width / m_width;
    float heightScale = height / m_height;
    GlStateManager::scalef(widthScale, heightScale, 1.0F);
    GlStateManager::translatef(m_width * 0.5F, m_height * 0.5F, 50.0F);
    GlStateManager::scalef(-57.0F / widthScale, 57.0F / widthScale, 360.0F);
    GlStateManager::rotatef(135.0F, 0.0F, 1.0F, 0.0F);
    Lighting::turnOn();
    GlStateManager::rotatef(-135.0F, 0.0F, 1.0F, 0.0F);

    Minecraft* minecraft = Minecraft::GetInstance();
    int texture = minecraft->mTextures->loadTexture(73);
    minecraft->mTextures->bind(texture);
    GlStateManager::rotatef(20.0F, 1.0F, 0.0F, 0.0F);

    const float rotation = mBookRotationPrev + (mBookRotation - mBookRotationPrev) * partialTicks;
    GlStateManager::translatef((1.0F - rotation) * 0.2F, (1.0F - rotation) * 0.1F, (1.0F - rotation) * 0.25F);
    GlStateManager::rotatef(-(1.0F - rotation) * 90.0F - 90.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(180.0F, 1.0F, 0.0F, 0.0F);

    float spread = mBookOpenPrev + (mBookOpen - mBookOpenPrev) * partialTicks + 0.25F;
    float spread2 = mBookOpenPrev + (mBookOpen - mBookOpenPrev) * partialTicks + 0.75F;
    spread = (spread - std::floor(spread)) * 1.6F - 0.3F;
    spread2 = (spread2 - std::floor(spread2)) * 1.6F - 0.3F;
    if (spread < 0.0F)
    {
        spread = 0.0F;
    }
    if (spread2 < 0.0F)
    {
        spread2 = 0.0F;
    }
    if (spread > 1.0F)
    {
        spread = 1.0F;
    }
    if (spread2 > 1.0F)
    {
        spread2 = 1.0F;
    }

    GlStateManager::enableCull();
    if (mBookModel == NULL)
    {
        EnchantTableRenderer* tableRenderer = static_cast<EnchantTableRenderer*>(
            BlockEntityRenderDispatcher::instance->getRenderer(BlockEntityType::ENCHANTMENT_TABLE_ENTITY));
        if (tableRenderer != NULL)
            mBookModel = tableRenderer->mBookModel;
        else
            mBookModel = new BookModel();
    }
    boost::shared_ptr<Entity> entity;
    mBookModel->render(entity, 0.0F, spread, spread2, rotation, 0.0F, 0.0625F, true);
    GlStateManager::disableCull();
    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
    tickBook();
}

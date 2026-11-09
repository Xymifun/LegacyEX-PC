#include "game/ui/UIControl_MinecraftHorse.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/ui/UIScene_HorseInventoryMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"

UIControl_MinecraftHorse::UIControl_MinecraftHorse()
{
    m_controlType = UIControl::eUIControlType_MinecraftHorse;
    Minecraft* minecraft = Minecraft::GetInstance();
    ScreenSizeCalculator sizeCalculator(minecraft->mOptions, minecraft->mFullWidth, minecraft->mFullHeight, -1);
    mScreenWidth = (float)minecraft->mFullWidth;
    mScaledWidth = (float)sizeCalculator.mWidthScale;
    mScreenHeight = (float)minecraft->mFullHeight;
    mScaledHeight = (float)sizeCalculator.mHeightScale;
}

void UIControl_MinecraftHorse::render(fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();
    float width = rect->xMax - rect->xMin;
    float height = rect->yMax - rect->yMin;
    GlStateManager::translatef(width * 0.5f, height - height / 7.5f, 50.0f);

    UIScene_HorseInventoryMenu* scene = static_cast<UIScene_HorseInventoryMenu*>(m_scene);
    boost::shared_ptr<AbstractHorse> horse = scene->m_horse;
    float aspect = mScreenWidth / mScreenHeight;
    float scale = width / aspect * 0.71f;
    GlStateManager::scalef(-scale, scale, scale);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);

    float oldYBodyRot = horse->mYBodyRot;
    float oldYRot = horse->mYRot;
    float oldXRot = horse->mXRot;
    float oldYHeadRot = horse->mYHeadRot;
    float centreY = m_y + m_height * 0.5f;
    float centreX = m_x + m_width * 0.5f;
    float xd = centreX - scene->m_pointerPos.x;
    float yd = centreY - 40.0f - scene->m_pointerPos.y;

    GlStateManager::rotatef(135.0f, 0.0f, 1.0f, 0.0f);
    Lighting::turnOn();
    GlStateManager::rotatef(-135.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-(std::atan(yd / 40.0f) * 20.0f), 1.0f, 0.0f, 0.0f);

    horse->mYBodyRot = std::atan(xd / 40.0f) * 20.0f;
    horse->mYRot = std::atan(xd / 40.0f) * 40.0f;
    horse->mXRot = -(std::atan(yd / 40.0f) * 20.0f);
    horse->mYHeadRot = horse->mYRot;
    GlStateManager::translatef(0.0f, 0.0f, 0.0f);

    Minecraft::GetInstance()->getEntityRenderDispatcher()->mCameraYRot = 180.0f;
    bool oldHideGui = minecraft->mOptions->mHideGui;
    minecraft->mOptions->mHideGui = true;
    boost::shared_ptr<Entity> entity = horse;
    Minecraft::GetInstance()->getEntityRenderDispatcher()->render(entity, 0.0, 0.0, 0.0, 0.0f, 1.0f, false, false,
                                                                  false, false);
    minecraft->mOptions->mHideGui = oldHideGui;

    horse->mYBodyRot = oldYBodyRot;
    horse->mYRot = oldYRot;
    horse->mXRot = oldXRot;
    horse->mYHeadRot = oldYHeadRot;

    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}

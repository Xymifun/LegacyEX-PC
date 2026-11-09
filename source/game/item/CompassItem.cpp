#include "game/item/CompassItem.hpp"

#include <cmath>
#include <cstring>

#include <boost/shared_ptr.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Math.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

static const std::wstring COMPASS_ICON_NAMES[4] = {std::wstring(L"compassP0"), std::wstring(L"compassP1"),
                                                   std::wstring(L"compassP2"), std::wstring(L"compassP3")};

CompassItem::CompassItem() : Item()
{
    memset(mIcons, 0, sizeof(mIcons));

    addProperty(ResourceLocation(L"angle"), new CompassItem_ItemPropertyFunction());
}

TextureAtlasSprite* CompassItem::getIcon(int auxValue)
{
    TextureAtlasSprite* icon = Item::getIcon(auxValue);
    Minecraft* minecraft = Minecraft::GetInstance();

    if (minecraft->mPlayer && auxValue == 0)
    {
        icon = mIcons[static_cast<LocalPlayer*>(minecraft->mPlayer.get())->mUserIndex];
    }

    return icon;
}

void CompassItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    for (int i = 0; i < 4; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(COMPASS_ICON_NAMES[i]);
    }
}

double CompassItem::CompassItem_ItemPropertyFunction::getFrameRotation(boost::shared_ptr<ItemFrame> frame)
{
    return (double)Mth::wrapDegrees(frame->mDirection->get2DDataValue() * 90 + 180);
}

double CompassItem::CompassItem_ItemPropertyFunction::getSpawnToAngle(Level* level, boost::shared_ptr<Entity> entity)
{
    BlockPos spawn = level->getSharedSpawnPos();

    return std::atan2((double)spawn.z - entity->mZ, (double)spawn.x - entity->mX);
}

double CompassItem::CompassItem_ItemPropertyFunction::wobble(Level* level, double value)
{
    if (level->getGameTime() == mLastUpdate)
    {
        return mRotation;
    }

    mLastUpdate = level->getGameTime();

    double delta = std::fmod(value - mRotation, 6.2831854820251465);
    delta = Mth::clamp(delta, -1.0, 1.0);
    mWobble += delta * 0.1;
    mWobble *= 0.8;
    mRotation += mWobble;

    return mRotation;
}

float CompassItem::CompassItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                          boost::shared_ptr<LivingEntity> entity)
{
    if (entity == NULL && !itemInstance->isFramed())
    {
        return 0.0f;
    }

    bool hasEntity = entity != NULL;
    boost::shared_ptr<Entity> source = hasEntity ? boost::shared_ptr<Entity>(entity)
                                                 : boost::dynamic_pointer_cast<Entity>(itemInstance->getFrame());

    if (level == NULL)
    {
        level = source->mLevel;
    }

    double angle;

    if (level->mDimension->isNaturalDimension())
    {
        double rotation = hasEntity ? source->mYRot : getFrameRotation(boost::dynamic_pointer_cast<ItemFrame>(source));

        rotation = std::fmod(rotation, 360.0);
        double spawnAngle = getSpawnToAngle(level, source);
        angle = 3.1415927410125732 - ((rotation - 90.0) * Mth::DEG_TO_RAD - spawnAngle);
    }
    else
    {
        angle = Math::random() * 6.2831854820251465;
    }

    if (hasEntity)
    {
        angle = wobble(level, angle);
    }

    return Mth::positiveModulo((float)(angle / 6.2831854820251465), 1.0f);
}

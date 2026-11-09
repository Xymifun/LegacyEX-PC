#include "game/core/Painting.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/GameRules.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

const int Painting::Motive::MAX_MOTIVE_NAME_LENGTH = 13;

Painting::Motive* paintingMotives[30] = {new Painting::Motive(L"Kebab", 16, 16, 0, 0),
                                         new Painting::Motive(L"Aztec", 16, 16, 16, 0),
                                         new Painting::Motive(L"Alban", 16, 16, 32, 0),
                                         new Painting::Motive(L"Aztec2", 16, 16, 48, 0),
                                         new Painting::Motive(L"Bomb", 16, 16, 64, 0),
                                         new Painting::Motive(L"Plant", 16, 16, 80, 0),
                                         new Painting::Motive(L"Wasteland", 16, 16, 96, 0),
                                         new Painting::Motive(L"Pool", 32, 16, 0, 32),
                                         new Painting::Motive(L"Courbet", 32, 16, 32, 32),
                                         new Painting::Motive(L"Sea", 32, 16, 64, 32),
                                         new Painting::Motive(L"Sunset", 32, 16, 96, 32),
                                         new Painting::Motive(L"Creebet", 32, 16, 128, 32),
                                         new Painting::Motive(L"Wanderer", 16, 32, 0, 64),
                                         new Painting::Motive(L"Graham", 16, 32, 16, 64),
                                         new Painting::Motive(L"Match", 32, 32, 0, 128),
                                         new Painting::Motive(L"Bust", 32, 32, 32, 128),
                                         new Painting::Motive(L"Stage", 32, 32, 64, 128),
                                         new Painting::Motive(L"Void", 32, 32, 96, 128),
                                         new Painting::Motive(L"SkullAndRoses", 32, 32, 128, 128),
                                         new Painting::Motive(L"Wither", 32, 32, 160, 128),
                                         new Painting::Motive(L"Fighters", 64, 32, 0, 96),
                                         new Painting::Motive(L"Pointer", 64, 64, 0, 192),
                                         new Painting::Motive(L"Pigscene", 64, 64, 64, 192),
                                         new Painting::Motive(L"BurningSkull", 64, 64, 128, 192),
                                         new Painting::Motive(L"Skeleton", 64, 48, 192, 64),
                                         new Painting::Motive(L"DonkeyKong", 64, 48, 192, 112),
                                         new Painting::Motive(L"1x1E1", 16, 16, 0, 16),
                                         new Painting::Motive(L"1x1E2", 16, 16, 16, 16),
                                         new Painting::Motive(L"1x1E3", 16, 16, 32, 16),
                                         new Painting::Motive(L"1x1E4", 16, 16, 48, 16)};

Painting::Motive::Motive(std::wstring name, int width, int height, int textureX, int textureY)
    : name(name),
      width(width),
      height(height),
      textureX(textureX),
      textureY(textureY)
{
}

Painting::Painting(Level* level) : HangingEntity(level)
{
    Entity::fjDerivedCtorCalls();
    _init(level);
}

Painting::Painting(Level* level, const BlockPos& pos, const Direction* direction) : HangingEntity(level, pos)
{
    Entity::fjDerivedCtorCalls();
    _init(level);
}

Painting::Painting(Level* level, const BlockPos& pos, const Direction* direction, const std::wstring& motiveName)
    : HangingEntity(level, pos)
{
    Entity::fjDerivedCtorCalls();
    _init(level);

    for (int i = 0; i < 30; ++i)
    {
        if (paintingMotives[i]->name == motiveName)
        {
            motive = paintingMotives[i];
            setDirection(direction);
            return;
        }
    }
    setDirection(direction);
}

Entity* Painting::create(Level* level)
{
    return new Painting(level);
}

void Painting::_init(Level* level)
{
    motive = NULL;
}

void Painting::PaintingPostConstructor(const Direction* direction, int unused)
{
    std::vector<Motive*> validMotives;
    int largestArea = 0;

    for (int i = 0; i < 30; ++i)
    {
        motive = paintingMotives[i];
        setDirection(direction);
        if (survives())
        {
            validMotives.push_back(motive);
            int area = motive->width * motive->height;
            if (area > largestArea)
                largestArea = area;
        }
    }

    if (!validMotives.empty())
    {
        for (std::vector<Motive*>::iterator it = validMotives.begin(); it != validMotives.end();)
        {
            if ((*it)->width * (*it)->height < largestArea)
                it = validMotives.erase(it);
            else
                ++it;
        }
        motive = validMotives.at((unsigned int)mRandom->nextInt((int)validMotives.size()));
    }

    setDirection(direction);
}

void Painting::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putString(L"Motive", motive->name);
    HangingEntity::addAdditonalSaveData(tag);
}

void Painting::readAdditionalSaveData(CompoundTag* tag)
{
    std::wstring motiveName = tag->getString(L"Motive");
    for (int i = 0; i < 30; ++i)
    {
        if (paintingMotives[i]->name == motiveName)
            motive = paintingMotives[i];
    }
    if (motive == NULL)
        motive = paintingMotives[0];
    HangingEntity::readAdditionalSaveData(tag);
}

int Painting::getWidth()
{
    return motive->width;
}

int Painting::getHeight()
{
    return motive->height;
}

void Painting::dropItem(boost::shared_ptr<Entity> breaker)
{
    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
        return;
    playSound(SoundEvents::PAINTING_BREAK, 1.0f, 1.0f);
    if (breaker != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, breaker->GetType()))
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(breaker);
        if (player->mAbilities.mInstabuild)
            return;
    }
    spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::painting)), 0.0f);
}

void Painting::playPlacementSound()
{
    playSound(SoundEvents::PAINTING_PLACE, 1.0f, 1.0f);
}

void Painting::moveTo(double x, double y, double z, float yRot, float xRot)
{
    setPos(x, y, z);
}

void Painting::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    BlockPos pos = mPos.offset(x - mX, y - mY, z - mZ);
    setPos((double)pos.x, (double)pos.y, (double)pos.z);
}

unsigned long long Painting::GetType()
{
    return EntityType::PAINTING;
}

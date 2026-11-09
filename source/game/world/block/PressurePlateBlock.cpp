#include "game/world/block/PressurePlateBlock.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* PressurePlateBlock::POWERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));

PressurePlateBlock::PressurePlateBlock(const std::wstring& name, Material* material, Sensitivity sensitivity)
    : BasePressurePlateBlock(name, material),
      mSensitivity(sensitivity)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<bool> powered(false);
    registerDefaultState(state->setBoxedValue(POWERED, &powered));
}

int PressurePlateBlock::getSignalForState(const BlockState* state)
{
    return state->getValue(POWERED) ? 15 : 0;
}

const BlockState* PressurePlateBlock::setSignalForState(const BlockState* state, int signal)
{
    return state->setValue(POWERED, signal > 0);
}

void PressurePlateBlock::playOnSound(Level* level, const BlockPos& pos)
{
    if (mMaterial == Material::WOOD)
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::WOOD_PRESSUREPLATE_CLICK_ON,
                         SoundSource::eSoundSource_Blocks, 0.3f, 0.8f, 16.0f);
    else
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::STONE_PRESSUREPLATE_CLICK_ON,
                         SoundSource::eSoundSource_Blocks, 0.3f, 0.6f, 16.0f);
}

void PressurePlateBlock::playOffSound(Level* level, const BlockPos& pos)
{
    if (mMaterial == Material::WOOD)
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::WOOD_PRESSUREPLATE_CLICK_OFF,
                         SoundSource::eSoundSource_Blocks, 0.3f, 0.7f, 16.0f);
    else
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::STONE_PRESSUREPLATE_CLICK_OFF,
                         SoundSource::eSoundSource_Blocks, 0.3f, 0.5f, 16.0f);
}

int PressurePlateBlock::getSignalStrength(Level* level, const BlockPos& pos)
{
    std::vector<boost::shared_ptr<Entity> >* entities = NULL;
    const AABB* box = getSensitiveAABB(pos);
    switch (mSensitivity)
    {
    case EVERYTHING:
        entities = level->getEntities(boost::shared_ptr<Entity>(), box, NULL);
        break;
    case MOBS:
        entities = level->getEntitiesOfClass(typeid(LivingEntity), box);
        break;
    }
    if (entities != NULL && !entities->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Entity> entity = *it;
            if (!entity->isIgnoringBlockTriggers())
            {
                if (mSensitivity != EVERYTHING && entities != NULL)
                    delete entities;
                return 15;
            }
        }
    }
    if (mSensitivity != EVERYTHING && entities != NULL)
        delete entities;
    return 0;
}

const BlockState* PressurePlateBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(POWERED, data == 1);
}

int PressurePlateBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(POWERED) ? 1 : 0;
}

BlockStateDefinition* PressurePlateBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {POWERED};
    return new BlockStateDefinition(this, properties, 1);
}

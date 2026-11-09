#include "game/world/block/LookAtBlockHint.hpp"

#include <algorithm>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"

LookAtBlockHint::LookAtBlockHint(eTutorial_Hint hint, Tutorial* tutorial, const BlockState** states,
                                 unsigned int stateCount, const Property* const* properties, unsigned int propertyCount,
                                 int descriptionId, int auxValue, bool useState)
    : TutorialHint(hint, tutorial, -1, TutorialHint::HINT_LOOK_AT_BLOCK, true, true, -1)
{
    mStateCount = stateCount;
    mStates = new const BlockState*[mStateCount];
    mStateIds = new Block*[mStateCount];
    for (unsigned int i = 0; i < mStateCount; ++i)
    {
        mStates[i] = states[i];
        mStateIds[i] = states[i]->getBlock();
    }
    mPropertyCount = propertyCount;
    mPropertyValues = new const Property*[mPropertyCount];
    for (unsigned int i = 0; i < mPropertyCount; ++i)
    {
        mPropertyValues[i] = properties[i];
    }
    mDescriptionId = descriptionId;
    mAuxValue = auxValue;
    mUseState = useState;
}

LookAtBlockHint::~LookAtBlockHint()
{
    delete[] mStates;
    delete[] mStateIds;
    delete[] mPropertyValues;
}

unsigned int LookAtBlockHint::onLookAt(const BlockState* state)
{
    Block* block = state->getBlock();
    if (block != NULL)
    {
        const BlockState* matchingState = NULL;
        Block* matchedBlock = NULL;
        for (unsigned int i = 0; i < mStateCount; ++i)
        {
            if (block == mStateIds[i])
            {
                matchedBlock = mStateIds[i];
                if (mPropertyCount > 0)
                {
                    for (unsigned int j = 0; j < mPropertyCount; ++j)
                    {
                        const Property* property = mPropertyValues[j];
                        if (std::find(mStates[i]->getProperties().begin(), mStates[i]->getProperties().end(), property)
                                != mStates[i]->getProperties().end()
                            && std::find(state->getProperties().begin(), state->getProperties().end(), property)
                                   != state->getProperties().end()
                            && mStates[i]->getBoxedValue(property) == state->getBoxedValue(property))
                        {
                            matchingState = mStates[i];
                            break;
                        }
                    }
                    if (matchingState != NULL)
                    {
                        break;
                    }
                }
                else
                {
                    matchingState = mStates[i];
                    break;
                }
            }
        }
        if (matchedBlock != NULL && matchingState != NULL)
        {
            if (!mUseState)
            {
                matchingState = state;
            }

            Item* item = Item::byBlock(matchedBlock) != NULL ? Item::byBlock(matchedBlock)
                                                             : matchedBlock->getResource(matchingState, NULL, 0);
            Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
            details->isItem = true;
            if (mDescriptionId >= 0)
            {
                details->itemId = mDescriptionId;
            }
            else if (mDescriptionId == -2)
            {
                details->itemId = -1;
            }
            else if (item != NULL)
            {
                details->itemId = Item::getId(item);
            }
            else
            {
                details->itemId = Block::getId(matchedBlock);
            }
            int auxValue = mAuxValue;
            if (auxValue < 0)
            {
                auxValue = matchedBlock->convertBlockStateToLegacyData(matchingState);
            }
            details->itemAuxValue = auxValue;
            if (item != NULL)
            {
                not_null_ptr<ItemInstance> itemInstance(new ItemInstance(item, 1, auxValue));
                if (item == Items::banner)
                {
                    BannerBlockEntity::addTags(itemInstance, 0xf, NULL);
                }
                if (mDescriptionId < 0)
                {
                    details->itemInstance = itemInstance;
                }
                details->titleId = itemInstance->getUseDescriptionId();
                details->itemDescriptionId = itemInstance->getDescriptionId(-1);
            }
            else
            {
                details->titleId = matchedBlock->getUseDescriptionId(-1);
                details->itemDescriptionId = matchedBlock->getDescriptionId(auxValue);
            }
            return tutorial->setMessage(this, details);
        }
    }
    return 0;
}

unsigned int LookAtBlockHint::onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block* block;
    bool found;
    unsigned int i;

    block = state->getBlock();
    if (block == NULL || block != Blocks::skull)
    {
        return 0;
    }
    found = false;
    for (i = 0; i < mStateCount; ++i)
    {
        if (mStateIds[i] == block)
        {
            found = true;
        }
    }
    if (!found)
    {
        return 0;
    }
    not_null_ptr<ItemInstance> itemInstance = block->getCloneItemInstance(level, pos, state);
    if (itemInstance->getAuxValue() == -1)
    {
        return 0;
    }
    Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails;
    details->titleId = itemInstance->getUseDescriptionId();
    if (itemInstance->getItem() != NULL)
    {
        details->itemDescription = itemInstance->getHoverName();
    }
    details->itemDescriptionId = itemInstance->getDescriptionId(-1);
    details->itemInstance = itemInstance;
    details->itemId = Item::VALIDgetId(itemInstance->getItem());
    details->itemAuxValue = itemInstance->getAuxValue();
    details->allowFade = true;
    details->force = true;
    return tutorial->setMessage(this, details);
}

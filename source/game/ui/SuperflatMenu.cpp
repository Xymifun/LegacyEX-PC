#include "game/ui/SuperflatMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/ui/UIControl_MultiList.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include <algorithm>
#include <cstdio>
#include <cwchar>
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/BannerBlock.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/core/DefaultPresets.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/item/Item.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/core/StringFormat.hpp"

namespace
{

const int OCEAN_MONUMENT_MIN_SEPARATION = 5;
const int SHIPWRECK_MIN_SEPARATION = 5;
const int WOODLAND_MANSION_MIN_SEPARATION = 6;

}

Block** getBlackList();
Block** getWhiteList();
Block** getPlacementRestrictedList();

SuperflatMenu::SliderLabelBehaviour::SliderLabelBehaviour(int name, unsigned int minimum, unsigned int maximum)
    : mMinimum(minimum),
      mMaximum(maximum),
      mName(name)
{
}

SuperflatMenu::CategoricalLabelBehaviour::CategoricalLabelBehaviour(int name, int* labels)
    : SliderLabelBehaviour(name, 0, getLength(labels) - 1),
      mLabels(labels)
{
}

int SuperflatMenu::CategoricalLabelBehaviour::getLength(int* labels)
{
    int length = 0;
    if (*labels < 0)
        return length;
    do
    {
        ++labels;
        ++length;
    } while (*labels >= 0);
    return length;
}

std::wstring SuperflatMenu::CategoricalLabelBehaviour::GetLabel(int value)
{
    std::wstring result;
    result = L10N::GetString(mName);
    result.append(labelSeparator);
    result.append(L10N::GetString(mLabels[value]));
    return result;
}

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name, unsigned int minimum, unsigned int maximum)
    : SliderLabelBehaviour(name, minimum, maximum),
      mType(eType_Integer),
      mScale(1.0),
      mFormat(L"%i")
{
}

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name, const std::wstring& format, int multiplier,
                                                                unsigned int minimum, unsigned int maximum)
    : SliderLabelBehaviour(name, minimum, maximum),
      mType(eType_Integer),
      mScale(multiplier),
      mFormat(format)
{
}

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name, const std::wstring& format, double scale,
                                                                unsigned int minimum, unsigned int maximum)
    : SliderLabelBehaviour(name, minimum, maximum),
      mType(eType_Decimal),
      mScale(scale),
      mFormat(format)
{
}

std::wstring SuperflatMenu::NumericalLabelBehaviour::GetLabel(int value)
{
    std::wstring result(L"Unrecognised label type");
    if (mType == eType_Integer)
        result = formatwstr(mFormat.c_str(), static_cast<int>(static_cast<unsigned int>(value) * mScale));
    else if (mType == eType_Decimal)
        result = formatwstr(mFormat.c_str(), static_cast<unsigned int>(value) * mScale);
    return std::wstring(L10N::GetString(mName)) + labelSeparator + result;
}

SuperflatMenu::Param::Param(bool terminal, bool parent, int name, int desc, int minimum, int maximum,
                            MappingFactory* factory, SliderLabelBehaviour* behaviour)
    : mTerminal(terminal),
      mParent(parent),
      mName(name),
      mDesc(desc),
      mMinValue(minimum),
      mMaxValue(maximum),
      mFactory(factory),
      mBehaviour(behaviour)
{
}

std::wstring SuperflatMenu::Param::getLabel(int value) const
{
    return mBehaviour->GetLabel(value);
}

int SuperflatMenu::Param::getName() const
{
    return mName;
}

bool SuperflatMenu::Param::isTerminal() const
{
    return mTerminal;
}

bool SuperflatMenu::Param::isParent() const
{
    return mParent;
}

bool SuperflatMenu::Param::isChild() const
{
    return !mParent;
}

int SuperflatMenu::Param::getDesc() const
{
    return mDesc;
}

int SuperflatMenu::Param::getMinValue() const
{
    return mMinValue;
}

int SuperflatMenu::Param::getMaxValue() const
{
    return mMaxValue;
}

SuperflatMenu::Param::MappingFactory SuperflatMenu::Param::getFunc() const
{
    return *mFactory;
}

SuperflatMenu::Param SuperflatMenu::Param::Parent(MappingFactory* factory, int name, int desc)
{
    return Param(false, true, name, desc, 0, 0, factory, NULL);
}

SuperflatMenu::Param SuperflatMenu::Param::Child(SliderLabelBehaviour* behaviour, int name)
{
    return Param(false, false, behaviour->mName, name, behaviour->mMinimum, behaviour->mMaximum, NULL, behaviour);
}

SuperflatMenu::Param SuperflatMenu::Param::Terminal()
{
    return Param(true, false, 0, 0, 0, 0, NULL, NULL);
}

SuperflatMenu::Mapping::Mapping(int control, UIControl_MultiList* list) : mControl(control), mList(list), mChildren()
{
}

bool SuperflatMenu::Mapping::isChecked()
{
    return mList->GetCheckboxValue(mControl);
}
void SuperflatMenu::Mapping::updateChildren()
{
    bool enabled = isChecked();
    for (std::vector<Sub*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        (*it)->setEnabled(enabled);
        (*it)->update();
    }
}
void SuperflatMenu::Mapping::setChecked(bool checked)
{
    mList->SetCheckboxValue(mControl, checked, true);
}

void SuperflatMenu::Mapping::writeConfig(FlatGeneratorInfo& info)
{
    write(info);
    updateChildren();
}

void SuperflatMenu::Mapping::readConfig(FlatGeneratorInfo& info)
{
    read(info);
    updateChildren();
}

SuperflatMenu::Mapping::Sub::Sub(Mapping* mapping)
{
    mIndex = mapping->mControl + static_cast<int>(mapping->mChildren.size()) + 1;
    mMapping = mapping;
    mapping->mChildren.push_back(this);
}

SuperflatMenu::Mapping::Sub::~Sub()
{
}

void SuperflatMenu::Mapping::Sub::setEnabled(bool enabled)
{
    mEnabled = enabled;
    mMapping->mList->EnableItem(mIndex, enabled, true);
}

void SuperflatMenu::Mapping::Sub::update()
{
    UIString label;
    if (!mEnabled)
        label = UIString(params[mIndex].getName());
    else
        label = UIString(params[mIndex].getLabel(mPosition));
    mMapping->mList->SetItemLabel(mIndex, label, true);
}

void SuperflatMenu::Mapping::Sub::setPosition(int position)
{
    mPosition = position;
    mMapping->mList->SetSliderValue(mIndex, position, true);
}

int SuperflatMenu::Mapping::Sub::getPosition()
{
    mPosition = mMapping->mList->GetSliderValue(mIndex);
    return mPosition;
}

SuperflatMenu::MapCanyons::MapCanyons(int control, UIControl_MultiList* list) : Mapping(control, list)
{
}
void SuperflatMenu::MapCanyons::write(FlatGeneratorInfo& info)
{
    info.setCanyons(isChecked());
}
void SuperflatMenu::MapCanyons::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasCanyons());
}

SuperflatMenu::MapCaves::MapCaves(int control, UIControl_MultiList* list) : Mapping(control, list)
{
}
void SuperflatMenu::MapCaves::write(FlatGeneratorInfo& info)
{
    info.setCaves(isChecked());
}
void SuperflatMenu::MapCaves::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasCaves());
}

SuperflatMenu::MapDecorations::MapDecorations(int control, UIControl_MultiList* list) : Mapping(control, list)
{
}
void SuperflatMenu::MapDecorations::write(FlatGeneratorInfo& info)
{
    info.setDecorations(isChecked());
}
void SuperflatMenu::MapDecorations::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasDecorations());
}

SuperflatMenu::MapDungeon::MapDungeon(int control, UIControl_MultiList* list) : Mapping(control, list)
{
}
void SuperflatMenu::MapDungeon::write(FlatGeneratorInfo& info)
{
    info.setDungeons(isChecked());
}
void SuperflatMenu::MapDungeon::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasDungeons());
}

SuperflatMenu::MapLake::MapLake(int control, UIControl_MultiList* list) : Mapping(control, list)
{
}
void SuperflatMenu::MapLake::write(FlatGeneratorInfo& info)
{
    info.setLakes(isChecked());
}
void SuperflatMenu::MapLake::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasLakes());
}

SuperflatMenu::MapLavaLake::MapLavaLake(int control, UIControl_MultiList* list) : Mapping(control, list)
{
}
void SuperflatMenu::MapLavaLake::write(FlatGeneratorInfo& info)
{
    info.setLavaLakes(isChecked());
}
void SuperflatMenu::MapLavaLake::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasLavaLakes());
}

SuperflatMenu::MapBiomeSpecific::MapBiomeSpecific(int control, UIControl_MultiList* list)
    : Mapping(control, list),
      mSub(this)
{
}
void SuperflatMenu::MapBiomeSpecific::write(FlatGeneratorInfo& info)
{
    if (isChecked())
        info.setBiomeSpecific(mSub.getPosition());
    else
        info.setBiomeSpecific(false);
}
void SuperflatMenu::MapBiomeSpecific::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getBiomeSpecificSpacing());
    setChecked(info.hasBiomeSpecific());
}

SuperflatMenu::MapMineshaft::MapMineshaft(int control, UIControl_MultiList* list) : Mapping(control, list), mSub(this)
{
}
void SuperflatMenu::MapMineshaft::write(FlatGeneratorInfo& info)
{
    if (isChecked())
        info.setMineshaft(mSub.getPosition());
    else
        info.setMineshaft(false);
}
void SuperflatMenu::MapMineshaft::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getMineshaftChance());
    setChecked(info.hasMineshaft());
}

SuperflatMenu::MapVillage::MapVillage(int control, UIControl_MultiList* list)
    : Mapping(control, list),
      mSub(this),
      mSub2(this)
{
}
void SuperflatMenu::MapVillage::write(FlatGeneratorInfo& info)
{
    if (isChecked())
        info.setVillage(mSub.getPosition(), mSub2.getPosition());
    else
        info.setVillage(false);
}
void SuperflatMenu::MapVillage::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getVillageSize());
    mSub2.setPosition(info.getVillageDistance());
    setChecked(info.hasVillage());
}

SuperflatMenu::MapStronghold::MapStronghold(int control, UIControl_MultiList* list)
    : Mapping(control, list),
      mSub(this),
      mSub2(this)
{
}
void SuperflatMenu::MapStronghold::write(FlatGeneratorInfo& info)
{
    if (isChecked())
        info.setStronghold(mSub2.getPosition(), mSub.getPosition());
    else
        info.setStronghold(false);
}
void SuperflatMenu::MapStronghold::read(FlatGeneratorInfo& info)
{
    mSub2.setPosition(info.getStrongholdDistance());
    mSub.setPosition(info.getStrongholdCount());
    setChecked(info.hasStronghold());
}

SuperflatMenu::MapOceanMonument::MapOceanMonument(int control, UIControl_MultiList* list)
    : Mapping(control, list),
      mSub(this),
      mSub2(this)
{
}
void SuperflatMenu::MapOceanMonument::write(FlatGeneratorInfo& info)
{
    if (isChecked())
    {
        const int separation = mSub2.getPosition();
        const std::pair<int, int> values = getSpacingAndSeparationFromPercentage(mSub.getPosition(), separation,
                                                                                 OCEAN_MONUMENT_MIN_SEPARATION);
        info.setOceanMonument(values.first, values.second);
    }
    else
    {
        info.setOceanMonument(false);
    }
}
void SuperflatMenu::MapOceanMonument::read(FlatGeneratorInfo& info)
{
    int spacing = info.getOceanMonumentSpacing();
    int separation = info.getOceanMonuemntSeparation();
    mSub.setPosition(spacing);
    mSub2.setPosition(separation * 100 / (spacing * 5));
    setChecked(info.hasOceanMonument());
}

SuperflatMenu::MapShipwrecks::MapShipwrecks(int control, UIControl_MultiList* list)
    : Mapping(control, list),
      mSub(this),
      mSub2(this)
{
}
void SuperflatMenu::MapShipwrecks::write(FlatGeneratorInfo& info)
{
    if (isChecked())
    {
        const int separation = mSub2.getPosition();
        const std::pair<int, int> values = getSpacingAndSeparationFromPercentage(mSub.getPosition(), separation,
                                                                                 SHIPWRECK_MIN_SEPARATION);
        info.setShipwrecks(values.first, values.second);
    }
    else
    {
        info.setShipwrecks(false);
    }
}
void SuperflatMenu::MapShipwrecks::read(FlatGeneratorInfo& info)
{
    int spacing = info.getShipwreckSpacing();
    int separation = info.getShipwreckSeparation();
    mSub.setPosition(spacing);
    mSub2.setPosition(separation * 100 / (spacing * 5));
    setChecked(info.hasShipwrecks());
}

SuperflatMenu::MapWoodlandMansion::MapWoodlandMansion(int control, UIControl_MultiList* list)
    : Mapping(control, list),
      mSub(this),
      mSub2(this)
{
}
void SuperflatMenu::MapWoodlandMansion::write(FlatGeneratorInfo& info)
{
    if (isChecked())
    {
        const int separation = mSub2.getPosition();
        const std::pair<int, int> values = getSpacingAndSeparationFromPercentage(mSub.getPosition(), separation,
                                                                                 WOODLAND_MANSION_MIN_SEPARATION);
        info.setWoodlandMansion(values.first, values.second);
    }
    else
    {
        info.setWoodlandMansion(false);
    }
}
void SuperflatMenu::MapWoodlandMansion::read(FlatGeneratorInfo& info)
{
    int spacing = info.getWoodlandMansionSpacing();
    int separation = info.getWoodlandMansionSeparation();
    mSub.setPosition(spacing);
    mSub2.setPosition(separation * 100 / (spacing * 5));
    setChecked(info.hasWoodlandMansion());
}

std::wstring SuperflatMenu::BiomeInfo::getName() const
{
    return Biome::biomes[mBiome]->getName(false);
}

SuperflatMenu::CustomSuperflatBlocks::CustomSuperflatBlocks()
    : boost::unordered_map<int, not_null_ptr<ItemInstance> >(),
      mMaxId(0)
{
    addItem(0, new ItemInstance(Blocks::air, 1, 0));
    addItem(1, new ItemInstance(Blocks::water, 1, 0));
    addItem(2, new ItemInstance(Blocks::lava, 1, 0));

    int nextId = 3;
    for (int group = 0; group < UIScene_CreativeMenu::ECreative_Inventory_Groups_Count; ++group)
    {
        const std::vector<const UIScene_CreativeMenu::ItemBuilder*>& builders =
            UIScene_CreativeMenu::s_groupItems[group];
        for (std::vector<const UIScene_CreativeMenu::ItemBuilder*>::const_iterator it = builders.begin();
             it != builders.end(); ++it)
        {
            not_null_ptr<ItemInstance> item = (*it)->build();
            if (!item->isEmpty() && Block::byItem(item->getItem()) != NULL && addItem(nextId, item))
                ++nextId;
        }
    }
}

SuperflatMenu::CustomSuperflatBlocks::~CustomSuperflatBlocks()
{
}

bool SuperflatMenu::CustomSuperflatBlocks::inlist(Block** list, Block* block)
{
    for (; *list != NULL; ++list)
    {
        if (*list == block)
            return true;
    }
    return false;
}

int SuperflatMenu::CustomSuperflatBlocks::addItem(int id, not_null_ptr<ItemInstance> itemValue)
{
    const int itemId = Item::getId(itemValue->getItem());
    Block* block = Block::byId(itemId);
    if (!Item::isItemABlock(itemId))
        return 0;
    if (block == NULL)
        return 0;

    const BlockState* state = block->getBlockState(itemValue->getAuxValue());
    const bool listed = !inlist(getBlackList(), block)
                        && (state->isSolidBlockingCubeAndNotSignalSource() || inlist(getWhiteList(), block));
    if (!listed || dynamic_cast<HalfSlabBlock*>(block) != NULL || dynamic_cast<BannerBlock*>(block) != NULL)
        return 0;

    if (dynamic_cast<HugeMushroomBlock*>(block) != NULL)
    {
        HugeMushroomBlock::Variant* variant = state->getValue(HugeMushroomBlock::VARIANT);
        if (variant != HugeMushroomBlock::Variant::ALL_OUTSIDE && variant != HugeMushroomBlock::Variant::STEM)
            return 0;
    }

    if (mMaxId < id)
        mMaxId = id;
    (*this)[id] = itemValue;
    return 1;
}

int SuperflatMenu::CustomSuperflatBlocks::addItem(int id, ItemInstance* itemValue)
{
    return addItem(id, not_null_ptr<ItemInstance>(itemValue));
}

not_null_ptr<ItemInstance> SuperflatMenu::CustomSuperflatBlocks::item(int id) const
{
    const_iterator it = find(id);
    if (it != end())
        return it->second;
    not_null_ptr<ItemInstance> empty = ItemInstance::EMPTY;
    return empty;
}

bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(Block* block)
{
    return inlist(getPlacementRestrictedList(), block);
}

bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(int id)
{
    return hasRestrictedPlacement(Block::byId(id));
}

bool SuperflatMenu::CustomSuperflatBlocks::sanitiseLayer(int* ids, int* data, int layer)
{
    int below = layer - 1;
    if (below < 0)
        return false;
    if (layer + 1 > 255)
        return false;

    int id = ids[layer];
    Block* block = Block::byId(id);
    Block* above = Block::byId(ids[layer + 1]);
    Block* underneath = Block::byId(ids[below]);

    if (block == Blocks::water)
    {
        if (underneath == Blocks::concrete_powder)
        {
            ids[below] = Block::getId(Blocks::concrete);
            return false;
        }
        if (underneath == Blocks::lava)
        {
            ids[below] = Block::getId(Blocks::obsidian);
            data[below] = 0;
            return false;
        }
    }
    if ((block == Blocks::lava && underneath == Blocks::water)
        || (block == Blocks::water && (underneath == Blocks::magma || underneath == Blocks::soul_sand)))
    {
        ids[below] = Block::getId(Blocks::stone);
        data[below] = 0;
        return false;
    }

    if (id != 0 && block->mMaterial->isLiquid()
        && (ids[below] == 0 || underneath == Blocks::carpet || underneath == Blocks::snow_layer))
    {
        ids[below] = ids[layer];
        data[below] = data[layer];
        sanitiseLayer(ids, data, below);
        return true;
    }

    if (block == Blocks::mycelium || block == Blocks::grass)
    {
        if (above->isSolidBlockingCube(above->defaultBlockState()))
        {
            ids[layer] = Block::getId(Blocks::dirt);
            data[layer] = 0;
            return true;
        }
    }

    if (block != NULL && dynamic_cast<FallingBlock*>(block) != NULL && !underneath->mMaterial->blocksMotion())
    {
        if (block == Blocks::sand)
        {
            ids[layer] = Block::getId(Blocks::sandstone);
            data[layer] = 0;
            return true;
        }
        if (block == Blocks::gravel)
        {
            ids[layer] = Block::getId(Blocks::stone);
            data[layer] = 0;
            return true;
        }
        if (block == Blocks::concrete_powder)
        {
            ids[layer] = Block::getId(Blocks::concrete);
            return true;
        }
        ids[layer] = Block::getId(Blocks::cobblestone);
        data[layer] = 0;
        return true;
    }

    if (hasRestrictedPlacement(block))
    {
        if (underneath->isSolidRender(underneath->defaultBlockState()) && underneath->mMaterial->blocksMotion()
            && underneath != Blocks::packed_ice)
            return false;
        ids[layer] = 0;
        data[layer] = 0;
        return true;
    }

    if (block == Blocks::red_mushroom_block || block == Blocks::brown_mushroom_block)
    {
        static int stemData = getMushroomDataValue(HugeMushroomBlock::Variant::STEM);
        static int allStemData = getMushroomDataValue(HugeMushroomBlock::Variant::ALL_STEM);
        static int centerData = getMushroomDataValue(HugeMushroomBlock::Variant::CENTER);
        static int allInsideData = getMushroomDataValue(HugeMushroomBlock::Variant::ALL_INSIDE);
        if (data[layer] == stemData)
            return false;
        if (data[layer] == allStemData)
            return false;
        if (block != above)
            data[layer] = centerData;
        else
            data[layer] = allInsideData;
        return true;
    }

    if (block == Blocks::air)
        return false;
    if (block == Blocks::lava)
        return false;
    if (static_cast<FireBlock*>(Blocks::fire)->getFlameOdds(block) < 1)
        return false;

    Block* nearestBelow = NULL;
    for (int y = below; y >= 0 && y >= layer - 3; --y)
    {
        Block* scanned = Block::byId(ids[y]);
        if (scanned != Blocks::air)
        {
            nearestBelow = scanned;
            break;
        }
    }
    if (nearestBelow != Blocks::lava)
        return false;
    ids[layer] = Block::getId(Blocks::stone);
    data[layer] = 0;
    return true;
}

void SuperflatMenu::CustomSuperflatBlocks::sanitise(FlatGeneratorInfo& info)
{
    int ids[256] = {0};
    int data[256] = {0};
    info.getLayers(ids, data);
    for (int i = 0; i < 256; ++i)
        sanitiseLayer(ids, data, i);

    std::vector<FlatLayerInfo> layers;
    for (int i = 0; i < 256; ++i)
        layers.push_back(FlatLayerInfo(1, ids[i], data[i]));
    info.setLayers(layers);
    info.reduceLayers();

    int biome = info.getBiome();
    if (biome == 0 || biome == 0x18)
    {
        int replacement = 0;
        if (info.hasOceanMonument())
            replacement = 0x18;
        info.setBiome(replacement);
    }
    else if (biome == 0x28 || biome == 0x29)
    {
        int replacement = 0x28;
        if (info.hasOceanMonument())
            replacement = 0x29;
        info.setBiome(replacement);
    }
    else if (biome == 0x2a || biome == 0x2b)
    {
        int replacement = 0x2a;
        if (info.hasOceanMonument())
            replacement = 0x2b;
        info.setBiome(replacement);
    }
    else if (biome == 0x2c || biome == 0x2d)
    {
        int replacement = 0x2c;
        if (info.hasOceanMonument())
            replacement = 0x2d;
        info.setBiome(replacement);
    }
    else if (biome == 0x2e || biome == 0x2f)
    {
        int replacement = 0x2e;
        if (info.hasOceanMonument())
            replacement = 0x2f;
        info.setBiome(replacement);
    }

    unsigned int topSolidLayer = info.findTopSolidLayer();
    if (topSolidLayer < 256)
    {
        if (Block::byId(ids[topSolidLayer])->defaultBlockState()->isTransparent() && info.hasWoodlandMansion())
            info.setWoodlandMansion(false);
    }
}

uint32_t SuperflatMenu::CustomSuperflatBlocks::verify(LaunchMoreOptionsMenuInitData* initData, SuperflatConfig* config)
{
    SuperflatConfig localConfig(*config);
    FlatGeneratorInfo info(&localConfig, false);
    info.reduceLayers();
    std::vector<FlatLayerInfo> layers;
    info.getLayers(layers);
    const int biome = info.getBiome();

    if (info.hasVillage())
    {
        switch (biome)
        {
        case 1:
        case 2:
        case 5:
        case 0xc:
        case 0x1e:
        case 0x23:
            break;
        default:
            return 0xa8872c87;
        }
    }

    if (info.hasLakes() || info.hasLavaLakes())
    {
        int top = std::max(info.getLayer(info.findTopNonAirLayer()).getEnd(), 1);
        int bottom = std::max(info.getLayer(info.findTopSolidLayer()).getEnd() - 4, 1);
        for (int height = top; height >= bottom; --height)
        {
            int index = info.getLayerIndexAtHeight(height);
            if (index < 0)
                return 0x6e694430;
            Block* block = Block::byId(info.getLayer(index).getId());
            if (block == NULL)
                return 0x6e694430;
            if (height < 4)
            {
                if (!block->mMaterial->isSolid())
                    return 0x6e694430;
            }
            else if (block->mMaterial->isLiquid())
            {
                return 0x6e694430;
            }
        }
    }

    for (std::vector<FlatLayerInfo>::const_iterator it = layers.begin() + 1; it != layers.end(); ++it)
    {
        Block* block = Block::byId(it->getId());
        int aboveId = -1;
        Block* above = NULL;
        if (it + 1 != layers.end())
        {
            aboveId = (it + 1)->getId();
            above = Block::byId(aboveId);
        }
        bool supportsLayers = block->defaultBlockState()->isSolidRender() && block->mMaterial->blocksMotion()
                              && block != Blocks::packed_ice;
        if (!supportsLayers
            && ((static_cast<int>(it->getHeight()) > 1 && hasRestrictedPlacement(block))
                || (aboveId != -1 && hasRestrictedPlacement(above))))
            return 0x16c16d5e;
        if (!block->mMaterial->blocksMotion() && above != NULL && dynamic_cast<FallingBlock*>(above) != NULL)
            return 0xf2c82a8d;
        if (block->defaultBlockState()->isTransparent() && info.hasWoodlandMansion() && it + 1 == layers.end())
            return 0x1b808acf;
    }

    if (info.hasBiomeSpecific() && biome != 2 && biome != 0x15 && biome != 6 && biome != 0x1e && biome != 0xc
        && biome != 0 && biome != 0x18 && biome != 0x2e && biome != 0x2f && biome != 0x2c && biome != 0x2d
        && biome != 0x2a && biome != 0x2b && biome != 0x28 && biome != 0x29)
        return 0xd862b5d5;
    if (info.hasOceanMonument() && biome != 0 && biome != 0x18 && biome != 0x2e && biome != 0x2f && biome != 0x2c
        && biome != 0x2d && biome != 0x2a && biome != 0x2b && biome != 0x28 && biome != 0x29)
        return 0xc2e51385;
    if (info.hasShipwrecks() && biome != 0 && biome != 0x18 && biome != 0x2e && biome != 0x2f && biome != 0x2c
        && biome != 0x2d && biome != 0x2a && biome != 0x2b && biome != 0x28 && biome != 0x29 && biome != 0x10
        && biome != 0x1a && biome != 0xf)
        return 0x37917102;
    if (info.hasWoodlandMansion() && biome != 0x1d)
        return 0xdae45b4d;

    if (!initData->mStructures
        && (info.hasVillage() || info.hasMineshaft() || info.hasStronghold() || info.hasBiomeSpecific()
            || info.hasOceanMonument() || info.hasDungeons() || info.hasDecorations()))
        return 0x258655f1;
    if ((info.hasCaves() && info.getHeight() < 2) || (info.hasCanyons() && info.getHeight() < 4))
        return 0x258655f1;
    return 0;
}

SuperflatConfig SuperflatMenu::PresetCache::sPresets[4];
bool SuperflatMenu::PresetCache::sPresetStored[4] = {false, false, false, false};

void SuperflatMenu::PresetCache::GetCachedPreset(int index, SuperflatConfig& config)
{
    if (!sPresetStored[index])
    {
        sPresets[index].makePreset(DefaultPresets::CLASSIC);
        sPresetStored[index] = true;
    }
    config = sPresets[index];
}

void SuperflatMenu::PresetCache::SetCachedPreset(int index, const SuperflatConfig& config)
{
    sPresetStored[index] = true;
    sPresets[index] = config;
}

SuperflatMenu::BiomeInfo* SuperflatMenu::getBiomeInfoA()
{
    static bool initialized = false;
    static BiomeInfo items[22] = {{1, static_cast<int>(0xe18c04c8u), not_null_ptr<ItemInstance>()},
                                  {35, static_cast<int>(0x70594c1du), not_null_ptr<ItemInstance>()},
                                  {37, static_cast<int>(0x512d43dbu), not_null_ptr<ItemInstance>()},
                                  {2, static_cast<int>(0xf440c270u), not_null_ptr<ItemInstance>()},
                                  {4, static_cast<int>(0xe2e84bd8u), not_null_ptr<ItemInstance>()},
                                  {29, static_cast<int>(0xa5e5b0f2u), not_null_ptr<ItemInstance>()},
                                  {21, static_cast<int>(0xcdc09066u), not_null_ptr<ItemInstance>()},
                                  {6, static_cast<int>(0xdccb0c5eu), not_null_ptr<ItemInstance>()},
                                  {3, static_cast<int>(0xb5196ce2u), not_null_ptr<ItemInstance>()},
                                  {0, static_cast<int>(IDS_BIOME_DESC_OCEAN), not_null_ptr<ItemInstance>()},
                                  {40, static_cast<int>(IDS_BIOME_DESC_WARM_OCEAN), not_null_ptr<ItemInstance>()},
                                  {42, static_cast<int>(0x5ecebfe6u), not_null_ptr<ItemInstance>()},
                                  {44, static_cast<int>(IDS_BIOME_DESC_COLD_OCEAN), not_null_ptr<ItemInstance>()},
                                  {46, static_cast<int>(0x84809552u), not_null_ptr<ItemInstance>()},
                                  {12, static_cast<int>(0xc36e4255u), not_null_ptr<ItemInstance>()},
                                  {30, static_cast<int>(0x54e18948u), not_null_ptr<ItemInstance>()},
                                  {5, static_cast<int>(0x970aa853u), not_null_ptr<ItemInstance>()},
                                  {32, static_cast<int>(0xb0f3c474u), not_null_ptr<ItemInstance>()},
                                  {14, static_cast<int>(0x64137d27u), not_null_ptr<ItemInstance>()},
                                  {8, static_cast<int>(0x36d871a9u), not_null_ptr<ItemInstance>()},
                                  {9, static_cast<int>(0x938424deu), not_null_ptr<ItemInstance>()},
                                  {-1, -1, not_null_ptr<ItemInstance>()}};
    if (!initialized)
    {
        initialized = true;
        items[0].mItem = createItem(Blocks::grass);
        items[1].mItem = createItem(Blocks::tallgrass);
        TypedBoxed<SandBlock::Variant*> sandValue(SandBlock::Variant::RED_SAND);
        items[2].mItem = not_null_ptr<ItemInstance>(createItem(Blocks::sand->defaultBlockState()->setBoxedValue(
            static_cast<const Property*>(SandBlock::TYPE), &sandValue)));
        items[3].mItem = createItem(Blocks::sand);
        items[4].mItem = createItem(Blocks::log);
        TypedBoxed<PlanksBlock::Variant*> logValue(PlanksBlock::Variant::DARK_OAK);
        items[5].mItem = not_null_ptr<ItemInstance>(createItem(Blocks::log2->defaultBlockState()->setBoxedValue(
            static_cast<const Property*>(NewLogBlock::VARIANT), &logValue)));
        items[6].mItem = createItem(Blocks::cocoa);
        items[7].mItem = createItem(Blocks::waterlily);
        items[8].mItem = createItem(Blocks::stone);
        items[9].mItem = createItem(Blocks::water);
        items[10].mItem = createItem(Blocks::water);
        items[11].mItem = createItem(Blocks::water);
        items[12].mItem = createItem(Blocks::water);
        items[13].mItem = createItem(Blocks::water);
        items[14].mItem = createItem(Blocks::ice);
        items[15].mItem = createItem(Blocks::snow_layer);
        TypedBoxed<PlanksBlock::Variant*> leavesValue(PlanksBlock::Variant::SPRUCE);
        items[16].mItem = not_null_ptr<ItemInstance>(createItem(Blocks::leaves->defaultBlockState()->setBoxedValue(
            static_cast<const Property*>(OldLeafBlock::VARIANT), &leavesValue)));
        items[17].mItem = createItem(Blocks::log2);
        items[18].mItem = createItem(Blocks::red_mushroom);
        items[19].mItem = createItem(Blocks::netherrack);
        items[20].mItem = createItem(Blocks::end_stone);
    }
    return items;
}

template <class T>
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make(int control, UIControl_MultiList* list)
{
    return new T(control, list);
}

template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapVillage>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapCaves>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapCanyons>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapMineshaft>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapStronghold>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapBiomeSpecific>(int,
                                                                                               UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapDungeon>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapDecorations>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapLake>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapLavaLake>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapOceanMonument>(int,
                                                                                               UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapShipwrecks>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapWoodlandMansion>(int,
                                                                                                 UIControl_MultiList*);

namespace
{

SuperflatMenu::Param::MappingFactory makeVillage = &SuperflatMenu::Mapping::make<SuperflatMenu::MapVillage>;
SuperflatMenu::Param::MappingFactory makeCaves = &SuperflatMenu::Mapping::make<SuperflatMenu::MapCaves>;
SuperflatMenu::Param::MappingFactory makeCanyons = &SuperflatMenu::Mapping::make<SuperflatMenu::MapCanyons>;
SuperflatMenu::Param::MappingFactory makeMineshaft = &SuperflatMenu::Mapping::make<SuperflatMenu::MapMineshaft>;
SuperflatMenu::Param::MappingFactory makeStronghold = &SuperflatMenu::Mapping::make<SuperflatMenu::MapStronghold>;
SuperflatMenu::Param::MappingFactory makeBiomeSpecific = &SuperflatMenu::Mapping::make<SuperflatMenu::MapBiomeSpecific>;
SuperflatMenu::Param::MappingFactory makeDungeon = &SuperflatMenu::Mapping::make<SuperflatMenu::MapDungeon>;
SuperflatMenu::Param::MappingFactory makeDecorations = &SuperflatMenu::Mapping::make<SuperflatMenu::MapDecorations>;
SuperflatMenu::Param::MappingFactory makeLake = &SuperflatMenu::Mapping::make<SuperflatMenu::MapLake>;
SuperflatMenu::Param::MappingFactory makeLavaLake = &SuperflatMenu::Mapping::make<SuperflatMenu::MapLavaLake>;
SuperflatMenu::Param::MappingFactory makeOceanMonument = &SuperflatMenu::Mapping::make<SuperflatMenu::MapOceanMonument>;
SuperflatMenu::Param::MappingFactory makeShipwrecks = &SuperflatMenu::Mapping::make<SuperflatMenu::MapShipwrecks>;
SuperflatMenu::Param::MappingFactory makeWoodlandMansion =
    &SuperflatMenu::Mapping::make<SuperflatMenu::MapWoodlandMansion>;

int villageSizeLabels[] = {IDS_SMALL, IDS_LARGE, -1};

}

SuperflatMenu::Param SuperflatMenu::params[SuperflatMenu::PARAM_COUNT] = {
    Param::Parent(&makeVillage, IDS_PARAM_VILLAGE, IDS_PARAM_DESC_VILLAGE),
    Param::Child(new CategoricalLabelBehaviour(IDS_PARAM_VILLAGE_SIZE, villageSizeLabels), IDS_PARAM_DESC_VILLAGE_SIZE),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_VILLAGE_DISTANCE, L"%im", 16, 16, 100),
                 IDS_PARAM_DESC_VILLAGE_DISTANCE),
    Param::Parent(&makeCaves, IDS_PARAM_CAVES, IDS_PARAM_DESC_CAVES),
    Param::Parent(&makeCanyons, IDS_PARAM_CANYONS, IDS_PARAM_DESC_CANYONS),
    Param::Parent(&makeMineshaft, IDS_PARAM_MINESHAFT, IDS_PARAM_DESC_MINESHAFT),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_MINESHAFT_CHANCE, L"%.1f%%", 0.1, 1, 10),
                 IDS_PARAM_DESC_MINESHAFT_CHANCE),
    Param::Parent(&makeStronghold, IDS_PARAM_STRONGHOLD, IDS_PARAM_DESC_STRONGHOLD),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_STRONGHOLD_COUNT, 1, 3), IDS_PARAM_DESC_STRONGHOLD_COUNT),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_VILLAGE_DISTANCE, L"%im", 16, 16, 100),
                 IDS_PARAM_DESC_STRONGHOLD_SPACING),
    Param::Parent(&makeBiomeSpecific, IDS_PARAM_BIOMESPECIFIC, IDS_PARAM_DESC_BIOMESPECIFIC),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_VILLAGE_DISTANCE, L"%im", 16, 16, 100),
                 IDS_PARAM_DESC_BIOMESPECIFIC_SPACING),
    Param::Parent(&makeOceanMonument, IDS_PARAM_OCEAN_MONUMENT, IDS_PARAM_DESC_OCEAN_MONUMENT),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_VILLAGE_DISTANCE, L"%im", 16, 32, 100),
                 IDS_PARAM_DESC_OCEAN_MONUMENT_SPACING),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_SEPERATION, L"%i%%", 5, 5, 15),
                 IDS_PARAM_DESC_OCEAN_MONUMENT_SEPERATION),
    Param::Parent(&makeShipwrecks, IDS_PARAM_SHIPWRECK, IDS_PARAM_DESC_SHIPWRECK),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_VILLAGE_DISTANCE, L"%im", 16, 32, 100),
                 IDS_PARAM_DESC_SHIPWRECK_SPACING),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_SEPERATION, L"%i%%", 5, 5, 15),
                 IDS_PARAM_DESC_SHIPWRECK_SEPERATION),
    Param::Parent(&makeWoodlandMansion, IDS_PARAM_WOODLAND_MANSION, IDS_PARAM_DESC_WOODLAND_MANSION),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_VILLAGE_DISTANCE, L"%im", 16, 16, 100),
                 IDS_PARAM_DESC_WOODLAND_MANSION_SPACING),
    Param::Child(new NumericalLabelBehaviour(IDS_PARAM_SEPERATION, L"%i%%", 5, 5, 15),
                 IDS_PARAM_DESC_WOODLAND_MANSION_SEPERATION),
    Param::Parent(&makeDungeon, IDS_PARAM_DUNGEON, IDS_PARAM_DESC_DUNGEON),
    Param::Parent(&makeDecorations, IDS_PARAM_DECORATION, IDS_PARAM_DESC_DECORATION),
    Param::Parent(&makeLake, IDS_PARAM_LAKES, IDS_PARAM_DESC_LAKES),
    Param::Parent(&makeLavaLake, IDS_PARAM_LAVA_LAKES, IDS_PARAM_DESC_LAVA_LAKES),
    Param::Terminal(),
};

std::wstring SuperflatMenu::labelSeparator(L" - ");

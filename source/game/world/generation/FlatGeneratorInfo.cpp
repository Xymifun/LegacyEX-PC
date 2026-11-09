#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/world/generation/LakeFeature.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"
#include "game/world/generation/MineShaftFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/core/StringFormat.hpp"
#include <algorithm>

static const int MAX_SPACING = 100;

std::pair<int, int> getSpacingAndSeparationFromPercentage(int spacing, int separationPercent, int minimumSeparation)
{
    separationPercent *= 5;
    spacing = std::max(std::min(MAX_SPACING, spacing), 16);
    minimumSeparation = std::max(separationPercent * spacing / 100, minimumSeparation);
    return std::make_pair(spacing, minimumSeparation);
}

FlatGeneratorInfo::FlatGeneratorInfo(SuperflatConfig* config)
{
    mOwnsConfig = false;
    mConfig = config;
}

FlatGeneratorInfo::FlatGeneratorInfo(SuperflatConfig* config, bool copy)
{
    if (copy)
    {
        mOwnsConfig = true;
        mOwnedConfig = boost::shared_ptr<SuperflatConfig>(config);
    }
    else
    {
        mOwnsConfig = false;
        mConfig = config;
    }
}

FlatGeneratorInfo::~FlatGeneratorInfo()
{
}

SuperflatConfig* FlatGeneratorInfo::core() const
{
    return mOwnsConfig ? mOwnedConfig.get() : mConfig;
}

bool FlatGeneratorInfo::hasCaves()
{
    return core()->hasParam(0x25);
}
bool FlatGeneratorInfo::hasCanyons()
{
    return core()->hasParam(0x26);
}
bool FlatGeneratorInfo::hasVillage()
{
    return core()->hasParam(0x20);
}
bool FlatGeneratorInfo::hasBiomeSpecific()
{
    return core()->hasParam(0x21);
}
bool FlatGeneratorInfo::hasMineshaft()
{
    return core()->hasParam(0x23);
}
bool FlatGeneratorInfo::hasStronghold()
{
    return core()->hasParam(0x22);
}
bool FlatGeneratorInfo::hasOceanMonument()
{
    return core()->hasParam(0x24);
}
bool FlatGeneratorInfo::hasWoodlandMansion()
{
    return core()->hasParam(0x27);
}
bool FlatGeneratorInfo::hasShipwrecks()
{
    return core()->hasParam(0x28);
}
bool FlatGeneratorInfo::hasDecorations()
{
    return core()->hasParam(0x30);
}
bool FlatGeneratorInfo::hasDungeons()
{
    return core()->hasParam(0x31);
}
bool FlatGeneratorInfo::hasLakes()
{
    return core()->hasParam(0x32);
}
bool FlatGeneratorInfo::hasLavaLakes()
{
    return core()->hasParam(0x33);
}

LargeCaveFeature* FlatGeneratorInfo::getCaves()
{
    LargeCaveFeature* feature;
    if (hasCaves())
        feature = new LargeCaveFeature();
    else
        feature = 0;
    return feature;
}
CanyonFeature* FlatGeneratorInfo::getCanyons()
{
    CanyonFeature* feature;
    if (hasCanyons())
        feature = new CanyonFeature();
    else
        feature = 0;
    return feature;
}
VillageFeature* FlatGeneratorInfo::getVillage(int size)
{
    VillageFeature* feature = 0;
    if (hasVillage())
        feature = new VillageFeature(core(), size);
    return feature;
}
RandomScatteredLargeFeature* FlatGeneratorInfo::getBiomeSpecific()
{
    RandomScatteredLargeFeature* feature = 0;
    if (hasBiomeSpecific())
        feature = new RandomScatteredLargeFeature(core());
    return feature;
}
MineShaftFeature* FlatGeneratorInfo::getMineshaft()
{
    MineShaftFeature* feature = 0;
    if (hasMineshaft())
        feature = new MineShaftFeature(core());
    return feature;
}
StrongholdFeature* FlatGeneratorInfo::getStronghold()
{
    StrongholdFeature* feature = 0;
    if (core()->hasParam(0x22))
        feature = new StrongholdFeature(core());
    return feature;
}
OceanMonumentFeature* FlatGeneratorInfo::getOceanMonument()
{
    OceanMonumentFeature* feature = 0;
    if (hasOceanMonument())
        feature = new OceanMonumentFeature(core());
    return feature;
}
ShipwreckFeature* FlatGeneratorInfo::getShipwreck(GenericOverworldLevelSource* source, OceanMonumentFeature* monument)
{
    return hasShipwrecks() ? new ShipwreckFeature(source, monument, core()) : 0;
}
LakeFeature* FlatGeneratorInfo::getLakes()
{
    return hasLakes() ? new LakeFeature(Blocks::water) : 0;
}
LakeFeature* FlatGeneratorInfo::getLavaLakes()
{
    return hasLavaLakes() ? new LakeFeature(Blocks::lava) : 0;
}

int FlatGeneratorInfo::getLayerIndexAtHeight(int height) const
{
    int index = 0;
    std::vector<FlatLayerInfo>& layers = core()->layers;
    for (std::vector<FlatLayerInfo>::iterator it = layers.begin(), end = layers.end(); it != end; ++it, ++index)
    {
        int start = it->getStart();
        if (start <= height && height <= it->getEnd())
            return index;
    }
    return -1;
}

FlatLayerInfo FlatGeneratorInfo::getLayer(int index) const
{
    const FlatLayerInfo* layer = &core()->layers[index];
    return *layer;
}

void FlatGeneratorInfo::getLayers(const BlockState* (&states)[256]) const
{
    for (int height = 0; height < 256; ++height)
    {
        int index = getLayerIndexAtHeight(height);
        if (index >= 0)
            states[height] = getLayer(index).getBlockState();
        else
        {
            static const BlockState* air = Blocks::air->defaultBlockState();
            states[height] = air;
        }
    }
}

void FlatGeneratorInfo::getLayers(std::vector<FlatLayerInfo>& layers) const
{
    layers = core()->layers;
}
void FlatGeneratorInfo::setDisplayname(int displayname)
{
    core()->setDisplayname(std::wstring(L10N::GetString(displayname)));
}
void FlatGeneratorInfo::setBiome(int biome)
{
    core()->setOption(0x10, 0, biome);
}

void FlatGeneratorInfo::setVillage(bool enabled)
{
    core()->removeParam(0x20);
    if (enabled)
        core()->addParam(0x20);
}
void FlatGeneratorInfo::setDungeons(bool enabled)
{
    if (enabled)
        core()->addParam(0x31);
    else
        core()->removeParam(0x31);
}
void FlatGeneratorInfo::setDecorations(bool enabled)
{
    if (enabled)
        core()->addParam(0x30);
    else
        core()->removeParam(0x30);
}
void FlatGeneratorInfo::setStronghold(bool enabled)
{
    core()->removeParam(0x22);
    if (enabled)
        core()->addParam(0x22);
}
void FlatGeneratorInfo::setMineshaft(bool enabled)
{
    core()->removeParam(0x23);
    if (enabled)
        core()->addParam(0x23);
}
void FlatGeneratorInfo::setLakes(bool enabled)
{
    if (enabled)
        core()->addParam(0x32);
    else
        core()->removeParam(0x32);
}
void FlatGeneratorInfo::setLavaLakes(bool enabled)
{
    if (enabled)
        core()->addParam(0x33);
    else
        core()->removeParam(0x33);
}
void FlatGeneratorInfo::setBiomeSpecific(bool enabled)
{
    core()->removeParam(0x21);
    if (enabled)
        core()->addParam(0x21);
}
void FlatGeneratorInfo::setWoodlandMansion(bool enabled)
{
    core()->removeParam(0x27);
    if (enabled)
        core()->addParam(0x27);
}
void FlatGeneratorInfo::setOceanMonument(bool enabled)
{
    core()->removeParam(0x24);
    if (enabled)
        core()->addParam(0x24);
}
void FlatGeneratorInfo::setShipwrecks(bool enabled)
{
    core()->removeParam(0x28);
    if (enabled)
        core()->addParam(0x28);
}
void FlatGeneratorInfo::setCaves(bool enabled)
{
    if (enabled)
        core()->addParam(0x25);
    else
        core()->removeParam(0x25);
}
void FlatGeneratorInfo::setCanyons(bool enabled)
{
    if (enabled)
        core()->addParam(0x26);
    else
        core()->removeParam(0x26);
}

void FlatGeneratorInfo::insertLayer(int index, const FlatLayerInfo& layer)
{
    std::vector<FlatLayerInfo>& layers = core()->layers;
    layers.insert(layers.begin() + index, layer);
    core()->updateLayers();
}
void FlatGeneratorInfo::addLayer(const FlatLayerInfo& layer)
{
    insertLayer(0, layer);
}
void FlatGeneratorInfo::setDisplayname(const std::wstring& displayname)
{
    core()->setDisplayname(displayname);
}
void FlatGeneratorInfo::setWoodlandMansion(int spacing, int separation)
{
    setWoodlandMansion(true);
    core()->setOption(0x27, 0, spacing);
    core()->setOption(0x27, 1, separation);
}
void FlatGeneratorInfo::setOceanMonument(int spacing, int separation)
{
    setOceanMonument(true);
    core()->setOption(0x24, 0, spacing);
    core()->setOption(0x24, 1, separation);
}
void FlatGeneratorInfo::setShipwrecks(int spacing, int separation)
{
    setShipwrecks(true);
    core()->setOption(0x28, 0, spacing);
    core()->setOption(0x28, 1, separation);
}
void FlatGeneratorInfo::setBiome()
{
    setBiome(Biome::PLAINS->id);
}
void FlatGeneratorInfo::setVillage(int size, int distance)
{
    core()->removeParam(0x20);
    core()->addParam(0x20);
    core()->setOption(0x20, 0, size);
    core()->setOption(0x20, 1, distance);
}
void FlatGeneratorInfo::setBiomeSpecific(int spacing)
{
    setBiomeSpecific(true);
    core()->setOption(0x21, 0, spacing);
}
void FlatGeneratorInfo::setMineshaft(int chance)
{
    setMineshaft(true);
    core()->setOption(0x23, 0, chance);
}
void FlatGeneratorInfo::setStronghold(int distance, int count)
{
    setStronghold(true);
    core()->setOption(0x22, 0, distance);
    core()->setOption(0x22, 1, count);
}

void FlatGeneratorInfo::setEqualTo(SuperflatConfig* config)
{
    *core() = *config;
}
int FlatGeneratorInfo::numLayers() const
{
    return (int)core()->layers.size();
}
void FlatGeneratorInfo::setLayer(int index, const FlatLayerInfo& layer)
{
    FlatLayerInfo* slot = &core()->layers[index];
    *slot = layer;
    core()->updateLayers();
}
void FlatGeneratorInfo::setLayers(const std::vector<FlatLayerInfo>& layers)
{
    core()->layers = layers;
    core()->updateLayers();
}
void FlatGeneratorInfo::remLayer(int index)
{
    std::vector<FlatLayerInfo>& layers = core()->layers;
    layers.erase(layers.begin() + index);
    core()->updateLayers();
}

void FlatGeneratorInfo::getLayers(int* ids, int* data) const
{
    for (int height = 0; height < 256; ++height)
    {
        int index = getLayerIndexAtHeight(height);
        if (index >= 0)
        {
            FlatLayerInfo layer = getLayer(index);
            ids[height] = Block::getId(layer.getBlock());
            data[height] = layer.getData();
        }
        else
        {
            data[height] = 0;
            ids[height] = 0;
        }
    }
}

int FlatGeneratorInfo::getHeight() const
{
    int height = 0;
    std::vector<FlatLayerInfo> layers;
    this->getLayers(layers);
    for (std::vector<FlatLayerInfo>::iterator it = layers.begin(), end = layers.end(); it != end; ++it)
        height += it->getHeight();
    return height;
}
int FlatGeneratorInfo::getTopLayer() const
{
    return numLayers() - 1;
}

int FlatGeneratorInfo::findTopNonAirLayer() const
{
    int layer = getTopLayer();
    while (layer >= 0)
    {
        FlatLayerInfo current = getLayer(layer);
        if (current.getId() != 0)
            return layer;
        layer -= getLayer(layer).getHeight();
    }
    return -1;
}

int FlatGeneratorInfo::findTopSolidLayer() const
{
    int layer = findTopNonAirLayer();
    while (layer >= 0)
    {
        FlatLayerInfo current = getLayer(layer);
        if (current.getBlockState()->isSolidBlockingCube())
            return layer;
        layer -= getLayer(layer).getHeight();
    }
    return -1;
}

int FlatGeneratorInfo::getBiome()
{
    return core()->maybeOption(0x10, 0, Biome::PLAINS->id);
}
int FlatGeneratorInfo::getVillageSize()
{
    return core()->maybeOption(0x20, 0, 1);
}
int FlatGeneratorInfo::getVillageDistance()
{
    return core()->maybeOption(0x20, 1, 0x20);
}
int FlatGeneratorInfo::getMineshaftChance()
{
    return core()->maybeOption(0x23, 0, 1);
}
int FlatGeneratorInfo::getStrongholdCount()
{
    return core()->maybeOption(0x22, 1, 1);
}
int FlatGeneratorInfo::getStrongholdDistance()
{
    return core()->maybeOption(0x22, 0, 0x20);
}
int FlatGeneratorInfo::getBiomeSpecificSpacing()
{
    return core()->maybeOption(0x21, 0, 0x20);
}
int FlatGeneratorInfo::getOceanMonumentSpacing()
{
    return core()->maybeOption(0x24, 0, 0x20);
}
int FlatGeneratorInfo::getOceanMonuemntSeparation()
{
    return core()->maybeOption(0x24, 1, 5);
}
int FlatGeneratorInfo::getShipwreckSpacing()
{
    return core()->maybeOption(0x28, 0, 0x20);
}
int FlatGeneratorInfo::getShipwreckSeparation()
{
    return core()->maybeOption(0x28, 1, 5);
}
int FlatGeneratorInfo::getWoodlandMansionSpacing()
{
    return core()->maybeOption(0x27, 0, 0x20);
}
int FlatGeneratorInfo::getWoodlandMansionSeparation()
{
    return core()->maybeOption(0x27, 1, 5);
}

std::wstring FlatGeneratorInfo::generateDescription()
{
    std::wstring description = L"";
    description += formatwstr(L"%ls: %ls<br /><br />", L10N::GetString(IDS_BIOME),
                              Biome::biomes[getBiome()]->getName(false).c_str());
    std::vector<FlatLayerInfo> layers;
    getLayers(layers);
    description += formatwstr(L"%ls:<br />", L10N::GetString(IDS_LAYERS));
    for (std::vector<FlatLayerInfo>::reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
        description += formatwstr(L"- %ls %ls<br />", it->getHeightText().c_str(), it->getNameText().c_str());
    std::wstring features = L"";
    if (hasVillage())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_VILLAGE));
    if (hasCaves())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_CAVES));
    if (hasCanyons())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_CANYONS));
    if (hasMineshaft())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_MINESHAFT));
    if (hasStronghold())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_STRONGHOLD));
    if (hasBiomeSpecific())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_BIOMESPECIFIC));
    if (hasDungeons())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_DUNGEON));
    if (hasDecorations())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_DECORATION));
    if (hasLakes())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_LAKES));
    if (hasLavaLakes())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_LAVA_LAKES));
    if (hasOceanMonument())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_OCEAN_MONUMENT));
    if (hasWoodlandMansion())
        features += formatwstr(L"- %ls<br />", L10N::GetString(IDS_PARAM_WOODLAND_MANSION));
    if (!features.empty())
    {
        description += formatwstr(L"<br />%ls:<br />", L10N::GetString(IDS_FEATURES));
        description += features;
    }
    return description;
}

void FlatGeneratorInfo::reduceLayers()
{
    SuperflatConfig* config = core();
    std::vector<FlatLayerInfo>& layers = config->layers;
    if (layers.size() > 1)
    {
        std::vector<FlatLayerInfo>::iterator previous = layers.begin() + 1;
        std::vector<FlatLayerInfo>::iterator current = layers.begin() + 2;
        if (current != layers.end())
        {
            do
            {
                if (previous->getId() == current->getId() && previous->getData() == current->getData())
                {
                    previous->setHeight(previous->getHeight() + current->getHeight());
                    current = layers.erase(current);
                }
                else
                {
                    previous = current;
                    ++current;
                }
            } while (current != layers.end());
            config = core();
        }
    }
    config->updateLayers();
    for (int index = (int)layers.size() - 1; index > 0 && getLayer(index).getId() == 0; --index)
        remLayer(index);
}

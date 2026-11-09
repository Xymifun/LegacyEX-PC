#ifndef GAME_WORLD_GENERATION_FLATGENERATORINFO_HPP
#define GAME_WORLD_GENERATION_FLATGENERATORINFO_HPP

#include <string>
#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>

class BlockState;
class CanyonFeature;
class GenericOverworldLevelSource;
class LakeFeature;
class LargeCaveFeature;
class MineShaftFeature;
class OceanMonumentFeature;
class OverworldLevelSource;
class RandomScatteredLargeFeature;
class ShipwreckFeature;
class StrongholdFeature;
class SuperflatConfig;
class VillageFeature;
class WoodlandMansionFeature;
class FlatLayerInfo;

std::pair<int, int> getSpacingAndSeparationFromPercentage(int spacing, int separationPercent, int minimumSeparation);

class FlatGeneratorInfo
{
public:
    FlatGeneratorInfo();
    FlatGeneratorInfo(SuperflatConfig* config);
    FlatGeneratorInfo(SuperflatConfig* config, bool copy);
    ~FlatGeneratorInfo();
    SuperflatConfig* core() const;
    bool hasCaves();
    LargeCaveFeature* getCaves();
    bool hasCanyons();
    CanyonFeature* getCanyons();
    bool hasVillage();
    VillageFeature* getVillage(int size);
    bool hasBiomeSpecific();
    RandomScatteredLargeFeature* getBiomeSpecific();
    bool hasMineshaft();
    MineShaftFeature* getMineshaft();
    bool hasStronghold();
    StrongholdFeature* getStronghold();
    bool hasOceanMonument();
    OceanMonumentFeature* getOceanMonument();
    bool hasWoodlandMansion();
    WoodlandMansionFeature* getWoodlandMansion(OverworldLevelSource* source);
    bool hasShipwrecks();
    ShipwreckFeature* getShipwreck(GenericOverworldLevelSource* source, OceanMonumentFeature* monument);
    bool hasDecorations();
    bool hasDungeons();
    bool hasLakes();
    LakeFeature* getLakes();
    bool hasLavaLakes();
    LakeFeature* getLavaLakes();
    int getLayerIndexAtHeight(int height) const;
    FlatLayerInfo getLayer(int index) const;
    void getLayers(const BlockState* (&states)[256]) const;
    void getLayers(std::vector<FlatLayerInfo>& layers) const;
    void setDisplayname(int displayname);
    void setBiome(int biome);
    void setVillage(bool enabled);
    void insertLayer(int index, const FlatLayerInfo& layer);
    void addLayer(const FlatLayerInfo& layer);
    void setDungeons(bool enabled);
    void setDecorations(bool enabled);
    void setStronghold(bool enabled);
    void setMineshaft(bool enabled);
    void setLakes(bool enabled);
    void setLavaLakes(bool enabled);
    void setBiomeSpecific(bool enabled);
    void setDisplayname(const std::wstring& displayname);
    void setWoodlandMansion(bool enabled);
    void setWoodlandMansion(int spacing, int separation);
    void setOceanMonument(bool enabled);
    void setOceanMonument(int spacing, int separation);
    void setShipwrecks(bool enabled);
    void setBiome();
    void setVillage(int size, int distance);
    void setBiomeSpecific(int spacing);
    void setEqualTo(SuperflatConfig* config);
    int numLayers() const;
    void setLayer(int index, const FlatLayerInfo& layer);
    void setLayers(const std::vector<FlatLayerInfo>& layers);
    void remLayer(int index);
    void getLayers(int* ids, int* data) const;
    int getHeight() const;
    int getTopLayer() const;
    int findTopNonAirLayer() const;
    int findTopSolidLayer() const;
    void setCaves(bool enabled);
    void setCanyons(bool enabled);
    void setMineshaft(int chance);
    void setStronghold(int distance, int count);
    void setShipwrecks(int spacing, int separation);
    int getBiome();
    int getVillageSize();
    int getVillageDistance();
    int getMineshaftChance();
    int getStrongholdCount();
    int getStrongholdDistance();
    int getBiomeSpecificSpacing();
    int getOceanMonumentSpacing();
    int getOceanMonuemntSeparation();
    int getShipwreckSpacing();
    int getShipwreckSeparation();
    int getWoodlandMansionSpacing();
    int getWoodlandMansionSeparation();
    std::wstring generateDescription();
    void reduceLayers();
    static FlatGeneratorInfo getDefault();
    void setEqualTo(const FlatGeneratorInfo& info);
    std::wstring getDisplayname();
    bool hasLayer(int index) const;
    FlatLayerInfo getLayerAtHeight(int height) const;
    void getSingleLayers(std::vector<FlatLayerInfo>& layers) const;
    bool operator==(const FlatGeneratorInfo& info) const;

private:
    bool mOwnsConfig;
    SuperflatConfig* mConfig;
    boost::shared_ptr<SuperflatConfig> mOwnedConfig;
};

#endif

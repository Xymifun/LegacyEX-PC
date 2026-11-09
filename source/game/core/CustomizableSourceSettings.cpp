#include "game/core/CustomizableSourceSettings.hpp"

CustomizableSourceSettings::Builder::Builder()
{
    setDefaults();
}

void CustomizableSourceSettings::Builder::setDefaults()
{
    coordinateScale = 684.411987f;
    heightScale = 684.411987f;
    lowerLimitScale = 512.0f;
    upperLimitScale = 512.0f;
    depthNoiseScaleX = 200.0f;
    depthNoiseScaleZ = 200.0f;
    depthNoiseScaleExponent = 0.5f;
    mainNoiseScaleX = 80.0f;
    mainNoiseScaleY = 160.0f;
    mainNoiseScaleZ = 80.0f;
    baseSize = 8.5f;
    stretchY = 12.0f;
    biomeDepthWeight = 1.0f;
    biomeDepthOffset = 0.0f;
    biomeScaleWeight = 1.0f;
    biomeScaleOffset = 0.0f;
    seaLevel = 63;
    useCaves = true;
    useDungeons = true;
    dungeonChance = 8;
    useStrongholds = true;
    useVillages = true;
    useMineShafts = true;
    useTemples = true;
    useOceanRuins = true;
    useMonuments = true;
    useMansions = true;
    useShipwrecks = true;
    useRavines = true;
    useWaterLakes = true;
    waterLakeChance = 4;
    useLavaLakes = true;
    lavaLakeChance = 80;
    useLavaOceans = false;
    useBuriedTreasure = true;
    fixedBiome = -1;
    biomeSize = 4;
    riverSize = 4;
    dirtCount = 33;
    dirtSize = 10;
    dirtMinHeight = 0;
    dirtMaxHeight = 128;
    gravelCount = 33;
    gravelSize = 8;
    gravelMinHeight = 0;
    gravelMaxHeight = 128;
    dioriteCount = 33;
    dioriteSize = 10;
    dioriteMinHeight = 0;
    dioriteMaxHeight = 80;
    graniteCount = 33;
    graniteSize = 10;
    graniteMinHeight = 0;
    graniteMaxHeight = 80;
    andesiteCount = 33;
    andesiteSize = 10;
    andesiteMinHeight = 0;
    andesiteMaxHeight = 80;
    coalCount = 17;
    coalSize = 20;
    coalMinHeight = 0;
    coalMaxHeight = 128;
    ironCount = 9;
    ironSize = 20;
    ironMinHeight = 0;
    ironMaxHeight = 64;
    goldCount = 9;
    goldSize = 2;
    goldMinHeight = 0;
    goldMaxHeight = 32;
    redstoneCount = 8;
    redstoneSize = 8;
    redstoneMinHeight = 0;
    redstoneMaxHeight = 16;
    diamondCount = 8;
    diamondSize = 1;
    diamondMinHeight = 0;
    diamondMaxHeight = 16;
    lapisCount = 7;
    lapisSize = 1;
    lapisMinHeight = 16;
    lapisMaxHeight = 16;
}

CustomizableSourceSettings::Builder* CustomizableSourceSettings::Builder::fromString(SuperflatConfig*)
{
    return new Builder();
}

CustomizableSourceSettings* CustomizableSourceSettings::Builder::build()
{
    return new CustomizableSourceSettings(this);
}

CustomizableSourceSettings::CustomizableSourceSettings(Builder* builder)
    : coordinateScale(builder->coordinateScale),
      heightScale(builder->heightScale),
      lowerLimitScale(builder->lowerLimitScale),
      upperLimitScale(builder->upperLimitScale),
      depthNoiseScaleX(builder->depthNoiseScaleX),
      depthNoiseScaleZ(builder->depthNoiseScaleZ),
      depthNoiseScaleExponent(builder->depthNoiseScaleExponent),
      mainNoiseScaleX(builder->mainNoiseScaleX),
      mainNoiseScaleY(builder->mainNoiseScaleY),
      mainNoiseScaleZ(builder->mainNoiseScaleZ),
      baseSize(builder->baseSize),
      stretchY(builder->stretchY),
      biomeDepthWeight(builder->biomeDepthWeight),
      biomeDepthOffset(builder->biomeDepthOffset),
      biomeScaleWeight(builder->biomeScaleWeight),
      biomeScaleOffset(builder->biomeScaleOffset),
      seaLevel(builder->seaLevel),
      useCaves(builder->useCaves),
      useDungeons(builder->useDungeons),
      dungeonChance(builder->dungeonChance),
      useStrongholds(builder->useStrongholds),
      useVillages(builder->useVillages),
      useMineShafts(builder->useMineShafts),
      useTemples(builder->useTemples),
      useOceanRuins(builder->useOceanRuins),
      useMonuments(builder->useMonuments),
      useMansions(builder->useMansions),
      useShipwrecks(builder->useShipwrecks),
      useRavines(builder->useRavines),
      useWaterLakes(builder->useWaterLakes),
      waterLakeChance(builder->waterLakeChance),
      useLavaLakes(builder->useLavaLakes),
      lavaLakeChance(builder->lavaLakeChance),
      useLavaOceans(builder->useLavaOceans),
      useBuriedTreasure(builder->useBuriedTreasure),
      fixedBiome(builder->fixedBiome),
      biomeSize(builder->biomeSize),
      riverSize(builder->riverSize),
      dirtCount(builder->dirtCount),
      dirtSize(builder->dirtSize),
      dirtMinHeight(builder->dirtMinHeight),
      dirtMaxHeight(builder->dirtMaxHeight),
      gravelCount(builder->gravelCount),
      gravelSize(builder->gravelSize),
      gravelMinHeight(builder->gravelMinHeight),
      gravelMaxHeight(builder->gravelMaxHeight),
      dioriteCount(builder->dioriteCount),
      dioriteSize(builder->dioriteSize),
      dioriteMinHeight(builder->dioriteMinHeight),
      dioriteMaxHeight(builder->dioriteMaxHeight),
      graniteCount(builder->graniteCount),
      graniteSize(builder->graniteSize),
      graniteMinHeight(builder->graniteMinHeight),
      graniteMaxHeight(builder->graniteMaxHeight),
      andesiteCount(builder->andesiteCount),
      andesiteSize(builder->andesiteSize),
      andesiteMinHeight(builder->andesiteMinHeight),
      andesiteMaxHeight(builder->andesiteMaxHeight),
      coalCount(builder->coalCount),
      coalSize(builder->coalSize),
      coalMinHeight(builder->coalMinHeight),
      coalMaxHeight(builder->coalMaxHeight),
      ironCount(builder->ironCount),
      ironSize(builder->ironSize),
      ironMinHeight(builder->ironMinHeight),
      ironMaxHeight(builder->ironMaxHeight),
      goldCount(builder->goldCount),
      goldSize(builder->goldSize),
      goldMinHeight(builder->goldMinHeight),
      goldMaxHeight(builder->goldMaxHeight),
      redstoneCount(builder->redstoneCount),
      redstoneSize(builder->redstoneSize),
      redstoneMinHeight(builder->redstoneMinHeight),
      redstoneMaxHeight(builder->redstoneMaxHeight),
      diamondCount(builder->diamondCount),
      diamondSize(builder->diamondSize),
      diamondMinHeight(builder->diamondMinHeight),
      diamondMaxHeight(builder->diamondMaxHeight),
      lapisCount(builder->lapisCount),
      lapisSize(builder->lapisSize),
      lapisMinHeight(builder->lapisMinHeight),
      lapisMaxHeight(builder->lapisMaxHeight)
{
}

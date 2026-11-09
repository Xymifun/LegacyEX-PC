#ifndef GAME_WORLD_BLOCK_MATERIAL_HPP
#define GAME_WORLD_BLOCK_MATERIAL_HPP

#include "game/world/block/MaterialColor.hpp"

class Material
{
public:
    static Material* AIR;
    static Material* GRASS;
    static Material* DIRT;
    static Material* WOOD;
    static Material* STONE;
    static Material* METAL;
    static Material* HEAVY_METAL;
    static Material* WATER;
    static Material* BUBBLE_COLUMN;
    static Material* LAVA;
    static Material* LEAVES;
    static Material* PLANT;
    static Material* REPLACEABLE_PLANT;
    static Material* BAMBOO;
    static Material* SPONGE;
    static Material* WOOL;
    static Material* FIRE;
    static Material* SAND;
    static Material* DECORATION;
    static Material* CLOTH_DECORATION;
    static Material* CONDUIT;
    static Material* GLASS;
    static Material* BUILDABLE_GLASS;
    static Material* EXPLOSIVE;
    static Material* CORAL;
    static Material* ICE;
    static Material* ICE_SOLID;
    static Material* TOP_SNOW;
    static Material* SNOW;
    static Material* CACTUS;
    static Material* CLAY;
    static Material* VEGETABLE;
    static Material* EGG;
    static Material* PORTAL;
    static Material* CAKE;
    static Material* WEB;
    static Material* PISTON;
    static Material* BARRIER;
    static Material* STRUCTURAL_AIR;

    Material(const MaterialColor* color);
    virtual ~Material() {}

    static void staticCtor();

    virtual bool isLiquid();
    virtual bool letsWaterThrough();
    virtual bool isSolid();
    virtual bool blocksLight();
    virtual bool blocksMotion();
    virtual Material* neverBuildable();
    virtual Material* notAlwaysDestroyable();
    virtual Material* flammable();
    virtual bool isFlammable();
    virtual Material* replaceable();
    virtual bool isReplaceable();
    virtual bool isSolidBlocking();
    virtual bool isAlwaysDestroyable();
    virtual int getPushReaction();

    Material* notPushable();
    Material* underWater();
    Material* destroyOnPush();
    Material* makeDestroyedByHand();

    bool isUnderWater();
    bool isDestroyedByHand();
    const MaterialColor* getColor();

    bool burnable;
    bool canBeReplaced;
    bool notBuildable;
    const MaterialColor* color;
    bool alwaysDestroyable;
    int pushReaction;
    bool destroyedByHand;
    bool submerged;
};

#endif

#include "game/world/block/Material.hpp"

#include "game/world/block/DecorationMaterial.hpp"
#include "game/world/block/GasMaterial.hpp"
#include "game/world/block/LiquidMaterial.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PortalMaterial.hpp"
#include "game/world/block/WebMaterial.hpp"

Material* Material::AIR;
Material* Material::GRASS;
Material* Material::DIRT;
Material* Material::WOOD;
Material* Material::STONE;
Material* Material::METAL;
Material* Material::HEAVY_METAL;
Material* Material::WATER;
Material* Material::BUBBLE_COLUMN;
Material* Material::LAVA;
Material* Material::LEAVES;
Material* Material::PLANT;
Material* Material::REPLACEABLE_PLANT;
Material* Material::BAMBOO;
Material* Material::SPONGE;
Material* Material::WOOL;
Material* Material::FIRE;
Material* Material::SAND;
Material* Material::DECORATION;
Material* Material::CLOTH_DECORATION;
Material* Material::CONDUIT;
Material* Material::GLASS;
Material* Material::BUILDABLE_GLASS;
Material* Material::EXPLOSIVE;
Material* Material::CORAL;
Material* Material::ICE;
Material* Material::ICE_SOLID;
Material* Material::TOP_SNOW;
Material* Material::SNOW;
Material* Material::CACTUS;
Material* Material::CLAY;
Material* Material::VEGETABLE;
Material* Material::EGG;
Material* Material::PORTAL;
Material* Material::CAKE;
Material* Material::WEB;
Material* Material::PISTON;
Material* Material::BARRIER;
Material* Material::STRUCTURAL_AIR;

Material::Material(const MaterialColor* color)
{
    this->color = color;

    burnable = false;
    canBeReplaced = false;
    notBuildable = false;
    alwaysDestroyable = true;
    pushReaction = 0;
    destroyedByHand = false;
    submerged = false;
}

Material* Material::replaceable()
{
    canBeReplaced = true;
    return this;
}

Material* Material::flammable()
{
    burnable = true;
    return this;
}

Material* Material::notAlwaysDestroyable()
{
    alwaysDestroyable = false;
    return this;
}

Material* Material::notPushable()
{
    pushReaction = 2;
    return this;
}

Material* Material::underWater()
{
    submerged = true;
    return this;
}

Material* Material::destroyOnPush()
{
    pushReaction = 1;
    return this;
}

Material* Material::makeDestroyedByHand()
{
    destroyedByHand = true;
    return this;
}

Material* Material::neverBuildable()
{
    notBuildable = true;
    return this;
}

void Material::staticCtor()
{
    AIR = new GasMaterial(MaterialColor::NONE);
    GRASS = new Material(MaterialColor::GRASS);
    DIRT = new Material(MaterialColor::DIRT);
    WOOD = (new Material(MaterialColor::WOOD))->flammable();
    STONE = (new Material(MaterialColor::STONE))->notAlwaysDestroyable();
    METAL = (new Material(MaterialColor::METAL))->notAlwaysDestroyable();
    HEAVY_METAL = (new Material(MaterialColor::METAL))->notAlwaysDestroyable()->notPushable();
    WATER = (new LiquidMaterial(MaterialColor::WATER))->destroyOnPush()->underWater();
    BUBBLE_COLUMN = (new LiquidMaterial(MaterialColor::WATER))->destroyOnPush()->underWater();
    LAVA = (new LiquidMaterial(MaterialColor::FIRE))->destroyOnPush();
    LEAVES = (new Material(MaterialColor::PLANT))->flammable()->neverBuildable()->destroyOnPush();
    PLANT = (new DecorationMaterial(MaterialColor::PLANT))->destroyOnPush();
    REPLACEABLE_PLANT = (new DecorationMaterial(MaterialColor::PLANT))->flammable()->destroyOnPush()->replaceable();
    BAMBOO = (new Material(MaterialColor::PLANT))->destroyOnPush();
    SPONGE = new Material(MaterialColor::CLOTH);
    WOOL = (new Material(MaterialColor::CLOTH))->flammable();
    FIRE = (new GasMaterial(MaterialColor::NONE))->destroyOnPush();
    SAND = new Material(MaterialColor::SAND);
    DECORATION = (new DecorationMaterial(MaterialColor::NONE))->destroyOnPush();
    CLOTH_DECORATION = (new DecorationMaterial(MaterialColor::CLOTH))->flammable();
    CONDUIT = new DecorationMaterial(MaterialColor::STONE);
    GLASS = (new Material(MaterialColor::NONE))->neverBuildable()->makeDestroyedByHand();
    BUILDABLE_GLASS = (new Material(MaterialColor::NONE))->makeDestroyedByHand();
    EXPLOSIVE = (new Material(MaterialColor::FIRE))->flammable()->neverBuildable();
    CORAL = (new Material(MaterialColor::PLANT))->destroyOnPush();
    ICE = (new Material(MaterialColor::ICE))->neverBuildable();
    ICE_SOLID = new Material(MaterialColor::ICE);
    TOP_SNOW = (new DecorationMaterial(MaterialColor::SNOW))
                   ->replaceable()
                   ->neverBuildable()
                   ->notAlwaysDestroyable()
                   ->destroyOnPush();
    SNOW = (new Material(MaterialColor::SNOW))->notAlwaysDestroyable();
    CACTUS = (new Material(MaterialColor::PLANT))->neverBuildable()->destroyOnPush();
    CLAY = new Material(MaterialColor::CLAY);
    VEGETABLE = (new Material(MaterialColor::PLANT))->destroyOnPush();
    EGG = (new Material(MaterialColor::PLANT))->destroyOnPush();
    PORTAL = (new PortalMaterial(MaterialColor::NONE))->notPushable();
    CAKE = (new Material(MaterialColor::NONE))->destroyOnPush();
    WEB = (new WebMaterial(MaterialColor::CLOTH))->notAlwaysDestroyable()->destroyOnPush();
    PISTON = (new Material(MaterialColor::STONE))->notPushable();
    BARRIER = (new Material(MaterialColor::NONE))->notAlwaysDestroyable()->notPushable();
    STRUCTURAL_AIR = new GasMaterial(MaterialColor::NONE);
}

bool Material::isLiquid()
{
    return false;
}

bool Material::letsWaterThrough()
{
    return !isLiquid() && !isSolid();
}

bool Material::isSolid()
{
    return true;
}

bool Material::blocksLight()
{
    return true;
}

bool Material::blocksMotion()
{
    return true;
}

bool Material::isFlammable()
{
    return burnable;
}

bool Material::isReplaceable()
{
    return canBeReplaced;
}

bool Material::isUnderWater()
{
    return submerged;
}

bool Material::isSolidBlocking()
{
    if (notBuildable)
    {
        return false;
    }

    return blocksMotion();
}

bool Material::isAlwaysDestroyable()
{
    return alwaysDestroyable;
}

int Material::getPushReaction()
{
    return pushReaction;
}

bool Material::isDestroyedByHand()
{
    return destroyedByHand;
}

const MaterialColor* Material::getColor()
{
    return color;
}

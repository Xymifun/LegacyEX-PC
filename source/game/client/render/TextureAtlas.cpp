#include "game/client/render/TextureAtlas.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "game/core/ClockAtlasSprite.hpp"
#include "game/core/CompassAtlasSprite.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/MemSect.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TextureHolder.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ItemFrameRenderer.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/GlideRingGenerator.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcModContent.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static std::wstring MISSING_ICON_NAME(L"missingno");

TextureAtlas::TextureAtlas(int iconType, const std::wstring& name, const std::wstring& path, BufferedImage* image,
                           bool singleImage)
    : mIconType(iconType),
      mName(name),
      mPath(path),
      mSpritePrefix(L"")
{
    mImage = image;
    mMissingIcon = 0;
    mStitchedTexture = 0;
    mSingleImage = singleImage;
    mMissingIcon = TextureAtlasSprite::create(MISSING_ICON_NAME);
}

void TextureAtlas::addIcon(bool singleImage, int x, int y, const std::wstring& name)
{
    const float vScale = singleImage ? 0.029411765f : 0.05882353f;
    const float scale = 0.0625f;
    const float vOffset = singleImage ? 544.0f : 272.0f;
    mSprites[name] = new TextureAtlasSprite(name, name, scale * static_cast<float>(y), vScale * static_cast<float>(x),
                                            scale * static_cast<float>(y + 1), vScale * static_cast<float>(x + 1),
                                            256.0f, vOffset);
}

void TextureAtlas::addIconWithName(bool singleImage, int x, int y, const std::wstring& name,
                                   const std::wstring& spriteName)
{
    const float vScale = singleImage ? 0.029411765f : 0.05882353f;
    const float scale = 0.0625f;
    const float vOffset = singleImage ? 544.0f : 272.0f;
    mSprites[name] = new TextureAtlasSprite(name, spriteName, scale * static_cast<float>(y),
                                            vScale * static_cast<float>(x), scale * static_cast<float>(y + 1),
                                            vScale * static_cast<float>(x + 1), 256.0f, vOffset);
}

void TextureAtlas::addIconSize(bool singleImage, int x, int y, const std::wstring& name, int width, int height)
{
    const float vScale = singleImage ? 0.029411765f : 0.05882353f;
    const float scale = 0.0625f;
    const float vOffset = singleImage ? 544.0f : 272.0f;
    mSprites[name] = new TextureAtlasSprite(name, name, scale * static_cast<float>(y), vScale * static_cast<float>(x),
                                            scale * static_cast<float>(y + width),
                                            vScale * static_cast<float>(x + height), 256.0f, vOffset);
}

void TextureAtlas::loadUVs()
{
    if (!mSprites.empty())
        return;
    for (boost::unordered_map<std::wstring, TextureAtlasSprite*>::iterator it = mSprites.begin(); it != mSprites.end();
         ++it)
        delete it->second;
    mSprites.clear();
    if (mIconType != 0)
    {
        addIcon(false, 0, 0, L"helmetCloth");
        addIcon(false, 0, 1, L"helmetChain");
        addIcon(false, 0, 2, L"helmetIron");
        addIcon(false, 0, 3, L"helmetDiamond");
        addIcon(false, 0, 4, L"helmetGold");
        addIcon(false, 0, 5, L"flintAndSteel");
        addIcon(false, 0, 6, L"flint");
        addIcon(false, 0, 7, L"coal");
        addIcon(false, 0, 8, L"string");
        addIcon(false, 0, 9, L"seeds");
        addIcon(false, 0, 10, L"apple");
        addIcon(false, 0, 11, L"appleGold");
        addIcon(false, 0, 12, L"egg");
        addIcon(false, 0, 13, L"sugar");
        addIcon(false, 0, 14, L"snowball");
        addIcon(false, 0, 15, L"elytra");
        addIcon(false, 1, 0, L"chestplateCloth");
        addIcon(false, 1, 1, L"chestplateChain");
        addIcon(false, 1, 2, L"chestplateIron");
        addIcon(false, 1, 3, L"chestplateDiamond");
        addIcon(false, 1, 4, L"chestplateGold");
        addIcon(false, 1, 5, L"bow");
        addIcon(false, 1, 6, L"brick");
        addIcon(false, 1, 7, L"ingotIron");
        addIcon(false, 1, 8, L"feather");
        addIcon(false, 1, 9, L"wheat");
        addIcon(false, 1, 10, L"painting");
        addIcon(false, 1, 11, L"reeds");
        addIcon(false, 1, 12, L"bone");
        addIcon(false, 1, 13, L"cake");
        addIcon(false, 1, 14, L"slimeball");
        addIcon(false, 1, 15, L"broken_elytra");
        addIcon(false, 2, 0, L"leggingsCloth");
        addIcon(false, 2, 1, L"leggingsChain");
        addIcon(false, 2, 2, L"leggingsIron");
        addIcon(false, 2, 3, L"leggingsDiamond");
        addIcon(false, 2, 4, L"leggingsGold");
        addIcon(false, 2, 5, L"arrow");
        addIcon(false, 2, 6, L"end_crystal");
        addIcon(false, 2, 7, L"ingotGold");
        addIcon(false, 2, 8, L"sulphur");
        addIcon(false, 2, 9, L"bread");
        addIcon(false, 2, 10, L"sign");
        addIcon(false, 2, 11, L"doorWood");
        addIcon(false, 2, 12, L"doorIron");
        addIcon(false, 2, 13, L"bed");
        addIcon(false, 2, 14, L"fireball");
        addIcon(false, 2, 15, L"chorus_fruit");
        addIcon(false, 3, 0, L"bootsCloth");
        addIcon(false, 3, 1, L"bootsChain");
        addIcon(false, 3, 2, L"bootsIron");
        addIcon(false, 3, 3, L"bootsDiamond");
        addIcon(false, 3, 4, L"bootsGold");
        addIcon(false, 3, 5, L"stick");
        addIcon(false, 3, 6, L"compass");
        addIcon(false, 3, 7, L"diamond");
        addIcon(false, 3, 8, L"redstone");
        addIcon(false, 3, 9, L"clay");
        addIcon(false, 3, 10, L"paper");
        addIcon(false, 3, 11, L"book");
        addIcon(false, 3, 12, L"map");
        addIcon(false, 3, 13, L"seeds_pumpkin");
        addIcon(false, 3, 14, L"seeds_melon");
        addIcon(false, 3, 15, L"chorus_fruit_popped");
        addIcon(false, 4, 0, L"swordWood");
        addIcon(false, 4, 1, L"swordStone");
        addIcon(false, 4, 2, L"swordIron");
        addIcon(false, 4, 3, L"swordDiamond");
        addIcon(false, 4, 4, L"swordGold");
        addIcon(false, 4, 5, L"fishingRod_uncast");
        addIcon(false, 4, 6, L"clock");
        addIcon(false, 4, 7, L"bowl");
        addIcon(false, 4, 8, L"mushroomStew");
        addIcon(false, 4, 9, L"yellowDust");
        addIcon(false, 4, 10, L"bucket");
        addIcon(false, 4, 11, L"bucketWater");
        addIcon(false, 4, 12, L"bucketLava");
        addIcon(false, 4, 13, L"milk");
        addIcon(false, 4, 14, L"dyePowder_black");
        addIcon(false, 4, 15, L"dyePowder_gray");
        addIcon(false, 5, 0, L"shovelWood");
        addIcon(false, 5, 1, L"shovelStone");
        addIcon(false, 5, 2, L"shovelIron");
        addIcon(false, 5, 3, L"shovelDiamond");
        addIcon(false, 5, 4, L"shovelGold");
        addIcon(false, 5, 5, L"fishingRod_cast");
        addIcon(false, 5, 6, L"diode");
        addIcon(false, 5, 7, L"porkchopRaw");
        addIcon(false, 5, 8, L"porkchopCooked");
        addIcon(false, 5, 9, L"fishRaw");
        addIcon(false, 5, 10, L"fishCooked");
        addIcon(false, 5, 11, L"rottenFlesh");
        addIcon(false, 5, 12, L"cookie");
        addIcon(false, 5, 13, L"shears");
        addIcon(false, 5, 14, L"dyePowder_red");
        addIcon(false, 5, 15, L"dyePowder_pink");
        addIcon(false, 6, 0, L"pickaxeWood");
        addIcon(false, 6, 1, L"pickaxeStone");
        addIcon(false, 6, 2, L"pickaxeIron");
        addIcon(false, 6, 3, L"pickaxeDiamond");
        addIcon(false, 6, 4, L"pickaxeGold");
        addIcon(false, 6, 5, L"bow_pull_0");
        addIcon(false, 6, 6, L"carrotOnAStick");
        addIcon(false, 6, 7, L"leather");
        addIcon(false, 6, 8, L"saddle");
        addIcon(false, 6, 9, L"beefRaw");
        addIcon(false, 6, 10, L"beefCooked");
        addIcon(false, 6, 11, L"enderPearl");
        addIcon(false, 6, 12, L"blazeRod");
        addIcon(false, 6, 13, L"melon");
        addIcon(false, 6, 14, L"dyePowder_green");
        addIcon(false, 6, 15, L"dyePowder_lime");
        addIcon(false, 7, 0, L"hatchetWood");
        addIcon(false, 7, 1, L"hatchetStone");
        addIcon(false, 7, 2, L"hatchetIron");
        addIcon(false, 7, 3, L"hatchetDiamond");
        addIcon(false, 7, 4, L"hatchetGold");
        addIcon(false, 7, 5, L"bow_pull_1");
        addIcon(false, 7, 6, L"potatoBaked");
        addIcon(false, 7, 7, L"potato");
        addIcon(false, 7, 8, L"carrots");
        addIcon(false, 7, 9, L"chickenRaw");
        addIcon(false, 7, 10, L"chickenCooked");
        addIcon(false, 7, 11, L"ghastTear");
        addIcon(false, 7, 12, L"goldNugget");
        addIcon(false, 7, 13, L"netherStalkSeeds");
        addIcon(false, 7, 14, L"dyePowder_brown");
        addIcon(false, 7, 15, L"dyePowder_yellow");
        addIcon(false, 8, 0, L"hoeWood");
        addIcon(false, 8, 1, L"hoeStone");
        addIcon(false, 8, 2, L"hoeIron");
        addIcon(false, 8, 3, L"hoeDiamond");
        addIcon(false, 8, 4, L"hoeGold");
        addIcon(false, 8, 5, L"bow_pull_2");
        addIcon(false, 8, 6, L"potatoPoisonous");
        addIcon(false, 8, 7, L"minecart");
        addIcon(false, 8, 8, L"boat");
        addIcon(false, 8, 9, L"speckledMelon");
        addIcon(false, 8, 10, L"fermentedSpiderEye");
        addIcon(false, 8, 11, L"spiderEye");
        addIcon(false, 8, 12, L"potion");
        addIcon(false, 8, 12, L"glassBottle");
        addIcon(false, 8, 13, L"potion_contents");
        addIcon(false, 8, 14, L"dyePowder_blue");
        addIcon(false, 8, 15, L"dyePowder_light_blue");
        addIcon(false, 9, 0, L"helmetCloth_overlay");
        addIcon(false, 9, 1, L"spectral_arrow");
        addIcon(false, 9, 2, L"iron_horse_armor");
        addIcon(false, 9, 3, L"diamond_horse_armor");
        addIcon(false, 9, 4, L"gold_horse_armor");
        addIcon(false, 9, 5, L"comparator");
        addIcon(false, 9, 6, L"carrotGolden");
        addIcon(false, 9, 7, L"minecart_chest");
        addIcon(false, 9, 8, L"pumpkinPie");
        addIcon(false, 9, 9, L"monsterPlacer");
        addIcon(false, 9, 10, L"potion_splash");
        addIcon(false, 9, 11, L"eyeOfEnder");
        addIcon(false, 9, 12, L"cauldron");
        addIcon(false, 9, 13, L"blazePowder");
        addIcon(false, 9, 14, L"dyePowder_purple");
        addIcon(false, 9, 15, L"dyePowder_magenta");
        addIcon(false, 10, 0, L"chestplateCloth_overlay");
        addIcon(false, 10, 1, L"tipped_arrow_base");
        addIcon(false, 10, 2, L"dragon_breath");
        addIcon(false, 10, 3, L"name_tag");
        addIcon(false, 10, 4, L"lead");
        addIcon(false, 10, 5, L"netherbrick");
        addIcon(false, 10, 6, L"fish_clownfish_raw");
        addIcon(false, 10, 7, L"minecart_furnace");
        addIcon(false, 10, 8, L"charcoal");
        addIcon(false, 10, 9, L"monsterPlacer_overlay");
        addIcon(false, 10, 10, L"bed_overlay");
        addIcon(false, 10, 11, L"expBottle");
        addIcon(false, 10, 12, L"brewingStand");
        addIcon(false, 10, 13, L"magmaCream");
        addIcon(false, 10, 14, L"dyePowder_cyan");
        addIcon(false, 10, 15, L"dyePowder_orange");
        addIcon(false, 11, 0, L"leggingsCloth_overlay");
        addIcon(false, 11, 1, L"tipped_arrow_head");
        addIcon(false, 11, 2, L"potion_bottle_lingering");
        addIcon(false, 11, 3, L"barrier");
        addIcon(false, 11, 4, L"mutton_raw");
        addIcon(false, 11, 5, L"rabbit_raw");
        addIcon(false, 11, 6, L"fish_pufferfish_raw");
        addIcon(false, 11, 7, L"minecart_hopper");
        addIcon(false, 11, 8, L"hopper");
        addIcon(false, 11, 9, L"nether_star");
        addIcon(false, 11, 10, L"emerald");
        addIcon(false, 11, 11, L"writingBook");
        addIcon(false, 11, 12, L"writtenBook");
        addIcon(false, 11, 13, L"flowerPot");
        addIcon(false, 11, 14, L"dyePowder_silver");
        addIcon(false, 11, 15, L"dyePowder_white");
        addIcon(false, 12, 0, L"bootsCloth_overlay");
        addIcon(false, 12, 1, L"beetroot");
        addIcon(false, 12, 2, L"beetroot_seeds");
        addIcon(false, 12, 3, L"beetroot_soup");
        addIcon(false, 12, 4, L"mutton_cooked");
        addIcon(false, 12, 5, L"rabbit_cooked");
        addIcon(false, 12, 6, L"fish_salmon_raw");
        addIcon(false, 12, 7, L"minecart_tnt");
        addIcon(false, 12, 8, L"wooden_armorstand");
        addIcon(false, 12, 9, L"fireworks");
        addIcon(false, 12, 10, L"fireworks_charge");
        addIcon(false, 12, 11, L"fireworks_charge_overlay");
        addIcon(false, 12, 12, L"netherquartz");
        addIcon(false, 12, 13, L"map_empty");
        addIcon(false, 12, 14, L"frame");
        addIcon(false, 12, 15, L"enchantedBook");
        addIcon(false, 13, 0, L"door_acacia");
        addIcon(false, 13, 1, L"door_birch");
        addIcon(false, 13, 2, L"door_dark_oak");
        addIcon(false, 13, 3, L"door_jungle");
        addIcon(false, 13, 4, L"door_spruce");
        addIcon(false, 13, 5, L"rabbit_stew");
        addIcon(false, 13, 6, L"fish_salmon_cooked");
        addIcon(false, 13, 7, L"minecart_command_block");
        addIcon(false, 13, 8, L"acacia_boat");
        addIcon(false, 13, 9, L"birch_boat");
        addIcon(false, 13, 10, L"dark_oak_boat");
        addIcon(false, 13, 11, L"jungle_boat");
        addIcon(false, 13, 12, L"spruce_boat");
        addIcon(false, 13, 13, L"prismarine_shard");
        addIcon(false, 13, 14, L"prismarine_crystals");
        addIcon(false, 13, 15, L"leather_horse_armor_base");
        addIcon(false, 14, 0, L"structure_void");
        addIcon(false, 14, 1, L"map_filled_markings");
        addIcon(false, 14, 2, L"totem");
        addIcon(false, 14, 3, L"shulker_shell");
        addIcon(false, 14, 4, L"iron_nugget");
        addIcon(false, 14, 5, L"rabbit_foot");
        addIcon(false, 14, 6, L"rabbit_hide");
        addIconWithName(false, 14, 7, L"compassP0", L"compass");
        addIconWithName(false, 14, 8, L"compassP1", L"compass");
        addIconWithName(false, 14, 9, L"compassP2", L"compass");
        addIconWithName(false, 14, 10, L"compassP3", L"compass");
        addIconWithName(false, 14, 11, L"clockP0", L"clock");
        addIconWithName(false, 14, 12, L"clockP1", L"clock");
        addIconWithName(false, 14, 13, L"clockP2", L"clock");
        addIconWithName(false, 14, 14, L"clockP3", L"clock");
        addIcon(false, 14, 15, L"dragonFireball");
        addIcon(false, 15, 0, L"record_13");
        addIcon(false, 15, 1, L"record_cat");
        addIcon(false, 15, 2, L"record_blocks");
        addIcon(false, 15, 3, L"record_chirp");
        addIcon(false, 15, 4, L"record_far");
        addIcon(false, 15, 5, L"record_mall");
        addIcon(false, 15, 6, L"record_mellohi");
        addIcon(false, 15, 7, L"record_stal");
        addIcon(false, 15, 8, L"record_strad");
        addIcon(false, 15, 9, L"record_ward");
        addIcon(false, 15, 10, L"record_11");
        addIcon(false, 15, 11, L"record_where are we now");
        addIcon(false, 15, 12, L"bucketFish");
        addIcon(false, 15, 13, L"bucketSalmon");
        addIcon(false, 15, 14, L"bucketPuffer");
        addIcon(false, 15, 15, L"bucketTropical");
        addIcon(false, 16, 0, L"leather_horse_armor_detail");
        addIcon(false, 16, 7, L"kelp");
        addIcon(false, 16, 8, L"dried_kelp");
        addIcon(false, 16, 9, L"sea_pickle");
        addIcon(false, 16, 10, L"nautilus");
        addIcon(false, 16, 11, L"nautilus_core");
        addIcon(false, 16, 12, L"turtle_helmet");
        addIcon(false, 16, 13, L"turtle_shell_piece");
        addIcon(false, 16, 14, L"trident");
        addIcon(false, 16, 15, L"phantom_membrane");
        ClockAtlasSprite* clock = new ClockAtlasSprite();
        TextureAtlasSprite* previousClock = mSprites[L"clock"];
        clock->initUVs(previousClock->getU0(false), previousClock->getV0(false), previousClock->getU1(false),
                       previousClock->getV1(false));
        delete previousClock;
        mSprites[L"clock"] = clock;
        ClockAtlasSprite* clockP0 = new ClockAtlasSprite(0, clock);
        previousClock = mSprites[L"clockP0"];
        clockP0->initUVs(previousClock->getU0(false), previousClock->getV0(false), previousClock->getU1(false),
                         previousClock->getV1(false));
        delete previousClock;
        mSprites[L"clockP0"] = clockP0;
        ClockAtlasSprite* clockP1 = new ClockAtlasSprite(1, clock);
        previousClock = mSprites[L"clockP1"];
        clockP1->initUVs(previousClock->getU0(false), previousClock->getV0(false), previousClock->getU1(false),
                         previousClock->getV1(false));
        delete previousClock;
        mSprites[L"clockP1"] = clockP1;
        ClockAtlasSprite* clockP2 = new ClockAtlasSprite(2, clock);
        previousClock = mSprites[L"clockP2"];
        clockP2->initUVs(previousClock->getU0(false), previousClock->getV0(false), previousClock->getU1(false),
                         previousClock->getV1(false));
        delete previousClock;
        mSprites[L"clockP2"] = clockP2;
        ClockAtlasSprite* clockP3 = new ClockAtlasSprite(3, clock);
        previousClock = mSprites[L"clockP3"];
        clockP3->initUVs(previousClock->getU0(false), previousClock->getV0(false), previousClock->getU1(false),
                         previousClock->getV1(false));
        delete previousClock;
        mSprites[L"clockP3"] = clockP3;
        CompassAtlasSprite* compass = new CompassAtlasSprite();
        TextureAtlasSprite* previousCompass = mSprites[L"compass"];
        compass->initUVs(previousCompass->getU0(false), previousCompass->getV0(false), previousCompass->getU1(false),
                         previousCompass->getV1(false));
        delete previousCompass;
        mSprites[L"compass"] = compass;
        CompassAtlasSprite* compassP0 = new CompassAtlasSprite(0, compass);
        previousCompass = mSprites[L"compassP0"];
        compassP0->initUVs(previousCompass->getU0(false), previousCompass->getV0(false), previousCompass->getU1(false),
                           previousCompass->getV1(false));
        delete previousCompass;
        mSprites[L"compassP0"] = compassP0;
        CompassAtlasSprite* compassP1 = new CompassAtlasSprite(1, compass);
        previousCompass = mSprites[L"compassP1"];
        compassP1->initUVs(previousCompass->getU0(false), previousCompass->getV0(false), previousCompass->getU1(false),
                           previousCompass->getV1(false));
        delete previousCompass;
        mSprites[L"compassP1"] = compassP1;
        CompassAtlasSprite* compassP2 = new CompassAtlasSprite(2, compass);
        previousCompass = mSprites[L"compassP2"];
        compassP2->initUVs(previousCompass->getU0(false), previousCompass->getV0(false), previousCompass->getU1(false),
                           previousCompass->getV1(false));
        delete previousCompass;
        mSprites[L"compassP2"] = compassP2;
        CompassAtlasSprite* compassP3 = new CompassAtlasSprite(3, compass);
        previousCompass = mSprites[L"compassP3"];
        compassP3->initUVs(previousCompass->getU0(false), previousCompass->getV0(false), previousCompass->getU1(false),
                           previousCompass->getV1(false));
        delete previousCompass;
        mSprites[L"compassP3"] = compassP3;
    }
    else
    {
        addIcon(true, 0, 0, L"grass_top");
        mSprites[L"grass_top"]->setFlags(2);
        addIcon(true, 0, 1, L"stone");
        addIcon(true, 0, 2, L"dirt");
        addIcon(true, 0, 3, L"grass_side");
        mSprites[L"grass_side"]->setFlags(1);
        addIcon(true, 0, 4, L"planks_oak");
        addIcon(true, 0, 5, L"stoneslab_side");
        addIcon(true, 0, 6, L"stoneslab_top");
        addIcon(true, 0, 7, L"brick");
        addIcon(true, 0, 8, L"tnt_side");
        addIcon(true, 0, 9, L"tnt_top");
        addIcon(true, 0, 10, L"tnt_bottom");
        addIcon(true, 0, 11, L"web");
        addIcon(true, 0, 12, L"flower_rose");
        addIcon(true, 0, 13, L"flower_dandelion");
        addIcon(true, 0, 14, L"portal");
        addIcon(true, 0, 15, L"sapling");
        addIcon(true, 1, 0, L"cobblestone");
        addIcon(true, 1, 1, L"bedrock");
        addIcon(true, 1, 2, L"sand");
        addIcon(true, 1, 3, L"gravel");
        addIcon(true, 1, 4, L"log_oak");
        addIcon(true, 1, 5, L"log_oak_top");
        addIcon(true, 1, 6, L"iron_block");
        addIcon(true, 1, 7, L"gold_block");
        addIcon(true, 1, 8, L"diamond_block");
        addIcon(true, 1, 9, L"emerald_block");
        addIcon(true, 1, 10, L"redstone_block");
        addIcon(true, 1, 11, L"dropper_front_horizontal");
        addIcon(true, 1, 12, L"mushroom_red");
        addIcon(true, 1, 13, L"mushroom_brown");
        addIcon(true, 1, 14, L"sapling_jungle");
        addIcon(true, 1, 15, L"fire_0");
        addIcon(true, 2, 0, L"gold_ore");
        addIcon(true, 2, 1, L"iron_ore");
        addIcon(true, 2, 2, L"coal_ore");
        addIcon(true, 2, 3, L"bookshelf");
        addIcon(true, 2, 4, L"cobblestone_mossy");
        addIcon(true, 2, 5, L"obsidian");
        addIcon(true, 2, 6, L"grass_side_overlay");
        addIcon(true, 2, 7, L"tallgrass");
        addIcon(true, 2, 8, L"dispenser_front_vertical");
        addIcon(true, 2, 9, L"beacon");
        addIcon(true, 2, 10, L"dropper_front_vertical");
        addIcon(true, 2, 11, L"workbench_top");
        addIcon(true, 2, 12, L"furnace_front");
        addIcon(true, 2, 13, L"furnace_side");
        addIcon(true, 2, 14, L"dispenser_front");
        addIcon(true, 2, 15, L"fire_1");
        addIcon(true, 3, 0, L"sponge");
        addIcon(true, 3, 1, L"glass");
        addIcon(true, 3, 2, L"diamond_ore");
        addIcon(true, 3, 3, L"redstone_ore");
        addIcon(true, 3, 4, L"leaves");
        addIcon(true, 3, 5, L"leaves_opaque");
        addIcon(true, 3, 6, L"stonebrick");
        addIcon(true, 3, 7, L"deadbush");
        addIcon(true, 3, 8, L"fern");
        addIcon(true, 3, 9, L"daylight_detector_top");
        addIcon(true, 3, 10, L"daylight_detector_side");
        addIcon(true, 3, 11, L"workbench_side");
        addIcon(true, 3, 12, L"workbench_front");
        addIcon(true, 3, 13, L"furnace_front_lit");
        addIcon(true, 3, 14, L"furnace_top");
        addIcon(true, 3, 15, L"sapling_spruce");
        addIcon(true, 4, 0, L"wool_colored_white");
        addIcon(true, 4, 1, L"mob_spawner");
        addIcon(true, 4, 2, L"snow");
        addIcon(true, 4, 3, L"ice");
        addIcon(true, 4, 4, L"snow_side");
        addIcon(true, 4, 5, L"cactus_top");
        addIcon(true, 4, 6, L"cactus_side");
        addIcon(true, 4, 7, L"cactus_bottom");
        addIcon(true, 4, 8, L"clay");
        addIcon(true, 4, 9, L"reeds");
        addIcon(true, 4, 10, L"jukebox_side");
        addIcon(true, 4, 11, L"jukebox_top");
        addIcon(true, 4, 12, L"waterlily");
        addIcon(true, 4, 13, L"mycel_side");
        addIcon(true, 4, 14, L"mycel_top");
        addIcon(true, 4, 15, L"sapling_birch");
        addIcon(true, 5, 0, L"torch_on");
        addIcon(true, 5, 1, L"door_wood_upper");
        addIcon(true, 5, 2, L"door_iron_upper");
        addIcon(true, 5, 3, L"ladder");
        addIcon(true, 5, 4, L"trapdoor");
        addIcon(true, 5, 5, L"iron_bars");
        addIcon(true, 5, 6, L"farmland_wet");
        addIcon(true, 5, 7, L"farmland_dry");
        addIcon(true, 5, 8, L"crops_0");
        addIcon(true, 5, 9, L"crops_1");
        addIcon(true, 5, 10, L"crops_2");
        addIcon(true, 5, 11, L"crops_3");
        addIcon(true, 5, 12, L"crops_4");
        addIcon(true, 5, 13, L"crops_5");
        addIcon(true, 5, 14, L"crops_6");
        addIcon(true, 5, 15, L"crops_7");
        addIcon(true, 6, 0, L"lever");
        addIcon(true, 6, 1, L"door_wood_lower");
        addIcon(true, 6, 2, L"door_iron_lower");
        addIcon(true, 6, 3, L"redstone_torch_on");
        addIcon(true, 6, 4, L"stonebrick_mossy");
        addIcon(true, 6, 5, L"stonebrick_cracked");
        addIcon(true, 6, 6, L"pumpkin_top");
        addIcon(true, 6, 7, L"netherrack");
        addIcon(true, 6, 8, L"soul_sand");
        addIcon(true, 6, 9, L"glowstone");
        addIcon(true, 6, 10, L"piston_top_sticky");
        addIcon(true, 6, 11, L"piston_top");
        addIcon(true, 6, 12, L"piston_side");
        addIcon(true, 6, 13, L"piston_bottom");
        addIcon(true, 6, 14, L"piston_inner_top");
        addIcon(true, 6, 15, L"stem_straight");
        addIcon(true, 7, 0, L"rail_normal_turned");
        addIcon(true, 7, 1, L"wool_colored_black");
        addIcon(true, 7, 2, L"wool_colored_gray");
        addIcon(true, 7, 3, L"redstone_torch_off");
        addIcon(true, 7, 4, L"log_spruce");
        addIcon(true, 7, 5, L"log_birch");
        addIcon(true, 7, 6, L"pumpkin_side");
        addIcon(true, 7, 7, L"pumpkin_face_off");
        addIcon(true, 7, 8, L"pumpkin_face_on");
        addIcon(true, 7, 9, L"cake_top");
        addIcon(true, 7, 10, L"cake_side");
        addIcon(true, 7, 11, L"cake_inner");
        addIcon(true, 7, 12, L"cake_bottom");
        addIcon(true, 7, 13, L"mushroom_block_skin_red");
        addIcon(true, 7, 14, L"mushroom_block_skin_brown");
        addIcon(true, 7, 15, L"stem_bent");
        addIcon(true, 8, 0, L"rail_normal");
        addIcon(true, 8, 1, L"wool_colored_red");
        addIcon(true, 8, 2, L"wool_colored_pink");
        addIcon(true, 8, 3, L"repeater_off");
        addIcon(true, 8, 4, L"leaves_spruce");
        addIcon(true, 8, 5, L"leaves_spruce_opaque");
        addIcon(true, 8, 6, L"conduit_top");
        addIcon(true, 8, 7, L"turtle_egg_hatch_0");
        addIcon(true, 8, 8, L"melon_side");
        addIcon(true, 8, 9, L"melon_top");
        addIcon(true, 8, 10, L"cauldron_top");
        addIcon(true, 8, 11, L"cauldron_inner");
        addIcon(true, 8, 12, L"sponge_wet");
        addIcon(true, 8, 13, L"mushroom_block_skin_stem");
        addIcon(true, 8, 14, L"mushroom_block_inside");
        addIcon(true, 8, 15, L"vine");
        addIcon(true, 9, 0, L"lapis_block");
        addIcon(true, 9, 1, L"wool_colored_green");
        addIcon(true, 9, 2, L"wool_colored_lime");
        addIcon(true, 9, 3, L"repeater_on");
        addIcon(true, 9, 4, L"glass_pane_top");
        addIcon(true, 9, 5, L"chest_top");
        addIcon(true, 9, 6, L"ender_chest_top");
        addIcon(true, 9, 7, L"turtle_egg_hatch_1");
        addIcon(true, 9, 8, L"turtle_egg_hatch_2");
        addIcon(true, 9, 9, L"log_jungle");
        addIcon(true, 9, 10, L"cauldron_side");
        addIcon(true, 9, 11, L"cauldron_bottom");
        addIcon(true, 9, 12, L"brewing_stand_base");
        addIcon(true, 9, 13, L"brewing_stand");
        addIcon(true, 9, 14, L"endframe_top");
        addIcon(true, 9, 15, L"endframe_side");
        addIcon(true, 10, 0, L"lapis_ore");
        addIcon(true, 10, 1, L"wool_colored_brown");
        addIcon(true, 10, 2, L"wool_colored_yellow");
        addIcon(true, 10, 3, L"rail_golden");
        addIcon(true, 10, 4, L"redstone_dust_cross");
        addIcon(true, 10, 5, L"redstone_dust_line");
        addIcon(true, 10, 6, L"enchantment_top");
        addIcon(true, 10, 7, L"dragon_egg");
        addIcon(true, 10, 8, L"cocoa_2");
        addIcon(true, 10, 9, L"cocoa_1");
        addIcon(true, 10, 10, L"cocoa_0");
        addIcon(true, 10, 11, L"emerald_ore");
        addIcon(true, 10, 12, L"trip_wire_source");
        addIcon(true, 10, 13, L"trip_wire");
        addIcon(true, 10, 14, L"endframe_eye");
        addIcon(true, 10, 15, L"end_stone");
        addIcon(true, 11, 0, L"sandstone_top");
        addIcon(true, 11, 1, L"wool_colored_blue");
        addIcon(true, 11, 2, L"wool_colored_light_blue");
        addIcon(true, 11, 3, L"rail_golden_powered");
        addIcon(true, 11, 4, L"redstone_dust_cross_overlay");
        addIcon(true, 11, 5, L"redstone_dust_line_overlay");
        addIcon(true, 11, 6, L"enchantment_side");
        addIcon(true, 11, 7, L"enchantment_bottom");
        addIcon(true, 11, 8, L"diamondRing");
        addIcon(true, 11, 9, L"itemframe_back");
        addIcon(true, 11, 10, L"flower_pot");
        addIcon(true, 11, 11, L"comparator_off");
        addIcon(true, 11, 12, L"comparator_on");
        addIcon(true, 11, 13, L"rail_activator");
        addIcon(true, 11, 14, L"rail_activator_powered");
        addIcon(true, 11, 15, L"quartz_ore");
        addIcon(true, 12, 0, L"sandstone_side");
        addIcon(true, 12, 1, L"wool_colored_purple");
        addIcon(true, 12, 2, L"wool_colored_magenta");
        addIcon(true, 12, 3, L"detectorRail");
        addIcon(true, 12, 4, L"leaves_jungle");
        addIcon(true, 12, 5, L"leaves_jungle_opaque");
        addIcon(true, 12, 6, L"planks_spruce");
        addIcon(true, 12, 7, L"planks_jungle");
        addIcon(true, 12, 8, L"carrots_stage_0");
        addIcon(true, 12, 9, L"carrots_stage_1");
        addIcon(true, 12, 10, L"carrots_stage_2");
        addIcon(true, 12, 11, L"carrots_stage_3");
        addIcon(true, 12, 12, L"slime");
        addIcon(true, 12, 13, L"water");
        addIconSize(true, 12, 14, L"water_flow", 2, 2);
        addIcon(true, 13, 0, L"sandstone_bottom");
        addIcon(true, 13, 1, L"wool_colored_cyan");
        addIcon(true, 13, 2, L"wool_colored_orange");
        addIcon(true, 13, 3, L"redstoneLight");
        addIcon(true, 13, 4, L"redstoneLight_lit");
        addIcon(true, 13, 5, L"stonebrick_carved");
        addIcon(true, 13, 6, L"planks_birch");
        addIcon(true, 13, 7, L"anvil_base");
        addIcon(true, 13, 8, L"anvil_top_damaged_1");
        addIcon(true, 13, 9, L"quartz_block_chiseled_top");
        addIcon(true, 13, 10, L"quartz_block_lines_top");
        addIcon(true, 13, 11, L"quartz_block_top");
        addIcon(true, 13, 12, L"hopper_outside");
        addIcon(true, 13, 13, L"detectorRail_on");
        addIcon(true, 14, 0, L"nether_brick");
        addIcon(true, 14, 1, L"wool_colored_silver");
        addIcon(true, 14, 2, L"nether_wart_stage_0");
        addIcon(true, 14, 3, L"nether_wart_stage_1");
        addIcon(true, 14, 4, L"nether_wart_stage_2");
        addIcon(true, 14, 5, L"sandstone_carved");
        addIcon(true, 14, 6, L"sandstone_smooth");
        addIcon(true, 14, 7, L"anvil_top");
        addIcon(true, 14, 8, L"anvil_top_damaged_2");
        addIcon(true, 14, 9, L"quartz_block_chiseled");
        addIcon(true, 14, 10, L"quartz_block_lines");
        addIcon(true, 14, 11, L"quartz_block_side");
        addIcon(true, 14, 12, L"hopper_inside");
        addIcon(true, 14, 13, L"lava");
        addIconSize(true, 14, 14, L"lava_flow", 2, 2);
        addIcon(true, 15, 0, L"destroy_0");
        addIcon(true, 15, 1, L"destroy_1");
        addIcon(true, 15, 2, L"destroy_2");
        addIcon(true, 15, 3, L"destroy_3");
        addIcon(true, 15, 4, L"destroy_4");
        addIcon(true, 15, 5, L"destroy_5");
        addIcon(true, 15, 6, L"destroy_6");
        addIcon(true, 15, 7, L"destroy_7");
        addIcon(true, 15, 8, L"destroy_8");
        addIcon(true, 15, 9, L"destroy_9");
        addIcon(true, 15, 10, L"hay_block_side");
        addIcon(true, 15, 11, L"quartz_block_bottom");
        addIcon(true, 15, 12, L"hopper_top");
        addIcon(true, 15, 13, L"hay_block_top");
        addIcon(true, 16, 0, L"coal_block");
        addIcon(true, 16, 1, L"hardened_clay");
        addIcon(true, 16, 2, L"noteblock");
        addIcon(true, 16, 3, L"stone_andesite");
        addIcon(true, 16, 4, L"stone_andesite_smooth");
        addIcon(true, 16, 5, L"stone_diorite");
        addIcon(true, 16, 6, L"stone_diorite_smooth");
        addIcon(true, 16, 7, L"stone_granite");
        addIcon(true, 16, 8, L"stone_granite_smooth");
        addIcon(true, 16, 9, L"potatoes_stage_0");
        addIcon(true, 16, 10, L"potatoes_stage_1");
        addIcon(true, 16, 11, L"potatoes_stage_2");
        addIcon(true, 16, 12, L"potatoes_stage_3");
        addIcon(true, 16, 13, L"log_spruce_top");
        addIcon(true, 16, 14, L"log_jungle_top");
        addIcon(true, 16, 15, L"log_birch_top");
        addIcon(true, 17, 0, L"hardened_clay_stained_black");
        addIcon(true, 17, 1, L"hardened_clay_stained_blue");
        addIcon(true, 17, 2, L"hardened_clay_stained_brown");
        addIcon(true, 17, 3, L"hardened_clay_stained_cyan");
        addIcon(true, 17, 4, L"hardened_clay_stained_gray");
        addIcon(true, 17, 5, L"hardened_clay_stained_green");
        addIcon(true, 17, 6, L"hardened_clay_stained_light_blue");
        addIcon(true, 17, 7, L"hardened_clay_stained_lime");
        addIcon(true, 17, 8, L"hardened_clay_stained_magenta");
        addIcon(true, 17, 9, L"hardened_clay_stained_orange");
        addIcon(true, 17, 10, L"hardened_clay_stained_pink");
        addIcon(true, 17, 11, L"hardened_clay_stained_purple");
        addIcon(true, 17, 12, L"hardened_clay_stained_red");
        addIcon(true, 17, 13, L"hardened_clay_stained_silver");
        addIcon(true, 17, 14, L"hardened_clay_stained_white");
        addIcon(true, 17, 15, L"hardened_clay_stained_yellow");
        addIcon(true, 18, 0, L"glass_black");
        addIcon(true, 18, 1, L"glass_blue");
        addIcon(true, 18, 2, L"glass_brown");
        addIcon(true, 18, 3, L"glass_cyan");
        addIcon(true, 18, 4, L"glass_gray");
        addIcon(true, 18, 5, L"glass_green");
        addIcon(true, 18, 6, L"glass_light_blue");
        addIcon(true, 18, 7, L"glass_lime");
        addIcon(true, 18, 8, L"glass_magenta");
        addIcon(true, 18, 9, L"glass_orange");
        addIcon(true, 18, 10, L"glass_pink");
        addIcon(true, 18, 11, L"glass_purple");
        addIcon(true, 18, 12, L"glass_red");
        addIcon(true, 18, 13, L"glass_silver");
        addIcon(true, 18, 14, L"glass_white");
        addIcon(true, 18, 15, L"glass_yellow");
        addIcon(true, 19, 0, L"glass_pane_top_black");
        addIcon(true, 19, 1, L"glass_pane_top_blue");
        addIcon(true, 19, 2, L"glass_pane_top_brown");
        addIcon(true, 19, 3, L"glass_pane_top_cyan");
        addIcon(true, 19, 4, L"glass_pane_top_gray");
        addIcon(true, 19, 5, L"glass_pane_top_green");
        addIcon(true, 19, 6, L"glass_pane_top_light_blue");
        addIcon(true, 19, 7, L"glass_pane_top_lime");
        addIcon(true, 19, 8, L"glass_pane_top_magenta");
        addIcon(true, 19, 9, L"glass_pane_top_orange");
        addIcon(true, 19, 10, L"glass_pane_top_pink");
        addIcon(true, 19, 11, L"glass_pane_top_purple");
        addIcon(true, 19, 12, L"glass_pane_top_red");
        addIcon(true, 19, 13, L"glass_pane_top_silver");
        addIcon(true, 19, 14, L"glass_pane_top_white");
        addIcon(true, 19, 15, L"glass_pane_top_yellow");
        addIcon(true, 20, 0, L"double_plant_fern_top");
        addIcon(true, 20, 1, L"double_plant_grass_top");
        addIcon(true, 20, 2, L"double_plant_paeonia_top");
        addIcon(true, 20, 3, L"double_plant_rose_top");
        addIcon(true, 20, 4, L"double_plant_syringa_top");
        addIcon(true, 20, 5, L"flower_tulip_orange");
        addIcon(true, 20, 6, L"double_plant_sunflower_top");
        addIcon(true, 20, 7, L"double_plant_sunflower_front");
        addIcon(true, 20, 8, L"log_acacia");
        addIcon(true, 20, 9, L"log_acacia_top");
        addIcon(true, 20, 10, L"planks_acacia");
        addIcon(true, 20, 11, L"leaves_acacia");
        addIcon(true, 20, 12, L"leaves_acacia_fast");
        addIcon(true, 20, 13, L"prismarine_bricks");
        addIcon(true, 20, 14, L"red_sand");
        addIcon(true, 20, 15, L"red_sandstone_top");
        addIcon(true, 21, 0, L"double_plant_fern_bottom");
        addIcon(true, 21, 1, L"double_plant_grass_bottom");
        addIcon(true, 21, 2, L"double_plant_paeonia_bottom");
        addIcon(true, 21, 3, L"double_plant_rose_bottom");
        addIcon(true, 21, 4, L"double_plant_syringa_bottom");
        addIcon(true, 21, 5, L"flower_tulip_pink");
        addIcon(true, 21, 6, L"double_plant_sunflower_bottom");
        addIcon(true, 21, 7, L"double_plant_sunflower_back");
        addIcon(true, 21, 8, L"log_big_oak");
        addIcon(true, 21, 9, L"log_big_oak_top");
        addIcon(true, 21, 10, L"planks_big_oak");
        addIcon(true, 21, 11, L"leaves_big_oak");
        addIcon(true, 21, 12, L"leaves_big_oak_fast");
        addIcon(true, 21, 13, L"prismarine_dark");
        addIcon(true, 21, 14, L"red_sandstone_bottom");
        addIcon(true, 21, 15, L"red_sandstone_normal");
        addIcon(true, 22, 0, L"flower_alium");
        addIcon(true, 22, 1, L"flower_blue_orchid");
        addIcon(true, 22, 2, L"flower_houstonia");
        addIcon(true, 22, 3, L"flower_oxeye_daisy");
        addIcon(true, 22, 4, L"flower_tulip_red");
        addIcon(true, 22, 5, L"flower_tulip_white");
        addIcon(true, 22, 6, L"sapling_acacia");
        addIcon(true, 22, 7, L"sapling_roofed_oak");
        addIcon(true, 22, 8, L"coarse_dirt");
        addIcon(true, 22, 9, L"dirt_podzol_side");
        addIcon(true, 22, 10, L"dirt_podzol_top");
        addIcon(true, 22, 11, L"leaves_birch");
        addIcon(true, 22, 12, L"leaves_birch_fast");
        addIcon(true, 22, 13, L"prismarine_rough");
        addIcon(true, 22, 14, L"red_sandstone_carved");
        addIcon(true, 22, 15, L"red_sandstone_smooth");
        addIcon(true, 23, 0, L"door_acacia_upper");
        addIcon(true, 23, 1, L"door_birch_upper");
        addIcon(true, 23, 2, L"door_dark_oak_upper");
        addIcon(true, 23, 3, L"door_jungle_upper");
        addIcon(true, 23, 4, L"door_spruce_upper");
        addIcon(true, 23, 5, L"chorus_flower");
        addIcon(true, 23, 6, L"chorus_flower_dead");
        addIcon(true, 23, 7, L"chorus_flower_plant");
        addIcon(true, 23, 8, L"end_bricks");
        addIcon(true, 23, 9, L"grass_path_side");
        addIcon(true, 23, 10, L"grass_path_top");
        addIcon(true, 23, 11, L"barrier");
        addIcon(true, 23, 12, L"ice_packed");
        addIcon(true, 23, 13, L"sea_lantern");
        addIcon(true, 23, 14, L"daylight_detector_inverted_top");
        addIcon(true, 23, 15, L"iron_trapdoor");
        addIcon(true, 24, 0, L"door_acacia_lower");
        addIcon(true, 24, 1, L"door_birch_lower");
        addIcon(true, 24, 2, L"door_dark_oak_lower");
        addIcon(true, 24, 3, L"door_jungle_lower");
        addIcon(true, 24, 4, L"door_spruce_lower");
        addIcon(true, 24, 5, L"purpur_block");
        addIcon(true, 24, 6, L"purpur_pillar");
        addIcon(true, 24, 7, L"purpur_pillar_top");
        addIcon(true, 24, 8, L"end_rod");
        addIcon(true, 24, 9, L"magma");
        addIcon(true, 24, 10, L"nether_wart_block");
        addIcon(true, 24, 11, L"red_nether_brick");
        addIcon(true, 24, 12, L"frosted_ice_0");
        addIcon(true, 24, 13, L"frosted_ice_1");
        addIcon(true, 24, 14, L"frosted_ice_2");
        addIcon(true, 24, 15, L"frosted_ice_3");
        addIcon(true, 25, 0, L"beetroots_stage_0");
        addIcon(true, 25, 1, L"beetroots_stage_1");
        addIcon(true, 25, 2, L"beetroots_stage_2");
        addIcon(true, 25, 3, L"beetroots_stage_3");
        addIcon(true, 25, 4, L"chain_command_block_back");
        addIcon(true, 25, 5, L"chain_command_block_conditional");
        addIcon(true, 25, 6, L"chain_command_block_front");
        addIcon(true, 25, 7, L"chain_command_block_side");
        addIcon(true, 25, 8, L"command_block_back");
        addIcon(true, 25, 9, L"command_block_conditional");
        addIcon(true, 25, 10, L"command_block_front");
        addIcon(true, 25, 11, L"command_block_side");
        addIcon(true, 25, 12, L"repeating_command_block_back");
        addIcon(true, 25, 13, L"repeating_command_block_conditional");
        addIcon(true, 25, 14, L"repeating_command_block_front");
        addIcon(true, 25, 15, L"repeating_command_block_side");
        addIcon(true, 26, 0, L"bone_block_side");
        addIcon(true, 26, 1, L"bone_block_top");
        addIcon(true, 26, 2, L"unused");
        addIcon(true, 26, 3, L"unused");
        addIcon(true, 26, 4, L"observer_front");
        addIcon(true, 26, 5, L"observer_side");
        addIcon(true, 26, 6, L"observer_back");
        addIcon(true, 26, 7, L"observer_back_lit");
        addIcon(true, 26, 8, L"observer_top");
        addIcon(true, 26, 9, L"goldRing");
        addIcon(true, 26, 10, L"emeraldRing");
        addIcon(true, 26, 11, L"structure_block");
        addIcon(true, 26, 12, L"structure_block_corner");
        addIcon(true, 26, 13, L"structure_block_data");
        addIcon(true, 26, 14, L"structure_block_load");
        addIcon(true, 26, 15, L"structure_block_save");
        addIcon(true, 27, 0, L"concrete_black");
        addIcon(true, 27, 1, L"concrete_blue");
        addIcon(true, 27, 2, L"concrete_brown");
        addIcon(true, 27, 3, L"concrete_cyan");
        addIcon(true, 27, 4, L"concrete_gray");
        addIcon(true, 27, 5, L"concrete_green");
        addIcon(true, 27, 6, L"concrete_light_blue");
        addIcon(true, 27, 7, L"concrete_lime");
        addIcon(true, 27, 8, L"concrete_magenta");
        addIcon(true, 27, 9, L"concrete_orange");
        addIcon(true, 27, 10, L"concrete_pink");
        addIcon(true, 27, 11, L"concrete_purple");
        addIcon(true, 27, 12, L"concrete_red");
        addIcon(true, 27, 13, L"concrete_silver");
        addIcon(true, 27, 14, L"concrete_white");
        addIcon(true, 27, 15, L"concrete_yellow");
        addIcon(true, 28, 0, L"concrete_powder_black");
        addIcon(true, 28, 1, L"concrete_powder_blue");
        addIcon(true, 28, 2, L"concrete_powder_brown");
        addIcon(true, 28, 3, L"concrete_powder_cyan");
        addIcon(true, 28, 4, L"concrete_powder_gray");
        addIcon(true, 28, 5, L"concrete_powder_green");
        addIcon(true, 28, 6, L"concrete_powder_light_blue");
        addIcon(true, 28, 7, L"concrete_powder_lime");
        addIcon(true, 28, 8, L"concrete_powder_magenta");
        addIcon(true, 28, 9, L"concrete_powder_orange");
        addIcon(true, 28, 10, L"concrete_powder_pink");
        addIcon(true, 28, 11, L"concrete_powder_purple");
        addIcon(true, 28, 12, L"concrete_powder_red");
        addIcon(true, 28, 13, L"concrete_powder_silver");
        addIcon(true, 28, 14, L"concrete_powder_white");
        addIcon(true, 28, 15, L"concrete_powder_yellow");
        addIcon(true, 29, 0, L"glazed_terracotta_black");
        addIcon(true, 29, 1, L"glazed_terracotta_blue");
        addIcon(true, 29, 2, L"glazed_terracotta_brown");
        addIcon(true, 29, 3, L"glazed_terracotta_cyan");
        addIcon(true, 29, 4, L"glazed_terracotta_gray");
        addIcon(true, 29, 5, L"glazed_terracotta_green");
        addIcon(true, 29, 6, L"glazed_terracotta_light_blue");
        addIcon(true, 29, 7, L"glazed_terracotta_lime");
        addIcon(true, 29, 8, L"glazed_terracotta_magenta");
        addIcon(true, 29, 9, L"glazed_terracotta_orange");
        addIcon(true, 29, 10, L"glazed_terracotta_pink");
        addIcon(true, 29, 11, L"glazed_terracotta_purple");
        addIcon(true, 29, 12, L"glazed_terracotta_red");
        addIcon(true, 29, 13, L"glazed_terracotta_silver");
        addIcon(true, 29, 14, L"glazed_terracotta_white");
        addIcon(true, 29, 15, L"glazed_terracotta_yellow");
        addIcon(true, 30, 0, L"shulker_top");
        addIcon(true, 30, 2, L"cauldron_water");
        addIcon(true, 30, 3, L"seagrass_doubletall_top");
        addIcon(true, 30, 4, L"coral_blue");
        addIcon(true, 30, 5, L"coral_purple");
        addIcon(true, 30, 6, L"coral_pink");
        addIcon(true, 30, 7, L"coral_red");
        addIcon(true, 30, 8, L"coral_yellow");
        addIcon(true, 30, 9, L"coral_plant_blue");
        addIcon(true, 30, 10, L"coral_plant_purple");
        addIcon(true, 30, 11, L"coral_plant_pink");
        addIcon(true, 30, 12, L"coral_plant_red");
        addIcon(true, 30, 13, L"coral_plant_yellow");
        addIcon(true, 30, 14, L"sea_pickle");
        addIcon(true, 30, 15, L"blue_ice");
        addIcon(true, 31, 0, L"dried_kelp_top");
        addIcon(true, 31, 1, L"dried_kelp_side");
        addIcon(true, 31, 2, L"seagrass_carried");
        addIcon(true, 31, 3, L"seagrass_doubletall_bottom");
        addIcon(true, 31, 4, L"coral_blue_dead");
        addIcon(true, 31, 5, L"coral_purple_dead");
        addIcon(true, 31, 6, L"coral_pink_dead");
        addIcon(true, 31, 7, L"coral_red_dead");
        addIcon(true, 31, 8, L"coral_yellow_dead");
        addIcon(true, 31, 9, L"coral_fan_blue");
        addIcon(true, 31, 10, L"coral_fan_purple");
        addIcon(true, 31, 11, L"coral_fan_pink");
        addIcon(true, 31, 12, L"coral_fan_red");
        addIcon(true, 31, 13, L"coral_fan_yellow");
        addIcon(true, 31, 14, L"coral_layered_top_red");
        addIcon(true, 31, 15, L"coral_layered_top_yellow");
        addIcon(true, 32, 0, L"kelp_a");
        addIconWithName(true, 32, 1, L"kelp_b", L"kelp_a");
        addIconWithName(true, 32, 2, L"kelp_c", L"kelp_a");
        addIconWithName(true, 32, 3, L"kelp_d", L"kelp_a");
        addIcon(true, 32, 4, L"kelp_top_a");
        addIconWithName(true, 32, 5, L"kelp_top_b", L"kelp_top_a");
        addIconWithName(true, 32, 6, L"kelp_top_c", L"kelp_top_a");
        addIconWithName(true, 32, 7, L"kelp_top_d", L"kelp_top_a");
        addIcon(true, 32, 8, L"seagrass");
        addIcon(true, 32, 9, L"coral_fan_blue_dead");
        addIcon(true, 32, 10, L"coral_fan_purple_dead");
        addIcon(true, 32, 11, L"coral_fan_pink_dead");
        addIcon(true, 32, 12, L"coral_fan_red_dead");
        addIcon(true, 32, 13, L"coral_fan_yellow_dead");
        addIcon(true, 32, 15, L"spruce_trapdoor");
        addIcon(true, 33, 0, L"stripped_log_oak");
        addIcon(true, 33, 1, L"stripped_log_oak_top");
        addIcon(true, 33, 2, L"stripped_log_acacia");
        addIcon(true, 33, 3, L"stripped_log_acacia_top");
        addIcon(true, 33, 4, L"stripped_log_birch");
        addIcon(true, 33, 5, L"stripped_log_birch_top");
        addIcon(true, 33, 6, L"stripped_log_dark_oak");
        addIcon(true, 33, 7, L"stripped_log_dark_oak_top");
        addIcon(true, 33, 8, L"stripped_log_jungle");
        addIcon(true, 33, 9, L"stripped_log_jungle_top");
        addIcon(true, 33, 10, L"stripped_log_spruce");
        addIcon(true, 33, 11, L"stripped_log_spruce_top");
        addIcon(true, 33, 12, L"acacia_trapdoor");
        addIcon(true, 33, 13, L"birch_trapdoor");
        addIcon(true, 33, 14, L"dark_oak_trapdoor");
        addIcon(true, 33, 15, L"jungle_trapdoor");
    }
#ifdef MINECRAFT_PC
    AddModSprites();
#endif
}

#ifdef MINECRAFT_PC
void TextureAtlas::AddModSprites()
{
    const std::vector<std::wstring> sprites = PcModContentAtlasSprites(mIconType);
    if (sprites.empty())
        return;
    const int columns = 16;
    const int baseRows = mIconType == 0 ? 34 : 17;
    const int rows = baseRows + static_cast<int>((sprites.size() + columns - 1) / columns);
    const float scale = static_cast<float>(baseRows) / static_cast<float>(rows);
    for (boost::unordered_map<std::wstring, TextureAtlasSprite*>::iterator it = mSprites.begin(); it != mSprites.end();
         ++it)
    {
        TextureAtlasSprite* sprite = it->second;
        sprite->initUVs(sprite->getU0(false), sprite->getV0(false) * scale, sprite->getU1(false),
                        sprite->getV1(false) * scale);
    }
    for (size_t index = 0; index < sprites.size(); ++index)
    {
        const int row = baseRows + static_cast<int>(index) / columns;
        const int column = static_cast<int>(index) % columns;
        mSprites[sprites[index]] = new TextureAtlasSprite(
            sprites[index], sprites[index], static_cast<float>(column) / columns, static_cast<float>(row) / rows,
            static_cast<float>(column + 1) / columns, static_cast<float>(row + 1) / rows, 256.0f,
            static_cast<float>(rows * 16));
    }
}
#endif

void TextureAtlas::makeTextureAnimated(TexturePack* pack, TextureAtlasSprite* sprite)
{
    if (sprite->hasOwnData() == 0)
    {
        mAnimatedSprites.push_back(sprite);
        return;
    }
    std::wstring parent = sprite->mParent;
    std::wstring animation = pack->getAnimationString(parent, sprite->getName(), mPath, true);
#ifdef MINECRAFT_PC
    if (!animation.empty())
        PcGoldenAgeAnimationFrames(mPath + parent, &animation);
#endif
    if (!animation.empty())
    {
        std::wstring textureName = mPath + parent + mSpritePrefix;
        std::vector<Texture*>* textures = TextureManager::getInstance()->createTextures(textureName, mSingleImage);
        if (textures == 0 || textures->empty())
            return;

        Texture* texture = textures->at(0);
        sprite->init(mStitchedTexture, textures, sprite->getX(), sprite->getY(), static_cast<int>(texture->getWidth()),
                     static_cast<int>(texture->getHeight()), false);
        if (textures->size() > 1)
        {
            mAnimatedSprites.push_back(sprite);
            sprite->loadAnimationFrames(animation);
        }
    }
}

void TextureAtlas::stitch()
{
    for (std::vector<TextureAtlasSprite*>::iterator it = mAnimatedSprites.begin(); it != mAnimatedSprites.end(); ++it)
        (*it)->freeFrameTextures();
    loadUVs();
    if (mIconType == 0)
    {
        for (unsigned int id = 1; id < 4096; ++id)
            if (Block::existsWithId(id))
                Block::byId(id)->registerIcons(this);
        GlideRingGenerator::registerIcons(this);
        Minecraft::GetInstance()->mLevelRenderer->registerTextures(this);
        static_cast<ItemFrameRenderer*>(
            Minecraft::GetInstance()->getEntityRenderDispatcher()->getRenderer(EntityType::ITEM_FRAME))
            ->registerTerrainTextures(this);
    }
    for (unsigned int id = 0; id < 32000; ++id)
    {
        if (!Item::existsWithId(id))
            continue;
        Item* item = Item::byId(id);
        if (item != 0 && item->getIconType() == mIconType)
            item->registerIcons(this);
    }
    boost::unordered_map<TextureHolder*, std::vector<Texture*>*> textureMap;
    mAnimatedSprites.clear();
    const std::wstring resourceName = mName + mSpritePrefix;
    TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    MemSect(32);
    std::wstring path(L"");
    BufferedImage* image;
    int height;
    int width;
    if (pack->hasFile(L"res/" + resourceName, false))
    {
        path = pack->getPath(true, 0);
        image = pack->getImageResource(resourceName, false, true, path);
        MemSect(0);
        height = image->getHeight();
        width = image->getWidth();
    }
    else
    {
        path = Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack->getPath(true, 0);
        pack = Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack;
        image = pack->getImageResource(resourceName, false, true, path);
        MemSect(0);
        height = image->getHeight();
        width = image->getWidth();
    }
#ifdef MINECRAFT_PC
    PcModContentPatchAtlas(mIconType, image);
    height = image->getHeight();
    PcGoldenAgePatchAtlas(mIconType, image);
#endif
    if (mStitchedTexture != 0)
    {
        TextureManager::getInstance()->unregisterTexture(mName, mStitchedTexture);
        delete mStitchedTexture;
    }
    mStitchedTexture = TextureManager::getInstance()->createTexture(mName, 1, width, height, 0, mSingleImage);
    mStitchedTexture->transferFromImage(image);
    delete image;
    TextureManager::getInstance()->registerName(mName, mStitchedTexture);
    for (boost::unordered_map<std::wstring, TextureAtlasSprite*>::iterator it = mSprites.begin(); it != mSprites.end();
         ++it)
    {
        TextureAtlasSprite* sprite = it->second;
        int x = (int)(sprite->getU0(false) * mStitchedTexture->getWidth());
        int y = (int)(sprite->getV0(false) * mStitchedTexture->getHeight());
        int width = (int)(sprite->getU1(false) * mStitchedTexture->getWidth()) - x;
        int height = (int)(sprite->getV1(false) * mStitchedTexture->getHeight()) - y;
        sprite->init(mStitchedTexture, 0, x, y, width, height, false);
    }
    MemSect(52);
    for (boost::unordered_map<std::wstring, TextureAtlasSprite*>::iterator it = mSprites.begin(); it != mSprites.end();
         ++it)
        makeTextureAnimated(pack, it->second);
    MemSect(0);
    mStitchedTexture->writeAsPNG(L"debug.stitched_" + mName + L".png");
    mStitchedTexture->updateOnGPU();
}

void TextureAtlas::freeStitch()
{
    if (mStitchedTexture != 0)
    {
        TextureManager::getInstance()->unregisterTexture(mName, mStitchedTexture);
        delete mStitchedTexture;
    }
    mStitchedTexture = 0;
}

void TextureAtlas::cycleAnimationFrames()
{
    for (std::vector<TextureAtlasSprite*>::iterator it = mAnimatedSprites.begin(); it != mAnimatedSprites.end(); ++it)
        (*it)->cycleFrames();
}

Texture* TextureAtlas::getStitchedTexture()
{
    return mStitchedTexture;
}

TextureAtlasSprite* TextureAtlas::registerIcon(const std::wstring& name)
{
    TextureAtlasSprite* sprite = 0;
    if (name.empty())
        sprite = mMissingIcon;
    boost::unordered_map<std::wstring, TextureAtlasSprite*>::iterator it = mSprites.find(name);
    if (it != mSprites.end())
        sprite = it->second;
    if (sprite == 0)
        sprite = mMissingIcon;
    return sprite;
}

int TextureAtlas::getIconType()
{
    return mIconType;
}

TextureAtlasSprite* TextureAtlas::getMissingIcon()
{
    return mMissingIcon;
}

#include "game/client/render/Textures.hpp"

#include <cstring>
#include <cwchar>
#include <vector>

#include "game/core/BufferedImage.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/Options.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/client/render/MemTextureProcessor.hpp"
#include "game/client/render/MemTexture.hpp"
#include "game/client/render/MobSkinMemTextureProcessor.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Texture.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/client/render/TextureAtlas.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

static int sIndexedTextures[256];
ResourceLocation BLOCKS_LOCATION(TN_BLOCK_ATLAS);
ResourceLocation ITEM_LOCATION(TN_ITEM_ATLAS);
static C4JRender::eTextureFormat sTextureFormat = C4JRender::eTextureFormat_RGBA8;
static bool sTextureMipmap = true;

Textures::Textures(TexturePackRepository* texturePackRepository, Options* options)
    : mOptions(options),
      mTexturePackRepository(texturePackRepository)
{
    InitializeCriticalSection(&mMutex);
    mMissingImage = new BufferedImage(16, 16, 0);
    mBlocks = new TextureAtlas(0, L"terrain", L"textures/blocks/", mMissingImage, true);
    mItems = new TextureAtlas(1, L"items", L"textures/items/", mMissingImage, true);
    loadIndexedTextures();
}

bool Textures::IsTUImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    static _TEXTURE_NAME textureIds[] = {
        TN_ARMOR_POWER,
        TN_MOB_ENDERMAN_EYES,
        TN_MISC_EXPLOSION,
        TN_MOB_ZOMBIE,
        TN_MISC_FOOTPRINT,
        TN_MOB_REDCOW,
        TN_MOB_SNOWMAN,
        TN_MOB_ENDERDRAGON_ENDER,
        TN_MOB_VILLAGER_VILLAGER,
        TN_MOB_VILLAGER_FARMER,
        TN_MOB_VILLAGER_LIBRARIAN,
        TN_MOB_VILLAGER_PRIEST,
        TN_MOB_VILLAGER_SMITH,
        TN_MOB_VILLAGER_BUTCHER,
        TN_MOB_ENDERDRAGON_ENDER_EYES,
        TN_BLUR_MISC_GLINT,
        TN_ITEM_BOOK,
        TN_MISC_PARTICLEFIELD,
        TN_MISC_TUNNEL,
        TN_MOB_ENDERDRAGON_BEAM,
        TN_ITEM_ATLAS,
        TN_BLOCK_ATLAS,
        TN_MISC_MAPICONS,
        TN_MOB_SKELETON_WITHER,
        TN_ITEM_ENDERCHEST,
        TN_ART_KZ,
        TN_MOB_WOLF_TAME,
        TN_MOB_WOLF_COLLAR,
        TN_PARTICLES,
        TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_VILLAGER,
        TN_ITEM_LEAD_KNOT,
        TN_MISC_BEACON_BEAM,
        TN_MOB_BAT,
        TN_MOB_HORSE_DONKEY,
        TN_MOB_HORSE_HORSE_BLACK,
        TN_MOB_HORSE_HORSE_BROWN,
        TN_MOB_HORSE_HORSE_CHESTNUT,
        TN_MOB_HORSE_HORSE_CREAMY,
        TN_MOB_HORSE_HORSE_DARKBROWN,
        TN_MOB_HORSE_HORSE_GRAY,
        TN_MOB_HORSE_HORSE_MARKINGS_BLACKDOTS,
        TN_MOB_HORSE_HORSE_MARKINGS_WHITE,
        TN_MOB_HORSE_HORSE_MARKINGS_WHITEDOTS,
        TN_MOB_HORSE_HORSE_MARKINGS_WHITEFIELD,
        TN_MOB_HORSE_HORSE_SKELETON,
        TN_MOB_HORSE_HORSE_WHITE,
        TN_MOB_HORSE_HORSE_ZOMBIE,
        TN_MOB_HORSE_MULE,
        TN_MOB_HORSE_ARMOR_HORSE_ARMOR_DIAMOND,
        TN_MOB_HORSE_ARMOR_HORSE_ARMOR_GOLD,
        TN_MOB_HORSE_ARMOR_HORSE_ARMOR_IRON,
        TN_MOB_WITCH,
        TN_MOB_WITHER_WITHER,
        TN_MOB_WITHER_WITHER_ARMOR,
        TN_MOB_WITHER_WITHER_INVULNERABLE,
        TN_ITEM_TRAPPED,
        TN_ITEM_TRAPPED_DOUBLE,
        TN_MOB_STEVE,
        TN_MOB_ALEX,
        TN_MOB_ALEX1,
        TN_MOB_ALEX2,
        TN_MOB_ALEX3,
        TN_MOB_ALEX4,
        TN_MOB_ALEX5,
        TN_MOB_ALEX6,
        TN_MOB_ALEX7,
        TN_MISC_ADDITIONALMAPICONS,
        TN_FONT_DEFAULT,
        TN_ITEM_ARMORSTAND_WOOD,
        TN_MOB_ENDERMITE,
        TN_MOB_GUARDIAN,
        TN_MOB_GUARDIAN_ELDER,
        TN_MOB_GUARDIAN_BEAM,
        TN_MOB_RABBIT_BROWN,
        TN_MOB_RABBIT_WHITE,
        TN_MOB_RABBIT_BLACK,
        TN_MOB_RABBIT_GOLD,
        TN_MOB_RABBIT_SALT,
        TN_MOB_RABBIT_WHITE_SPLOTCHED,
        TN_MOB_RABBIT_TOAST,
        TN_MOB_RABBIT_CAERBANNOG,
        TN_TEXTURES_DESTROY_STAGE_0,
        TN_TEXTURES_DESTROY_STAGE_1,
        TN_TEXTURES_DESTROY_STAGE_2,
        TN_TEXTURES_DESTROY_STAGE_3,
        TN_TEXTURES_DESTROY_STAGE_4,
        TN_TEXTURES_DESTROY_STAGE_5,
        TN_TEXTURES_DESTROY_STAGE_6,
        TN_TEXTURES_DESTROY_STAGE_7,
        TN_TEXTURES_DESTROY_STAGE_8,
        TN_TEXTURES_DESTROY_STAGE_9,
        TN_ITEM_BANNER_BANNER_BASE,
        TN_ITEM_BANNER_BANNER_ATLAS,
        TN_MOB_BEAR_POLARBEAR,
        TN_ITEM_BOAT_BOAT_OAK,
        TN_ITEM_BOAT_BOAT_SPRUCE,
        TN_ITEM_BOAT_BOAT_BIRCH,
        TN_ITEM_BOAT_BOAT_JUNGLE,
        TN_ITEM_BOAT_BOAT_ACACIA,
        TN_ITEM_BOAT_BOAT_DARKOAK,
        TN_MOB_SKELETON,
        TN_MOB_SKELETON_STRAY,
        TN_MOB_SKELETON_STRAY_OVERLAY,
        TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_FARMER,
        TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_LIBRARIAN,
        TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_PRIEST,
        TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_SMITH,
        TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_BUTCHER,
        TN_MOB_ZOMBIE_HUSK,
        TN_MOB_ENDERDRAGON_DRAGON_FIREBALL,
        TN_MOB_SHULKER_ENDERGOLEM,
        TN_MOB_SHULKER_SPARK,
        TN_ITEM_SPECTRAL_ARROW,
        TN_ITEM_TIPPED_ARROW,
        TN_ITEM_ELYTRA,
        TN_MOB_DEVSTEVE,
        TN_MOB_DEVALEX,
        TN_ENVIRONMENT_THERMAL_1,
        TN_ENVIRONMENT_THERMAL_2,
        TN_ENVIRONMENT_BOOSTARROW,
        TN_MOB_GLIDE_GHOST,
        TN_MOB_LLAMA_DECOR_DECOR_WHITE,
        TN_MOB_LLAMA_DECOR_DECOR_ORANGE,
        TN_MOB_LLAMA_DECOR_DECOR_MAGENTA,
        TN_MOB_LLAMA_DECOR_DECOR_LIGHT_BLUE,
        TN_MOB_LLAMA_DECOR_DECOR_YELLOW,
        TN_MOB_LLAMA_DECOR_DECOR_LIME,
        TN_MOB_LLAMA_DECOR_DECOR_PINK,
        TN_MOB_LLAMA_DECOR_DECOR_GRAY,
        TN_MOB_LLAMA_DECOR_DECOR_SILVER,
        TN_MOB_LLAMA_DECOR_DECOR_CYAN,
        TN_MOB_LLAMA_DECOR_DECOR_PURPLE,
        TN_MOB_LLAMA_DECOR_DECOR_BLUE,
        TN_MOB_LLAMA_DECOR_DECOR_BROWN,
        TN_MOB_LLAMA_DECOR_DECOR_GREEN,
        TN_MOB_LLAMA_DECOR_DECOR_RED,
        TN_MOB_LLAMA_DECOR_DECOR_BLACK,
        TN_MOB_LLAMA_LLAMA_CREAMY,
        TN_MOB_LLAMA_LLAMA_WHITE,
        TN_MOB_LLAMA_LLAMA_BROWN,
        TN_MOB_LLAMA_LLAMA_GRAY,
        TN_MOB_LLAMA_SPIT,
        TN_MOB_ILLAGER_EVOKER,
        TN_MOB_ILLAGER_FANGS,
        TN_MOB_ILLAGER_VINDICATOR,
        TN_MOB_ILLAGER_VEX,
        TN_MOB_ILLAGER_VEX_CHARGING,
        TN_MOB_ILLAGER_ILLUSIONIST,
        TN_MOB_PARROT_PARROT_RED_BLUE,
        TN_MOB_PARROT_PARROT_BLUE,
        TN_MOB_PARROT_PARROT_GREEN,
        TN_MOB_PARROT_PARROT_YELLOW_BLUE,
        TN_MOB_PARROT_PARROT_GREY,
        TN_ITEM_BED,
        TN_ITEM_FIREWORK,
        TN_MOB_HORSE_ARMOR_HORSE_ARMOR_LEATHER_1,
        TN_MOB_HORSE_ARMOR_HORSE_ARMOR_LEATHER_1_B,
        TN_MOB_FISH_COD,
        TN_MOB_FISH_PUFFERFISH,
        TN_MOB_FISH_SALMON,
        TN_MOB_FISH_TROPICAL_A,
        TN_MOB_FISH_TROPICAL_A_PATTERN_1,
        TN_MOB_FISH_TROPICAL_A_PATTERN_2,
        TN_MOB_FISH_TROPICAL_A_PATTERN_3,
        TN_MOB_FISH_TROPICAL_A_PATTERN_4,
        TN_MOB_FISH_TROPICAL_A_PATTERN_5,
        TN_MOB_FISH_TROPICAL_A_PATTERN_6,
        TN_MOB_FISH_TROPICAL_B,
        TN_MOB_FISH_TROPICAL_B_PATTERN_1,
        TN_MOB_FISH_TROPICAL_B_PATTERN_2,
        TN_MOB_FISH_TROPICAL_B_PATTERN_3,
        TN_MOB_FISH_TROPICAL_B_PATTERN_4,
        TN_MOB_FISH_TROPICAL_B_PATTERN_5,
        TN_MOB_FISH_TROPICAL_B_PATTERN_6,
        TN_MOB_SEA_TURTLE,
        TN_MOB_DOLPHIN,
        TN_ITEM_TRIDENT,
        TN_MOB_ZOMBIE_DROWNED,
        TN_ITEM_TRIDENT_RIPTIDE,
        TN_ITEM_CONDUIT_CONDUIT_BASE,
        TN_ITEM_CONDUIT_CONDUIT_CAGE,
        TN_ITEM_CONDUIT_CONDUIT_WIND_HORIZONTAL,
        TN_ITEM_CONDUIT_CONDUIT_WIND_VERTICAL,
        TN_ITEM_CONDUIT_CONDUIT_CLOSED,
        TN_ITEM_CONDUIT_CONDUIT_OPEN,
        TN_MOB_PHANTOM,
        TN_UNINDEXED,
    };
    static const wchar_t* textureNames[] = {
        L"font/Default",    L"font/Mojangles_7", L"font/Mojangles_11",
        L"armor/cloth_1",   L"armor/cloth_1_b",  L"armor/cloth_2",
        L"armor/cloth_2_b", L"armor/turtle_1",   NULL,
    };
    if (textureName < TN_UNINDEXED)
    {
        for (_TEXTURE_NAME* entry = textureIds; *entry < TN_UNINDEXED; ++entry)
        {
            if (textureName == *entry)
                return true;
        }
    }
    for (const wchar_t** entry = textureNames; *entry != NULL; ++entry)
    {
        if (name == *entry)
            return true;
    }
    return false;
}

bool Textures::IsOriginalImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    static _TEXTURE_NAME textureIds[] = {
        TN_MOB_CHAR1, TN_MOB_CHAR2,  TN_MOB_CHAR3,       TN_MOB_CHAR4,         TN_MOB_CHAR5, TN_MOB_CHAR6,
        TN_MOB_CHAR7, TN_MISC_MAPBG, TN_MISC_GRASSCOLOR, TN_MISC_FOLIAGECOLOR, TN_UNINDEXED,
    };
    static const wchar_t* textureNames[] = {
        L"misc/watercolor.png",
        NULL,
    };
    if (textureName < TN_UNINDEXED)
    {
        for (_TEXTURE_NAME* entry = textureIds; *entry < TN_UNINDEXED; ++entry)
        {
            if (textureName == *entry)
                return true;
        }
    }
    for (const wchar_t** entry = textureNames; *entry != NULL; ++entry)
    {
        if (name == *entry)
            return true;
    }
    return false;
}

BufferedImage* Textures::readImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
#ifdef MINECRAFT_PC
    BufferedImage* goldenAge = PcGoldenAgeReadImage(name);
    if (goldenAge != NULL)
        return goldenAge;
#endif
    MemSect(32);
    bool titleUpdate = IsTUImage(textureName, name);
    std::wstring path = L"";
    bool useSelected = mTexturePackRepository->getSelected() != TexturePackRepository::defaultTexturePack
        && mTexturePackRepository->getSelected()->hasFile(L"res/" + name, false);
    BufferedImage* image;
    if (useSelected)
    {
        path = mTexturePackRepository->getSelected()->getPath(titleUpdate, NULL);
        image = mTexturePackRepository->getSelected()->getImageResource(name, false, titleUpdate, path);
    }
    else
    {
        path = TexturePackRepository::defaultTexturePack->getPath(titleUpdate, NULL);
        if (!IsOriginalImage(textureName, name) && !titleUpdate)
        {
            image = TexturePackRepository::defaultTexturePack->getImageResource(L"1_2_2/" + name, false, false, path);
        }
        else
        {
            image = TexturePackRepository::defaultTexturePack->getImageResource(name, false, titleUpdate, path);
        }
    }
    MemSect(0);
    return image;
}

void Textures::setTextureFormat(const std::wstring& name)
{
    sTextureFormat = C4JRender::eTextureFormat_RGBA8;
}

arrayWithLength<int> Textures::anaglyph(arrayWithLength<int> pixels)
{
    arrayWithLength<int> result(pixels.length, true);
    unsigned int i;
    for (i = 0; i < pixels.length; ++i)
    {
        unsigned int value = (unsigned int)pixels[i];
        int alpha = (int)((value >> 24) & 255);
        int red = (int)((value >> 16) & 255);
        int green = (int)((value >> 8) & 255);
        int blue = (int)(value & 255);
        int newRed = (red * 30 + green * 59 + blue * 11) / 100;
        int newGreen = (red * 30 + green * 70) / 100;
        int newBlue = (red * 30 + blue * 70) / 100;
        result[i] = (alpha << 24) | (newRed << 16) | (newGreen << 8) | newBlue;
    }
    delete[] pixels.data;
    return result;
}

void Textures::loadTexture(BufferedImage* image, int texture, bool blur, bool clamp)
{
    MemSect(0x21);
    GlStateManager::bindTexture(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    if (blur)
    {
        GlStateManager::texParameteri(1, 1);
        GlStateManager::texParameteri(2, 1);
    }
    if (clamp)
    {
        GlStateManager::texParameteri(3, 0);
        GlStateManager::texParameteri(4, 0);
    }
    else
    {
        GlStateManager::texParameteri(3, 1);
        GlStateManager::texParameteri(4, 1);
    }
    int width = (int)image->getWidth();
    int height = (int)image->getHeight();
    arrayWithLength<int> pixels((unsigned int)(width * height), true);
    image->getRGB(0, 0, width, height, pixels, 0, width, 0);
    if (mOptions != NULL && mOptions->mAnaglyph3d)
    {
        pixels = anaglyph(pixels);
    }
    unsigned int byteCount = (unsigned int)(width * height) * 4;
    arrayWithLength<unsigned char> bytes(byteCount, true);
    for (unsigned int i = 0; i < pixels.length; ++i)
    {
        unsigned int value = (unsigned int)pixels.data[i];
        bytes.data[i * 4] = (unsigned char)(value >> 16);
        bytes.data[i * 4 + 1] = (unsigned char)(value >> 8);
        bytes.data[i * 4 + 2] = (unsigned char)value;
        bytes.data[i * 4 + 3] = (unsigned char)(value >> 24);
    }
    ByteBuffer* buffer = MemoryTracker::createByteBuffer((int)byteCount);
    buffer->clear();
    buffer->put(bytes);
    buffer->position(0)->limit(byteCount);
    delete[] pixels.data;
    delete[] bytes.data;
    int levels = 1;
    if (sTextureMipmap)
    {
        int widthLevels = 1;
        int heightLevels = 1;
        if (width > 16)
        {
            do
            {
                ++widthLevels;
            } while ((8 << widthLevels) < width);
        }
        if (height > 16)
        {
            do
            {
                ++heightLevels;
            } while ((8 << heightLevels) < height);
        }
        levels = heightLevels < widthLevels ? heightLevels : widthLevels;
        if (levels > 5)
            levels = 5;
        gRenderer->TextureSetTextureLevels(levels);
    }
    gRenderer->TextureData(width, height, buffer->getBuffer(), 0, sTextureFormat);
    if (sTextureMipmap)
    {
        for (int level = 1; level < levels; ++level)
        {
            int previousWidth = width >> (level - 1);
            int levelWidth = width >> level;
            int levelHeight = height >> level;
            unsigned int levelCount = levelWidth * levelHeight;
            unsigned int* levelPixels = new unsigned int[levelCount];
            if (image->getData(level) == NULL)
            {
                for (int x = 0; x < levelWidth; ++x)
                {
                    for (int y = 0; y < levelHeight; ++y)
                    {
                        unsigned int topLeft = buffer->getInt((y * 2 * previousWidth + x * 2) * 4);
                        unsigned int topRight = buffer->getInt((y * 2 * previousWidth + x * 2 + 1) * 4);
                        unsigned int bottomRight = buffer->getInt(((y * 2 + 1) * previousWidth + x * 2 + 1) * 4);
                        unsigned int bottomLeft = buffer->getInt(((y * 2 + 1) * previousWidth + x * 2) * 4);
                        unsigned int bottom = Texture::crispBlend((int)((bottomRight << 24) | (bottomRight >> 8)),
                                                                  (int)((bottomLeft << 24) | (bottomLeft >> 8)));
                        unsigned int top = Texture::crispBlend((int)((topLeft << 24) | (topLeft >> 8)),
                                                               (int)((topRight << 24) | (topRight >> 8)));
                        unsigned int blended = Texture::crispBlend((int)top, (int)bottom);
                        levelPixels[y * levelWidth + x] = (blended << 8) | (blended >> 24);
                    }
                }
            }
            else
            {
                memcpy(levelPixels, image->getData(level), levelCount * 4);
                for (unsigned int i = 0; i < levelCount; ++i)
                {
                    levelPixels[i] = (levelPixels[i] << 8) | (levelPixels[i] >> 24);
                }
            }
            for (int x = 0; x < levelWidth; ++x)
            {
                for (int y = 0; y < levelHeight; ++y)
                {
                    buffer->putInt((y * levelWidth + x) * 4, (int)levelPixels[y * levelWidth + x]);
                }
            }
            delete[] levelPixels;
            gRenderer->TextureData(levelWidth, levelHeight, buffer->getBuffer(), level, sTextureFormat);
        }
    }
    delete buffer;
    MemSect(0);
}

int Textures::loadTexture(_TEXTURE_NAME textureName, const std::wstring& name)
{
    bool loaded = find(name) != end();
    int texture = (*this)[name];
    if (loaded)
        return texture;
    std::wstring workingName = name;
    if (name == L"environment/clouds" || name == L"%clamp%misc/shadow" || name == L"%blur%misc/pumpkinblur"
        || name == L"%clamp%misc/shadow" || name == L"gui/icons" || name == L"gui/gui" || name == L"misc/footprint"
        || name == L"mob/shulker/endergolem" || name == L"item/bed" || name == L"font/Default")
    {
        sTextureMipmap = false;
    }
    if (wcsncmp(name.c_str(), L"armor/", 6) == 0 && name != L"armor/power")
    {
        sTextureMipmap = false;
    }
    setTextureFormat(name);
    texture = MemoryTracker::genTextures();
    std::wstring tag = L"%blur%";
    bool blur = name.substr(0, tag.length()) == tag;
    if (blur)
        workingName = name.substr(6);
    tag = L"%clamp%";
    bool clamp = name.substr(0, tag.length()) == tag;
    if (clamp)
        workingName = name.substr(7);
    BufferedImage* image = readImage(textureName, workingName);
    if (image->isMipmapDisabled())
        sTextureMipmap = false;
    if ((int)image->getHeight() > 0 && (int)image->getWidth() > 0)
    {
        loadTexture(image, texture, blur, clamp);
    }
    delete image;
    (*this)[name] = texture;
    sTextureFormat = C4JRender::eTextureFormat_RGBA8;
    sTextureMipmap = true;
    return texture;
}

void Textures::loadIndexedTextures()
{
    static const wchar_t* textureNames[] = {
        L"misc/grasscolor",
        L"misc/foliagecolor",
        L"%blur%misc/pumpkinblur",
        L"%clamp%misc/shadow",
        L"art/kz",
        L"environment/clouds",
        L"environment/rain",
        L"environment/snow",
        L"gui/gui",
        L"gui/icons",
        L"item/arrows",
        L"item/cart",
        L"item/sign",
        L"misc/mapbg",
        L"misc/mapicons",
        L"misc/water",
        L"misc/footprint",
        L"mob/saddle",
        L"mob/sheep_fur",
        L"mob/spider_eyes",
        L"particles",
        L"mob/chicken",
        L"mob/cow",
        L"mob/pig",
        L"mob/sheep",
        L"mob/squid",
        L"mob/wolf",
        L"mob/wolf_tame",
        L"mob/wolf_angry",
        L"mob/creeper",
        L"mob/ghast",
        L"mob/ghast_fire",
        L"mob/zombie",
        L"mob/pigzombie",
        L"mob/skeleton",
        L"mob/slime",
        L"mob/spider",
        L"mob/steve",
        L"mob/char1",
        L"mob/char2",
        L"mob/char3",
        L"mob/char4",
        L"mob/char5",
        L"mob/char6",
        L"mob/char7",
        L"terrain/moon",
        L"terrain/sun",
        L"armor/power",
        L"mob/cavespider",
        L"mob/enderman",
        L"mob/silverfish",
        L"mob/enderman_eyes",
        L"misc/explosion",
        L"item/xporb",
        L"item/chest",
        L"item/largechest",
        L"item/enderchest",
        L"mob/redcow",
        L"mob/snowman",
        L"mob/enderdragon/ender",
        L"mob/fire",
        L"mob/lava",
        L"mob/villager/villager",
        L"mob/villager/farmer",
        L"mob/villager/librarian",
        L"mob/villager/priest",
        L"mob/villager/smith",
        L"mob/villager/butcher",
        L"mob/enderdragon/crystal",
        L"mob/enderdragon/shuffle",
        L"mob/enderdragon/beam",
        L"mob/enderdragon/ender_eyes",
        L"%blur%misc/glint",
        L"item/book",
        L"misc/tunnel",
        L"misc/particlefield",
        L"terrain/moon_phases",
        L"mob/ozelot",
        L"mob/cat_black",
        L"mob/cat_red",
        L"mob/cat_siamese",
        L"mob/villager_golem",
        L"mob/skeleton_wither",
        L"mob/wolf_collar",
        L"mob/zombie_villager/zombie_villager",
        L"item/lead_knot",
        L"misc/beacon_beam",
        L"mob/bat",
        L"mob/horse/donkey",
        L"mob/horse/horse_black",
        L"mob/horse/horse_brown",
        L"mob/horse/horse_chestnut",
        L"mob/horse/horse_creamy",
        L"mob/horse/horse_darkbrown",
        L"mob/horse/horse_gray",
        L"mob/horse/horse_markings_blackdots",
        L"mob/horse/horse_markings_white",
        L"mob/horse/horse_markings_whitedots",
        L"mob/horse/horse_markings_whitefield",
        L"mob/horse/horse_skeleton",
        L"mob/horse/horse_white",
        L"mob/horse/horse_zombie",
        L"mob/horse/mule",
        L"mob/horse/armor/horse_armor_diamond",
        L"mob/horse/armor/horse_armor_gold",
        L"mob/horse/armor/horse_armor_iron",
        L"mob/witch",
        L"mob/wither/wither",
        L"mob/wither/wither_armor",
        L"mob/wither/wither_invulnerable",
        L"item/trapped",
        L"item/trapped_double",
        L"mob/alex",
        L"mob/alex1",
        L"mob/alex2",
        L"mob/alex3",
        L"mob/alex4",
        L"mob/alex5",
        L"mob/alex6",
        L"mob/alex7",
        L"misc/additionalmapicons",
        L"item/armorstand/wood",
        L"font/Default",
        L"font/alternate",
        L"mob/endermite",
        L"mob/guardian",
        L"mob/guardian_elder",
        L"mob/guardian_beam",
        L"mob/rabbit/brown",
        L"mob/rabbit/white",
        L"mob/rabbit/black",
        L"mob/rabbit/gold",
        L"mob/rabbit/salt",
        L"mob/rabbit/white_splotched",
        L"mob/rabbit/toast",
        L"mob/rabbit/caerbannog",
        L"textures/destroy_stage_0",
        L"textures/destroy_stage_1",
        L"textures/destroy_stage_2",
        L"textures/destroy_stage_3",
        L"textures/destroy_stage_4",
        L"textures/destroy_stage_5",
        L"textures/destroy_stage_6",
        L"textures/destroy_stage_7",
        L"textures/destroy_stage_8",
        L"textures/destroy_stage_9",
        L"item/banner/banner_base",
        L"item/banner/Banner_Atlas",
        L"mob/bear/polarbear",
        L"item/boat/boat_oak",
        L"item/boat/boat_spruce",
        L"item/boat/boat_birch",
        L"item/boat/boat_jungle",
        L"item/boat/boat_acacia",
        L"item/boat/boat_darkoak",
        L"mob/skeleton/stray",
        L"mob/skeleton/stray_overlay",
        L"mob/zombie_villager/zombie_farmer",
        L"mob/zombie_villager/zombie_librarian",
        L"mob/zombie_villager/zombie_priest",
        L"mob/zombie_villager/zombie_smith",
        L"mob/zombie_villager/zombie_butcher",
        L"mob/zombie/husk",
        L"mob/enderdragon/dragon_fireball",
        L"mob/shulker/endergolem",
        L"mob/shulker/spark",
        L"item/spectral_arrow",
        L"item/tipped_arrow",
        L"item/elytra",
        L"mob/DevSteve",
        L"mob/DevAlex",
        L"environment/Thermal_1",
        L"environment/Thermal_2",
        L"environment/BoostArrow",
        L"mob/Glide_Ghost",
        L"mob/llama/decor/decor_white",
        L"mob/llama/decor/decor_orange",
        L"mob/llama/decor/decor_magenta",
        L"mob/llama/decor/decor_light_blue",
        L"mob/llama/decor/decor_yellow",
        L"mob/llama/decor/decor_lime",
        L"mob/llama/decor/decor_pink",
        L"mob/llama/decor/decor_gray",
        L"mob/llama/decor/decor_silver",
        L"mob/llama/decor/decor_cyan",
        L"mob/llama/decor/decor_purple",
        L"mob/llama/decor/decor_blue",
        L"mob/llama/decor/decor_brown",
        L"mob/llama/decor/decor_green",
        L"mob/llama/decor/decor_red",
        L"mob/llama/decor/decor_black",
        L"mob/llama/llama_creamy",
        L"mob/llama/llama_white",
        L"mob/llama/llama_brown",
        L"mob/llama/llama_gray",
        L"mob/llama/spit",
        L"mob/illager/evoker",
        L"mob/illager/fangs",
        L"mob/illager/vindicator",
        L"mob/illager/vex",
        L"mob/illager/vex_charging",
        L"mob/illager/illusionist",
        L"mob/parrot/parrot_red_blue",
        L"mob/parrot/parrot_blue",
        L"mob/parrot/parrot_green",
        L"mob/parrot/parrot_yellow_blue",
        L"mob/parrot/parrot_grey",
        L"item/bed",
        L"item/firework",
        L"mob/horse/armor/horse_armor_leather_1",
        L"mob/horse/armor/horse_armor_leather_1_b",
        L"mob/fish/cod",
        L"mob/fish/pufferfish",
        L"mob/fish/salmon",
        L"mob/fish/tropical_a",
        L"mob/fish/tropical_a_pattern_1",
        L"mob/fish/tropical_a_pattern_2",
        L"mob/fish/tropical_a_pattern_3",
        L"mob/fish/tropical_a_pattern_4",
        L"mob/fish/tropical_a_pattern_5",
        L"mob/fish/tropical_a_pattern_6",
        L"mob/fish/tropical_b",
        L"mob/fish/tropical_b_pattern_1",
        L"mob/fish/tropical_b_pattern_2",
        L"mob/fish/tropical_b_pattern_3",
        L"mob/fish/tropical_b_pattern_4",
        L"mob/fish/tropical_b_pattern_5",
        L"mob/fish/tropical_b_pattern_6",
        L"mob/sea_turtle",
        L"mob/dolphin",
        L"item/trident",
        L"mob/zombie/drowned",
        L"item/trident_riptide",
        L"item/conduit/conduit_base",
        L"item/conduit/conduit_cage",
        L"item/conduit/conduit_wind_horizontal",
        L"item/conduit/conduit_wind_vertical",
        L"item/conduit/conduit_closed",
        L"item/conduit/conduit_open",
        L"mob/phantom",
    };
    for (unsigned int i = 0; i < sizeof(textureNames) / sizeof(textureNames[0]); ++i)
    {
        sIndexedTextures[i] = loadTexture(static_cast<_TEXTURE_NAME>(i), textureNames[i]);
    }
}

arrayWithLength<int> Textures::loadTexturePixels(BufferedImage* image, arrayWithLength<int> pixels)
{
    int width = (int)image->getWidth();
    int height = (int)image->getHeight();
    image->getRGB(0, 0, width, height, pixels, 0, width, 0);
    return pixels;
}

arrayWithLength<int> Textures::loadTexturePixels(BufferedImage* image)
{
    arrayWithLength<int> pixels(image->getWidth() * image->getHeight(), true);
    return loadTexturePixels(image, pixels);
}

arrayWithLength<int> Textures::loadTexturePixels(_TEXTURE_NAME textureName, const std::wstring& name)
{
    arrayWithLength<int> cached = mTexturePixels[name];
    if (cached.data != NULL)
        return cached;
    arrayWithLength<int> pixels;
    BufferedImage* image = readImage(textureName, name);
    pixels = loadTexturePixels(image);
    delete image;
    mTexturePixels[name] = pixels;
    return pixels;
}

void Textures::bind(int texture)
{
    if (texture >= 0)
        GlStateManager::bindTexture(texture);
}

void Textures::bindTexture(const std::wstring& name)
{
    bind(loadTexture(TN_UNINDEXED, name));
}

int Textures::loadTexture(int textureName)
{
    if (textureName == -1)
        return 0;
    if (textureName == 0xf1)
    {
        mBlocks->getStitchedTexture()->bind(0);
        return (int)mBlocks->getStitchedTexture()->getGlId();
    }
    if (textureName == 0xf0)
    {
        mItems->getStitchedTexture()->bind(0);
        return (int)mItems->getStitchedTexture()->getGlId();
    }
    return sIndexedTextures[textureName];
}

void Textures::bindTexture(const ResourceLocation* location)
{
    if (location->isPreloaded())
        bind(loadTexture(location->getTexture()));
    else
        bind(loadTexture(TN_UNINDEXED, location->getPath()));
}

void Textures::bindTextureLayers(const ResourceLocation* location)
{
    int count = location->getTextureCount();
    int i;
    for (i = 0; i < count; ++i)
    {
        gRenderer->TextureBind(i, loadTexture(location->getTexture(i)));
    }
}

ResourceLocation* Textures::getTextureLocation(int textureName)
{
    switch (textureName)
    {
    case 0:
        return &BLOCKS_LOCATION;
    case 1:
        return &ITEM_LOCATION;
    }
    return NULL;
}

ResourceLocation* Textures::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<ItemEntity> item = boost::dynamic_pointer_cast<ItemEntity>(entity);
    not_null_ptr<ItemInstance> instance = item->getItem();
    return getTextureLocation(instance->getItem()->getIconType());
}

void Textures::clearLastBoundId()
{
    mLastBoundId = -1;
}

void Textures::loadTexture(BufferedImage* image, int texture)
{
    loadTexture(image, texture, false, false);
}

int Textures::getTexture(BufferedImage* image, C4JRender::eTextureFormat format, bool mipmap)
{
    int texture = MemoryTracker::genTextures();
    sTextureFormat = format;
    sTextureMipmap = mipmap;
    loadTexture(image, texture);
    sTextureFormat = C4JRender::eTextureFormat_RGBA8;
    sTextureMipmap = true;
    mTextureImages[texture] = image;
    return texture;
}

void Textures::replaceTexture(arrayWithLength<int> pixels, int width, int height, int texture)
{
    bind(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
    if (mOptions != NULL && mOptions->mAnaglyph3d)
    {
        pixels = anaglyph(pixels);
    }
    unsigned int byteCount = (unsigned int)(width * height * 4);
    arrayWithLength<unsigned char> bytes(byteCount, true);
    for (unsigned int i = 0; i < pixels.length; ++i)
    {
        unsigned int value = (unsigned int)pixels.data[i];
        int alpha = (int)((value >> 24) & 255);
        int red = (int)((value >> 16) & 255);
        int green = (int)((value >> 8) & 255);
        int blue = (int)(value & 255);
        if (mOptions != NULL && mOptions->mAnaglyph3d)
        {
            int newRed = (red * 30 + green * 59 + blue * 11) / 100;
            int newGreen = (red * 30 + green * 70) / 100;
            int newBlue = (red * 30 + blue * 70) / 100;
            red = newRed;
            green = newGreen;
            blue = newBlue;
        }
        bytes.data[i * 4] = (unsigned char)red;
        bytes.data[i * 4 + 1] = (unsigned char)green;
        bytes.data[i * 4 + 2] = (unsigned char)blue;
        bytes.data[i * 4 + 3] = (unsigned char)alpha;
    }
    ByteBuffer* buffer = MemoryTracker::createByteBuffer((int)byteCount);
    buffer->put(bytes);
    buffer->position(0)->limit(byteCount);
    delete[] bytes.data;
    gRenderer->TextureDataUpdate(0, 0, width, height, buffer->getBuffer(), 0);
    delete buffer;
}

void Textures::replaceTextureDirect(arrayWithLength<int> pixels, int width, int height, int texture)
{
    GlStateManager::bindTexture(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    std::vector<unsigned int> nativePixels(pixels.length);
    for (unsigned int i = 0; i < pixels.length; ++i)
    {
        unsigned int value = static_cast<unsigned int>(pixels.data[i]);
        nativePixels[i] = (value >> 24) | ((value >> 8) & 0x0000ff00u) | ((value << 8) & 0x00ff0000u) | (value << 24);
    }
    gRenderer->TextureDataUpdate(0, 0, width, height, nativePixels.data(), 0);
#else
    gRenderer->TextureDataUpdate(0, 0, width, height, pixels.data, 0);
#endif
}

void Textures::replaceTextureDirect(arrayWithLength<short> pixels, int width, int height, int texture)
{
    GlStateManager::bindTexture(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
    gRenderer->TextureDataUpdate(0, 0, width, height, pixels.data, 0);
}

void Textures::releaseTexture(int texture)
{
    mTextureImages.erase(texture);
    GlStateManager::deleteTexture(texture);
}

intptr_t Textures::addMemTexture(const std::wstring& name, MemTextureProcessor* processor)
{
    MemTexture* texture = NULL;
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found == mMemTextures.end() || (texture = found->second) == NULL)
    {
        unsigned char* data = NULL;
        unsigned int size = 0;
        gApp.GetMemFileDetails(name, &data, &size);
        if (size == 0)
        {
            mMemTextures[name] = NULL;
            LeaveCriticalSection(&mMutex);
        }
        else
        {
            texture = new MemTexture(name, data, size, processor);
            mMemTextures[name] = texture;
            LeaveCriticalSection(&mMutex);
        }
    }
    else
    {
        texture->m_referenceCount = texture->m_referenceCount + 1;
        LeaveCriticalSection(&mMutex);
    }
    delete processor;
    return (intptr_t)texture;
}

int Textures::loadMemTexture(const std::wstring& name, const std::wstring& path)
{
    MemTexture* texture = NULL;
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end())
        texture = found->second;
    LeaveCriticalSection(&mMutex);
    if (texture == NULL)
    {
        if (!gApp.IsFileInMemoryTextures(name))
        {
            return path.empty() ? -1 : loadTexture(TN_UNINDEXED, path);
        }
        texture = (MemTexture*)addMemTexture(name, new MobSkinMemTextureProcessor());
        if (texture == NULL)
            return path.empty() ? -1 : loadTexture(TN_UNINDEXED, path);
    }
    MemTexture& fields = *texture;
    fields.m_textureFormat = 0;
    int result;
    if (fields.m_image == NULL || fields.m_loaded)
    {
        result = fields.m_textureId;
    }
    else
    {
        if (name.substr(0, 7) == L"dlcskin" || name.substr(0, 7) == L"dlccape")
            sTextureMipmap = false;
        if (fields.m_textureId < 0)
        {
            result = getTexture(fields.m_image, C4JRender::eTextureFormat_RGBA8, sTextureMipmap);
            fields.m_textureId = result;
            fields.m_loaded = true;
            sTextureMipmap = true;
        }
        else
        {
            loadTexture(fields.m_image, fields.m_textureId);
            result = fields.m_textureId;
            sTextureMipmap = true;
            fields.m_loaded = true;
        }
    }
    if (result >= 0)
        return result;
    return path.empty() ? -1 : loadTexture(TN_UNINDEXED, path);
}

int Textures::loadMemTexture(const std::wstring& name, int texture)
{
    MemTexture* memTex = NULL;
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end())
        memTex = found->second;
    LeaveCriticalSection(&mMutex);
    if (memTex == NULL)
    {
        if (!gApp.IsFileInMemoryTextures(name))
            return loadTexture(texture);
        memTex = (MemTexture*)addMemTexture(name, new MobSkinMemTextureProcessor());
        if (memTex == NULL)
            return loadTexture(texture);
    }
    MemTexture& fields = *memTex;
    fields.m_textureFormat = 0;
    int result;
    if (fields.m_image == NULL || fields.m_loaded)
    {
        result = fields.m_textureId;
    }
    else
    {
        if (name.substr(0, 7) == L"dlcskin" || name.substr(0, 7) == L"dlccape")
            sTextureMipmap = false;
        if (fields.m_textureId < 0)
        {
            result = getTexture(fields.m_image, C4JRender::eTextureFormat_RGBA8, sTextureMipmap);
            fields.m_textureId = result;
            fields.m_loaded = true;
            sTextureMipmap = true;
        }
        else
        {
            loadTexture(fields.m_image, fields.m_textureId);
            result = fields.m_textureId;
            sTextureMipmap = true;
            fields.m_loaded = true;
        }
    }
    if (result >= 0)
        return result;
    return loadTexture(texture);
}

void Textures::removeMemTexture(const std::wstring& name)
{
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end())
    {
        MemTexture* texture = found->second;
        if (texture == NULL)
        {
            mMemTextures.erase(name);
        }
        else
        {
            MemTexture& fields = *texture;
            fields.m_referenceCount = fields.m_referenceCount - 1;
            if (fields.m_referenceCount == 0)
            {
                if (fields.m_textureId >= 0)
                    releaseTexture(fields.m_textureId);
                mMemTextures.erase(name);
                delete texture;
            }
        }
    }
    LeaveCriticalSection(&mMutex);
}

void Textures::tick(bool force, bool unused, bool reload)
{
    boost::unordered_map<std::wstring, MemTexture*>::iterator it;

    if (unused != mReloading)
    {
        mReloading = unused;
        for (it = mMemTextures.begin(); it != mMemTextures.end(); ++it)
        {
            if (it->second != NULL)
            {
                it->second->m_textureReady = mReloading;
            }
        }
    }
    MemSect(0x16);

    if (reload)
    {
        if (!force)
        {
            MemSect(0);
            return;
        }
        mBlocks->cycleAnimationFrames();
        mItems->cycleAnimationFrames();
    }

    EnterCriticalSection(&mMutex);
    it = mMemTextures.begin();
    while (it != mMemTextures.end())
    {
        MemTexture* texture = it->second;
        if (texture != NULL && !texture->m_textureReady && ++texture->m_textureFormat > 20)
        {
            if (texture->m_textureId >= 0)
                releaseTexture(texture->m_textureId);
            delete texture;
            it = mMemTextures.erase(it);
        }
        else
        {
            ++it;
        }
    }
    LeaveCriticalSection(&mMutex);
    MemSect(0);
}

void Textures::freeStitch()
{
    mBlocks->freeStitch();
    mItems->freeStitch();
}

void Textures::stitch()
{
    mBlocks->stitch();
    mItems->stitch();
}

void Textures::reloadAll()
{
    for (iterator it = begin(); it != end(); ++it)
    {
        releaseTexture(it->second);
    }
    clear();
    mTextureImages.clear();
    freeStitch();
    loadIndexedTextures();
    mTexturePixels.clear();
    GrassColor::init(loadTexturePixels(TN_MISC_GRASSCOLOR, L"misc/grasscolor"));
    FoliageColor::init(loadTexturePixels(TN_MISC_FOLIAGECOLOR, L"misc/foliagecolor"));
    stitch();
    mTexturePackRepository->clearInvalidTexturePacks();
}

TextureAtlasSprite* Textures::getMissingIcon(int atlas)
{
    if (atlas != 0)
        return mItems->getMissingIcon();
    return mBlocks->getMissingIcon();
}

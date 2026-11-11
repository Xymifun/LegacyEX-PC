#include "platform/pc/PcModContent.hpp"

#include "platform/pc/PcMods.hpp"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcScriptsInternal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <deque>
#include <map>
#include <set>

#include "game/client/audio/SoundType.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/TextureUtil.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/FoodItem.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Recipe.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ShapedRecipe.hpp"
#include "game/item/ShapelessRecipe.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Material.hpp"

using namespace PcScripts;

namespace
{
const int STRING_BASE = 0x7FD00000;
const int STRING_MASK = 0xFFF00000;
const int FIRST_ITEM_ID = 466;
const int LAST_ITEM_ID = 1023;
const int ATLAS_COLUMNS = 16;
const int BASE_ROWS[2] = {34, 17};
const int BLOCK_ID_LIMIT = 4096;
const size_t MAX_TEXTURE_SIZE = 4 * 1024 * 1024;

struct Ref
{
    std::string name;
    int data;
    bool anyData;
    Ref() : data(0), anyData(false) {}
};

struct BlockDef
{
    std::string mod;
    std::string path;
    std::string key;
    int nameId;
    int descriptionId;
    int textures[3];
    std::string material;
    std::string sound;
    float hardness;
    float resistance;
    int light;
    bool dropsNothing;
    Ref drop;
    bool hasDrop;
    int dropMin;
    int dropMax;
    PcModTab tab;
    int id;
    Block* block;
    void* (*make)(int id);
};

struct ItemDef
{
    std::string mod;
    std::string path;
    std::string key;
    int nameId;
    int descriptionId;
    int texture;
    int stack;
    PcModTab tab;
    bool food;
    int hunger;
    float saturation;
    bool meat;
    Item* item;
    void* (*make)(int id);
};

struct RecipeDef
{
    std::string mod;
    bool shaped;
    Ref result;
    int count;
    std::vector<std::string> rows;
    std::map<char, Ref> key;
    std::vector<Ref> ingredients;
    int group;
};

struct SmeltDef
{
    std::string mod;
    Ref input;
    Ref result;
    int count;
    float experience;
};

struct AtlasEntry
{
    std::wstring sprite;
    std::string file;
};

bool defsLoaded = false;
std::vector<BlockDef> modBlocks;
std::vector<ItemDef> modItems;
std::vector<RecipeDef> modRecipes;
std::vector<SmeltDef> modSmelts;
std::deque<std::wstring> modStrings;
std::deque<AtlasEntry> atlasEntries[2];

std::wstring Widen(const std::string& s)
{
    return Wide(s.data(), s.size());
}

const PcMod* FindMod(const std::string& id)
{
    const std::vector<PcMod>& list = PcModsLoaded();
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (list[i].id == id)
            return &list[i];
    }
    return 0;
}

bool ValidPath(const char* path, bool slashes)
{
    if (*path == 0 || strlen(path) > 64)
        return false;
    char prev = '/';
    for (const char* c = path; *c != 0; ++c)
    {
        const bool ok = (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '_' || *c == '-'
                        || (slashes && *c == '/' && prev != '/');
        if (!ok)
            return false;
        prev = *c;
    }
    return prev != '/';
}

int AddString(const std::string& s)
{
    modStrings.push_back(Widen(s));
    return STRING_BASE | (int)(modStrings.size() - 1);
}

int AddTexture(int atlas, const std::string& mod, const std::string& tex)
{
    const std::wstring sprite = Widen("pcmod:" + mod + "/" + tex);
    for (size_t i = 0; i < atlasEntries[atlas].size(); ++i)
    {
        if (atlasEntries[atlas][i].sprite == sprite)
            return (int)i;
    }
    AtlasEntry e;
    e.sprite = sprite;
    const PcMod* owner = FindMod(mod);
    e.file = (owner != 0 ? owner->directory : std::string("mods/" + mod)) + "/textures/" + tex + ".png";
    atlasEntries[atlas].push_back(e);
    return (int)(atlasEntries[atlas].size() - 1);
}

PcModTab TabByName(const char* name)
{
    static const char* const names[] = {
        "none",
        "building",
        "decoration",
        "redstone",
        "transport",
        "materials",
        "food",
        "tools",
        "brewing",
        "misc"};
    for (int i = 0; i < (int)(sizeof(names) / sizeof(names[0])); ++i)
    {
        if (strcmp(names[i], name) == 0)
            return static_cast<PcModTab>(i);
    }
    return static_cast<PcModTab>(-1);
}

int GroupByName(const char* name)
{
    static const char* const names[] = {
        "structures",
        "tools",
        "food",
        "armour",
        "mechanisms",
        "vehicles",
        "misc"};
    for (int i = 0; i < (int)(sizeof(names) / sizeof(names[0])); ++i)
    {
        if (strcmp(names[i], name) == 0)
            return i;
    }
    return -1;
}

const char* const materialNames[] = {
    "stone",
    "metal",
    "wood",
    "dirt",
    "grass",
    "sand",
    "clay",
    "wool"};
const char* const soundNames[] = {
    "stone",
    "wood",
    "gravel",
    "grass",
    "metal",
    "glass",
    "wool",
    "sand",
    "snow"};

bool InList(const char* s, const char* const* list, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (strcmp(s, list[i]) == 0)
            return true;
    }
    return false;
}

const char* FieldString(lua_State* L, const char* what, const char* key, bool required)
{
    lua_getfield(L, 1, key);
    if (lua_isnil(L, -1))
    {
        if (required)
            luaL_error(L, "%s needs \"%s\"", what, key);
        return 0;
    }
    if (lua_type(L, -1) != LUA_TSTRING)
        luaL_error(L, "%s: \"%s\" must be a string", what, key);
    return lua_tostring(L, -1);
}

double FieldNumber(lua_State* L, const char* what, const char* key, double dflt, double lo, double hi)
{
    lua_getfield(L, 1, key);
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        return dflt;
    }
    if (lua_type(L, -1) != LUA_TNUMBER)
        luaL_error(L, "%s: \"%s\" must be a number", what, key);
    const double v = lua_tonumber(L, -1);
    if (!(v >= lo && v <= hi))
        luaL_error(L, "%s: \"%s\" must be from %f to %f", what, key, lo, hi);
    lua_pop(L, 1);
    return v;
}

bool CheckRef(const char* s)
{
    const char* at = strchr(s, '@');
    const size_t nameLen = at != 0 ? (size_t)(at - s) : strlen(s);
    if (nameLen == 0 || nameLen > 140)
        return false;
    int colons = 0;
    for (size_t i = 0; i < nameLen; ++i)
    {
        const char c = s[i];
        if (c == ':')
            ++colons;
        else if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == '/'))
            return false;
    }
    if (colons > 1 || s[0] == ':' || s[nameLen - 1] == ':')
        return false;
    if (at == 0)
        return true;
    if (strcmp(at + 1, "*") == 0)
        return true;
    char* stop = 0;
    const long d = strtol(at + 1, &stop, 10);
    return stop != at + 1 && *stop == 0 && d >= 0 && d <= 32767;
}

Ref ParseRef(const char* s, const std::string& mod)
{
    Ref r;
    const char* at = strchr(s, '@');
    r.name.assign(s, at != 0 ? (size_t)(at - s) : strlen(s));
    if (at != 0 && strcmp(at + 1, "*") == 0)
        r.anyData = true;
    else if (at != 0)
        r.data = (int)strtol(at + 1, 0, 10);
    if (r.name.find(':') == std::string::npos)
        r.name = "?" + mod + ":" + r.name;
    return r;
}

void ResolveNamespace(Ref* r)
{
    if (r->name.empty() || r->name[0] != '?')
        return;
    const std::string own = r->name.substr(1);
    const std::string path = own.substr(own.find(':') + 1);
    bool found = false;
    for (size_t i = 0; i < modBlocks.size() && !found; ++i)
        found = modBlocks[i].key == own;
    for (size_t i = 0; i < modItems.size() && !found; ++i)
        found = modItems[i].key == own;
    r->name = found ? own : "minecraft:" + path;
}

const char* FieldRef(lua_State* L, const char* what, const char* key, bool required)
{
    const char* s = FieldString(L, what, key, required);
    if (s != 0 && !CheckRef(s))
        luaL_error(L, "%s: \"%s\" is not a block or item name (\"name\", \"mod:name\", \"name@data\")", what, s);
    return s;
}

bool Declared(const std::string& key)
{
    for (size_t i = 0; i < modBlocks.size(); ++i)
    {
        if (modBlocks[i].key == key)
            return true;
    }
    for (size_t i = 0; i < modItems.size(); ++i)
    {
        if (modItems[i].key == key)
            return true;
    }
    return false;
}

void CheckNewId(lua_State* L, const char* what, const char* path)
{
    if (!ValidPath(path, false))
        luaL_error(L, "%s: an id is 1 to 64 lowercase letters, digits, '_' or '-' (not \"%s\")", what, path);
    bool dup;
    {
        const std::string key = StateOf(L)->modId + ":" + path;
        dup = Declared(key);
    }
    if (dup)
        luaL_error(L, "%s: \"%s\" is declared twice", what, path);
}

void CheckTexture(lua_State* L, const char* what, const char* tex)
{
    if (tex != 0 && !ValidPath(tex, true))
        luaL_error(L, "%s: a texture is a file name in textures/ without \".png\" (not \"%s\")", what, tex);
}

int ContentBlock(lua_State* L)
{
    luaL_checkstack(L, 48, "content");
    const char* what = "content.block";
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);
    const char* path = FieldString(L, what, "id", true);
    CheckNewId(L, what, path);
    const char* name = FieldString(L, what, "name", false);
    const char* desc = FieldString(L, what, "description", false);
    const char* tex = FieldString(L, what, "texture", false);
    const char* side = FieldString(L, what, "side", false);
    const char* top = FieldString(L, what, "top", false);
    const char* bottom = FieldString(L, what, "bottom", false);
    CheckTexture(L, what, tex);
    CheckTexture(L, what, side);
    CheckTexture(L, what, top);
    CheckTexture(L, what, bottom);
    if (tex == 0 && side == 0)
        luaL_error(L, "%s \"%s\" needs \"texture\" (all faces) or \"side\"", what, path);
    const char* mat = FieldString(L, what, "material", false);
    if (mat != 0 && !InList(mat, materialNames, sizeof(materialNames) / sizeof(materialNames[0])))
        luaL_error(L, "%s: material \"%s\" is not stone, metal, wood, dirt, grass, sand, clay or wool", what, mat);
    const char* sound = FieldString(L, what, "sound", false);
    if (sound != 0 && !InList(sound, soundNames, sizeof(soundNames) / sizeof(soundNames[0])))
        luaL_error(L, "%s: sound \"%s\" is not stone, wood, gravel, grass, metal, glass, wool, sand or snow", what,
                   sound);
    const double hardness = FieldNumber(L, what, "hardness", 1.5, -1.0, 1000.0);
    const double resistance = FieldNumber(L, what, "resistance", -1.0, -1.0, 1000000.0);
    const double light = FieldNumber(L, what, "light", 0.0, 0.0, 15.0);
    const char* drops = FieldString(L, what, "drops", false);
    if (drops != 0 && strcmp(drops, "nothing") != 0 && !CheckRef(drops))
        luaL_error(L, "%s: drops \"%s\" is not \"nothing\" or a block or item name", what, drops);
    double dropMin = 1.0;
    double dropMax = 1.0;
    lua_getfield(L, 1, "drop_count");
    if (lua_type(L, -1) == LUA_TNUMBER)
        dropMin = dropMax = lua_tonumber(L, -1);
    else if (lua_type(L, -1) == LUA_TTABLE)
    {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        dropMin = lua_tonumber(L, -2);
        dropMax = lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    else if (!lua_isnil(L, -1))
        luaL_error(L, "%s: drop_count is a number or {min, max}", what);
    lua_pop(L, 1);
    if (!(dropMin >= 0 && dropMax >= dropMin && dropMax <= 64))
        luaL_error(L, "%s: drop_count must be from 0 to 64 (and min <= max)", what);
    const char* tab = FieldString(L, what, "tab", false);
    if (tab != 0 && TabByName(tab) < 0)
        luaL_error(L, "%s: tab \"%s\" is not none, building, decoration, redstone, transport, materials, food, "
                      "tools, brewing or misc",
                   what, tab);

    const std::string mod = StateOf(L)->modId;
    BlockDef def;
    def.mod = mod;
    def.path = path;
    def.key = mod + ":" + path;
    def.nameId = AddString(name != 0 ? name : path);
    def.descriptionId = AddString(desc != 0 ? desc : "");
    const std::string sideTex = side != 0 ? side : tex;
    def.textures[0] = AddTexture(0, mod, sideTex);
    def.textures[1] = AddTexture(0, mod, top != 0 ? std::string(top) : tex != 0 ? std::string(tex) : sideTex);
    def.textures[2] =
        AddTexture(0, mod, bottom != 0 ? std::string(bottom) : tex != 0 ? std::string(tex) : sideTex);
    def.material = mat != 0 ? mat : "stone";
    def.sound = sound != 0 ? sound : "";
    def.hardness = (float)hardness;
    def.resistance = (float)resistance;
    def.light = (int)light;
    def.dropsNothing = drops != 0 && strcmp(drops, "nothing") == 0;
    def.hasDrop = drops != 0 && !def.dropsNothing;
    if (def.hasDrop)
        def.drop = ParseRef(drops, mod);
    def.dropMin = (int)dropMin;
    def.dropMax = (int)dropMax;
    def.tab = tab != 0 ? TabByName(tab) : ePcModTab_Building;
    def.id = 0;
    def.block = 0;
    def.make = 0;
    modBlocks.push_back(def);
    return 0;
}

int ContentItem(lua_State* L)
{
    luaL_checkstack(L, 48, "content");
    const char* what = "content.item";
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);
    const char* path = FieldString(L, what, "id", true);
    CheckNewId(L, what, path);
    const char* name = FieldString(L, what, "name", false);
    const char* desc = FieldString(L, what, "description", false);
    const char* tex = FieldString(L, what, "texture", true);
    CheckTexture(L, what, tex);
    const double stack = FieldNumber(L, what, "stack", 64.0, 1.0, 64.0);
    const char* tab = FieldString(L, what, "tab", false);
    if (tab != 0 && TabByName(tab) < 0)
        luaL_error(L, "%s: tab \"%s\" is not a creative tab name", what, tab);
    double hunger = 0.0;
    double saturation = 0.6;
    bool meat = false;
    lua_getfield(L, 1, "food");
    const bool food = lua_istable(L, -1);
    if (food)
    {
        lua_getfield(L, -1, "hunger");
        lua_getfield(L, -2, "saturation");
        lua_getfield(L, -3, "meat");
        if (lua_type(L, -3) != LUA_TNUMBER || !(lua_tonumber(L, -3) >= 1 && lua_tonumber(L, -3) <= 20))
            luaL_error(L, "%s: food.hunger must be a number from 1 to 20", what);
        hunger = lua_tonumber(L, -3);
        if (!lua_isnil(L, -2))
        {
            if (lua_type(L, -2) != LUA_TNUMBER || !(lua_tonumber(L, -2) >= 0 && lua_tonumber(L, -2) <= 10))
                luaL_error(L, "%s: food.saturation must be a number from 0 to 10", what);
            saturation = lua_tonumber(L, -2);
        }
        meat = lua_toboolean(L, -1) != 0;
        lua_pop(L, 3);
    }
    else if (!lua_isnil(L, -1))
        luaL_error(L, "%s: food is a table {hunger = N, saturation = N, meat = true}", what);
    lua_pop(L, 1);

    const std::string mod = StateOf(L)->modId;
    ItemDef def;
    def.mod = mod;
    def.path = path;
    def.key = mod + ":" + path;
    def.nameId = AddString(name != 0 ? name : path);
    def.descriptionId = AddString(desc != 0 ? desc : "");
    def.texture = AddTexture(1, mod, tex);
    def.stack = (int)stack;
    def.tab = tab != 0 ? TabByName(tab) : (food ? ePcModTab_Food : ePcModTab_Misc);
    def.food = food;
    def.hunger = (int)hunger;
    def.saturation = (float)saturation;
    def.meat = meat;
    def.item = 0;
    def.make = 0;
    modItems.push_back(def);
    return 0;
}

int ContentRecipe(lua_State* L)
{
    luaL_checkstack(L, 48, "content");
    const char* what = "content.recipe";
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);
    const char* result = FieldRef(L, what, "result", true);
    const double count = FieldNumber(L, what, "count", 1.0, 1.0, 64.0);
    const char* group = FieldString(L, what, "group", false);
    if (group != 0 && GroupByName(group) < 0)
        luaL_error(L, "%s: group \"%s\" is not structures, tools, food, armour, mechanisms, vehicles or misc", what,
                   group);
    lua_getfield(L, 1, "pattern");
    const int pattern = lua_gettop(L);
    lua_getfield(L, 1, "key");
    const int key = lua_gettop(L);
    lua_getfield(L, 1, "ingredients");
    const int ingredients = lua_gettop(L);
    const bool shaped = lua_istable(L, pattern);
    if (shaped == lua_istable(L, ingredients))
        luaL_error(L, "%s for %s needs either pattern and key, or ingredients", what, result);
    if (shaped)
    {
        const lua_Integer rows = (lua_Integer)lua_rawlen(L, pattern);
        if (rows < 1 || rows > 3)
            luaL_error(L, "%s for %s: a pattern has 1 to 3 rows", what, result);
        size_t w = 0;
        for (lua_Integer row = 1; row <= rows; ++row)
        {
            lua_rawgeti(L, pattern, row);
            size_t len = 0;
            const char* s = lua_tolstring(L, -1, &len);
            if (s == 0 || len < 1 || len > 3 || (w != 0 && len != w))
                luaL_error(L, "%s for %s: pattern rows are strings of 1 to 3 characters, all as wide", what, result);
            w = len;
            for (size_t i = 0; i < len; ++i)
            {
                if (s[i] == ' ')
                    continue;
                if (!lua_istable(L, key))
                    luaL_error(L, "%s for %s needs a key", what, result);
                lua_pushlstring(L, s + i, 1);
                lua_rawget(L, key);
                const char* r = lua_tostring(L, -1);
                if (lua_type(L, -1) != LUA_TSTRING || !CheckRef(r))
                    luaL_error(L, "%s for %s: key '%c' must name a block or item", what, result, s[i]);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
    }
    else
    {
        const lua_Integer len = (lua_Integer)lua_rawlen(L, ingredients);
        if (len < 1 || len > 9)
            luaL_error(L, "%s for %s: 1 to 9 ingredients", what, result);
        for (lua_Integer i = 1; i <= len; ++i)
        {
            lua_rawgeti(L, ingredients, i);
            if (lua_type(L, -1) != LUA_TSTRING || !CheckRef(lua_tostring(L, -1)))
                luaL_error(L, "%s for %s: ingredient %d must name a block or item", what, result, (int)i);
            lua_pop(L, 1);
        }
    }

    const std::string mod = StateOf(L)->modId;
    RecipeDef def;
    def.mod = mod;
    def.shaped = shaped;
    def.result = ParseRef(result, mod);
    def.count = (int)count;
    def.group = group != 0 ? GroupByName(group) : Recipe::eGroupType_Miscellaneous;
    if (shaped)
    {
        const lua_Integer rows = (lua_Integer)lua_rawlen(L, pattern);
        for (lua_Integer row = 1; row <= rows; ++row)
        {
            lua_rawgeti(L, pattern, row);
            const std::string s = lua_tostring(L, -1);
            lua_pop(L, 1);
            def.rows.push_back(s);
            for (size_t i = 0; i < s.size(); ++i)
            {
                if (s[i] == ' ' || def.key.count(s[i]) != 0)
                    continue;
                lua_pushlstring(L, &s[i], 1);
                lua_rawget(L, key);
                def.key[s[i]] = ParseRef(lua_tostring(L, -1), mod);
                lua_pop(L, 1);
            }
        }
    }
    else
    {
        const lua_Integer len = (lua_Integer)lua_rawlen(L, ingredients);
        for (lua_Integer i = 1; i <= len; ++i)
        {
            lua_rawgeti(L, ingredients, i);
            def.ingredients.push_back(ParseRef(lua_tostring(L, -1), mod));
            lua_pop(L, 1);
        }
    }
    modRecipes.push_back(def);
    return 0;
}

int ContentSmelt(lua_State* L)
{
    luaL_checkstack(L, 48, "content");
    const char* what = "content.smelt";
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);
    const char* input = FieldRef(L, what, "input", true);
    const char* result = FieldRef(L, what, "result", true);
    const double count = FieldNumber(L, what, "count", 1.0, 1.0, 64.0);
    const double xp = FieldNumber(L, what, "xp", 0.1, 0.0, 100.0);
    const std::string mod = StateOf(L)->modId;
    SmeltDef def;
    def.mod = mod;
    def.input = ParseRef(input, mod);
    def.result = ParseRef(result, mod);
    def.count = (int)count;
    def.experience = (float)xp;
    modSmelts.push_back(def);
    return 0;
}

void OpenContentApi(lua_State* L)
{
    static const luaL_Reg contentFuncs[] = {{"block", ContentBlock},
                                        {"item", ContentItem},
                                        {"recipe", ContentRecipe},
                                        {"smelt", ContentSmelt},
                                        {0, 0}};
    lua_newtable(L);
    SetFunctions(L, contentFuncs);
    lua_setglobal(L, "content");
}

bool IsFile(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

template <class T>
void DropMod(std::vector<T>* list, const std::string& mod)
{
    for (size_t i = list->size(); i > 0; --i)
    {
        if ((*list)[i - 1].mod == mod)
            list->erase(list->begin() + (long)(i - 1));
    }
}

void LoadDefinitions()
{
    if (defsLoaded)
        return;
    defsLoaded = true;
    const std::vector<PcMod>& list = PcModsLoaded();
    for (size_t i = 0; i < list.size(); ++i)
    {
        const PcMod& m = list[i];
        if (!IsFile(m.directory + "/scripts/content.lua"))
            continue;
        if (m.side != "both")
        {
            printf("[pc] content: %s: scripts/content.lua is not run: blocks and items must be on every player's "
                   "game, so the mod's side must be \"both\" (it is \"%s\")\n",
                   m.id.c_str(), m.side.c_str());
            continue;
        }
        const size_t nBlocks = modBlocks.size();
        const size_t nItems = modItems.size();
        const size_t nRecipes = modRecipes.size();
        const size_t nSmelts = modSmelts.size();
        if (!RunOnce(eSide_Content, m, OpenContentApi))
        {
            DropMod(&modBlocks, m.id);
            DropMod(&modItems, m.id);
            DropMod(&modRecipes, m.id);
            DropMod(&modSmelts, m.id);
            printf("[pc] content: %s: none of its blocks, items or recipes are added\n", m.id.c_str());
            continue;
        }
        printf("[pc] content: %s: %u blocks, %u items, %u recipes, %u smelting recipes\n", m.id.c_str(),
               (unsigned int)(modBlocks.size() - nBlocks), (unsigned int)(modItems.size() - nItems),
               (unsigned int)(modRecipes.size() - nRecipes),
               (unsigned int)(modSmelts.size() - nSmelts));
    }
    for (size_t i = 0; i < modBlocks.size(); ++i)
    {
        if (modBlocks[i].hasDrop)
            ResolveNamespace(&modBlocks[i].drop);
    }
    for (size_t i = 0; i < modRecipes.size(); ++i)
    {
        ResolveNamespace(&modRecipes[i].result);
        for (std::map<char, Ref>::iterator it = modRecipes[i].key.begin(); it != modRecipes[i].key.end(); ++it)
            ResolveNamespace(&it->second);
        for (size_t k = 0; k < modRecipes[i].ingredients.size(); ++k)
            ResolveNamespace(&modRecipes[i].ingredients[k]);
    }
    for (size_t i = 0; i < modSmelts.size(); ++i)
    {
        ResolveNamespace(&modSmelts[i].input);
        ResolveNamespace(&modSmelts[i].result);
    }
    for (int atlas = 0; atlas < 2; ++atlas)
    {
        for (size_t i = 0; i < atlasEntries[atlas].size(); ++i)
        {
            if (!IsFile(atlasEntries[atlas][i].file))
                printf("[pc] content: missing texture %s\n", atlasEntries[atlas][i].file.c_str());
        }
    }
    fflush(stdout);
}

unsigned int Hash(const std::string& s)
{
    unsigned int h = 2166136261u;
    for (size_t i = 0; i < s.size(); ++i)
    {
        h ^= (unsigned char)s[i];
        h *= 16777619u;
    }
    return h;
}

bool ByKey(const BlockDef* a, const BlockDef* b)
{
    return a->key < b->key;
}

void AssignBlockIds()
{
    std::vector<BlockDef*> order;
    for (size_t i = 0; i < modBlocks.size(); ++i)
        order.push_back(&modBlocks[i]);
    std::sort(order.begin(), order.end(), ByKey);
    const int range = kPcModLastBlockId - kPcModFirstBlockId + 1;
    std::vector<bool> taken(range, false);
    for (size_t i = 0; i < order.size(); ++i)
    {
        if ((int)i >= range)
        {
            printf("[pc] content: no block id left for %s\n", order[i]->key.c_str());
            continue;
        }
        int slot = (int)(Hash(order[i]->key) % (unsigned int)range);
        while (taken[slot])
            slot = (slot + 1) % range;
        taken[slot] = true;
        order[i]->id = kPcModFirstBlockId + slot;
    }
}

Material* MaterialByName(const std::string& name)
{
    if (name == "metal")
        return Material::METAL;
    if (name == "wood")
        return Material::WOOD;
    if (name == "dirt")
        return Material::DIRT;
    if (name == "grass")
        return Material::GRASS;
    if (name == "sand")
        return Material::SAND;
    if (name == "clay")
        return Material::CLAY;
    if (name == "wool")
        return Material::WOOL;
    return Material::STONE;
}

const SoundType* SoundByName(const std::string& name, const std::string& material)
{
    const std::string s = !name.empty() ? name
        : material == "metal" ? "metal"
        : material == "wood" ? "wood"
        : material == "dirt" || material == "clay" ? "gravel"
        : material == "grass" ? "grass"
        : material == "sand" ? "sand"
        : material == "wool" ? "wool"
        : "stone";
    if (s == "wood")
        return SoundType::WOOD;
    if (s == "gravel")
        return SoundType::GRAVEL;
    if (s == "grass")
        return SoundType::GRASS;
    if (s == "metal")
        return SoundType::METAL;
    if (s == "glass")
        return SoundType::GLASS;
    if (s == "wool")
        return SoundType::CLOTH;
    if (s == "sand")
        return SoundType::SAND;
    if (s == "snow")
        return SoundType::SNOW;
    return SoundType::STONE;
}

Item* FindItem(const std::string& name)
{
    const ResourceLocation loc(Widen(name));
    return Item::itemRegistry.containsKey(loc) ? Item::itemRegistry.get(loc) : 0;
}

class PcModBlock : public Block
{
public:
    PcModBlock(Material* material, size_t index)
        : Block(material), mIndex(index), mTop(0), mBottom(0), mDropResolved(false), mDrop(0)
    {
        DerivedInit();
    }

    virtual void registerIcons(IconRegister* iconRegister)
    {
        Block::registerIcons(iconRegister);
        const BlockDef& def = modBlocks[mIndex];
        mTop = iconRegister->registerIcon(atlasEntries[0][def.textures[1]].sprite);
        mBottom = iconRegister->registerIcon(atlasEntries[0][def.textures[2]].sprite);
    }

    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state)
    {
        if (direction == Direction::UP && mTop != 0)
            return mTop;
        if (direction == Direction::DOWN && mBottom != 0)
            return mBottom;
        return Block::getTexture(direction, state);
    }

    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus)
    {
        const BlockDef& def = modBlocks[mIndex];
        if (!def.hasDrop)
            return Block::getResource(state, random, lootBonus);
        if (!mDropResolved)
        {
            mDropResolved = true;
            mDrop = FindItem(def.drop.name);
            if (mDrop == 0)
                printf("[pc] content: %s drops %s, which does not exist; it drops itself\n", def.key.c_str(),
                       def.drop.name.c_str());
        }
        return mDrop != 0 ? mDrop : Block::getResource(state, random, lootBonus);
    }

    virtual int getResourceCount(Random* random)
    {
        const BlockDef& def = modBlocks[mIndex];
        if (def.dropsNothing)
            return 0;
        return def.dropMax > def.dropMin ? def.dropMin + random->nextInt(def.dropMax - def.dropMin + 1) : def.dropMin;
    }

    virtual int getSpawnResourcesAuxValue(const BlockState* state)
    {
        const BlockDef& def = modBlocks[mIndex];
        return def.hasDrop && mDrop != 0 ? def.drop.data : 0;
    }

private:
    size_t mIndex;
    TextureAtlasSprite* mTop;
    TextureAtlasSprite* mBottom;
    bool mDropResolved;
    Item* mDrop;
};

class PcModItem : public Item
{
public:
    PcModItem() {}
};

ItemInstance* MakeInstance(const Ref& r, int count, bool ingredient)
{
    Item* item = FindItem(r.name);
    if (item == 0)
        return 0;
    return new ItemInstance(item, count, r.anyData && ingredient ? -1 : r.data);
}

struct Pixels
{
    int width;
    int height;
    std::vector<unsigned int> data;
};

bool LoadPng(const std::string& path, Pixels* pixels)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return false;
    std::vector<unsigned char> raw;
    unsigned char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0 && raw.size() <= MAX_TEXTURE_SIZE)
        raw.insert(raw.end(), buf, buf + n);
    fclose(fp);
    if (raw.empty() || raw.size() > MAX_TEXTURE_SIZE)
        return false;
    int* pix = 0;
    if (TextureUtil::LoadTextureData(&raw[0], (unsigned int)raw.size(), &pixels->width,
        &pixels->height, &pix) != 0
        || pix == 0 || pixels->width <= 0 || pixels->height <= 0)
        return false;
    pixels->data.assign(reinterpret_cast<unsigned int*>(pix),
                        reinterpret_cast<unsigned int*>(pix) + pixels->width * pixels->height);
    free(pix);
    return true;
}

unsigned int Sample(const Pixels& src, int x, int y, int size)
{
    const int side = std::min(src.width, src.height);
    const int x0 = x * side / size;
    const int y0 = y * side / size;
    const int x1 = std::max(x0 + 1, (x + 1) * side / size);
    const int y1 = std::max(y0 + 1, (y + 1) * side / size);
    unsigned long long aSum = 0;
    unsigned long long rSum = 0;
    unsigned long long gSum = 0;
    unsigned long long bSum = 0;
    unsigned long long n = 0;
    for (int sy = y0; sy < y1; ++sy)
    {
        for (int sx = x0; sx < x1; ++sx)
        {
            const unsigned int px = src.data[sy * src.width + sx];
            const unsigned int a = px >> 24;
            aSum += a;
            rSum += ((px >> 16) & 0xFF) * a;
            gSum += ((px >> 8) & 0xFF) * a;
            bSum += (px & 0xFF) * a;
            ++n;
        }
    }
    if (aSum == 0)
        return 0;
    return (unsigned int)(aSum / n) << 24 | (unsigned int)(rSum / aSum) << 16
           | (unsigned int)(gSum / aSum) << 8 | (unsigned int)(bSum / aSum);
}

struct WorldIds
{
    bool active;
    bool identity;
    std::map<std::string, int> ids;
    int load[BLOCK_ID_LIMIT];
    int save[BLOCK_ID_LIMIT];
    std::wstring missing;
};
WorldIds currentWorld;

std::map<std::string, int> RuntimeIds()
{
    std::map<std::string, int> ids;
    for (size_t i = 0; i < modBlocks.size(); ++i)
    {
        if (modBlocks[i].block != 0)
            ids[modBlocks[i].key] = modBlocks[i].id;
    }
    return ids;
}
}

int PcModContentAddName(const char* text)
{
    return AddString(text != 0 ? text : "");
}

bool PcModContentAddNativeBlock(const std::string& mod, const char* name, const char* displayName, int tab,
                                void* (*make)(int id))
{
    if (name == 0 || !ValidPath(name, true) || make == 0 || Declared(mod + ":" + name))
        return false;
    BlockDef def;
    def.mod = mod;
    def.path = name;
    def.key = mod + ":" + name;
    def.nameId = AddString(displayName != 0 ? displayName : name);
    def.descriptionId = AddString("");
    def.textures[0] = def.textures[1] = def.textures[2] = 0;
    def.hardness = 0.0f;
    def.resistance = -1.0f;
    def.light = 0;
    def.dropsNothing = false;
    def.hasDrop = false;
    def.dropMin = def.dropMax = 1;
    def.tab = tab >= 0 && tab <= (int)ePcModTab_Misc ? (PcModTab)tab : ePcModTab_None;
    def.id = 0;
    def.block = 0;
    def.make = make;
    modBlocks.push_back(def);
    return true;
}

bool PcModContentAddNativeItem(const std::string& mod, const char* name, const char* displayName, int tab,
                               void* (*make)(int id))
{
    if (name == 0 || !ValidPath(name, true) || make == 0 || Declared(mod + ":" + name))
        return false;
    ItemDef def;
    def.mod = mod;
    def.path = name;
    def.key = mod + ":" + name;
    def.nameId = AddString(displayName != 0 ? displayName : name);
    def.descriptionId = AddString("");
    def.texture = 0;
    def.stack = 64;
    def.tab = tab >= 0 && tab <= (int)ePcModTab_Misc ? (PcModTab)tab : ePcModTab_None;
    def.food = false;
    def.hunger = 0;
    def.saturation = 0.0f;
    def.meat = false;
    def.item = 0;
    def.make = make;
    modItems.push_back(def);
    return true;
}

const wchar_t* PcModContentAddNativeTexture(int atlas, const std::string& mod, const char* file)
{
    if (file == 0 || !ValidPath(file, true))
        return 0;
    int n = AddTexture(atlas == 0 ? 0 : 1, mod, file);
    return atlasEntries[atlas == 0 ? 0 : 1][n].sprite.c_str();
}

void PcModContentRegisterBlocks()
{
    PcNativeModsRegister();
    LoadDefinitions();
    AssignBlockIds();
    for (size_t i = 0; i < modBlocks.size(); ++i)
    {
        BlockDef& def = modBlocks[i];
        if (def.id == 0)
            continue;
        if (def.make != 0)
        {
            Block* nb = (Block*)def.make(def.id);
            if (nb == 0)
            {
                printf("[pc] content: %s: the mod made no block\n", def.key.c_str());
                continue;
            }
            nb->setNameAndDescriptionId(def.nameId, def.descriptionId);
            Block::registerBlock(def.id, ResourceLocation(Widen(def.mod), Widen(def.path)), nb);
            def.block = nb;
            continue;
        }
        PcModBlock* b = new PcModBlock(MaterialByName(def.material), i);
        b->setDestroyTime(def.hardness);
        if (def.resistance >= 0.0f)
            b->setExplodeable(def.resistance);
        b->setSoundType(SoundByName(def.sound, def.material));
        if (def.light > 0)
            b->setLightEmission((float)def.light / 15.0f);
        b->setIconName(atlasEntries[0][def.textures[0]].sprite);
        b->setNameAndDescriptionId(def.nameId, def.descriptionId);
        Block::registerBlock(def.id, ResourceLocation(Widen(def.mod), Widen(def.path)), b);
        def.block = b;
    }
    if (!modBlocks.empty())
    {
        printf("[pc] content: %u mod blocks registered\n", (unsigned int)modBlocks.size());
        fflush(stdout);
    }
}

void PcModContentRegisterItems()
{
    for (size_t i = 0; i < modBlocks.size(); ++i)
    {
        if (modBlocks[i].block != 0)
            Item::registerBlock(modBlocks[i].block);
    }
    std::vector<ItemDef*> order;
    for (size_t i = 0; i < modItems.size(); ++i)
        order.push_back(&modItems[i]);
    struct Compare
    {
        static bool ByKey(const ItemDef* a, const ItemDef* b) { return a->key < b->key; }
    };
    std::sort(order.begin(), order.end(), Compare::ByKey);
    const int range = LAST_ITEM_ID - FIRST_ITEM_ID + 1;
    for (size_t i = 0; i < order.size(); ++i)
    {
        ItemDef& def = *order[i];
        int slot = (int)(Hash(def.key) % (unsigned int)range);
        int tries = 0;
        while (Item::existsWithId(FIRST_ITEM_ID + slot) && tries < range)
        {
            slot = (slot + 1) % range;
            ++tries;
        }
        if (tries >= range)
        {
            printf("[pc] content: no item id left for %s\n", def.key.c_str());
            continue;
        }
        Item* item = 0;
        if (def.make != 0)
        {
            item = (Item*)def.make(FIRST_ITEM_ID + slot);
            if (item == 0)
            {
                printf("[pc] content: %s: the mod made no item\n", def.key.c_str());
                continue;
            }
        }
        else
        {
            item = def.food ? static_cast<Item*>(new FoodItem(def.hunger, def.saturation, def.meat))
                            : static_cast<Item*>(new PcModItem());
            item->setIconName(atlasEntries[1][def.texture].sprite);
            item->setMaxStackSize(def.stack);
        }
        item->setDescriptionId(def.nameId, def.descriptionId);
        Item::registerItem(FIRST_ITEM_ID + slot, ResourceLocation(Widen(def.mod), Widen(def.path)), item);
        def.item = item;
    }
}

void PcModContentAddRecipes(Recipes* recipes)
{
    unsigned int added = 0;
    for (size_t i = 0; i < modRecipes.size(); ++i)
    {
        const RecipeDef& def = modRecipes[i];
        ItemInstance* out = MakeInstance(def.result, def.count, false);
        bool ok = out != 0;
        if (def.shaped && ok)
        {
            const int w = (int)def.rows[0].size();
            const int h = (int)def.rows.size();
            ItemInstance** ingredients = new ItemInstance*[w * h];
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    const char ch = def.rows[y][x];
                    ItemInstance*& slot = ingredients[y * w + x];
                    slot = ItemInstance::EMPTY.get();
                    if (ch == ' ')
                        continue;
                    ItemInstance* ing = MakeInstance(def.key.find(ch)->second, 1, true);
                    if (ing == 0)
                        ok = false;
                    else
                        slot = ing;
                }
            }
            if (ok)
            {
                ShapedRecipe* r = new ShapedRecipe(w, h, ingredients, out, def.group);
                recipes->addCustomRecipe(r, Recipes::eRecipeCategory_Crafting);
            }
        }
        else if (ok)
        {
            std::vector<ItemInstance*>* ingredients = new std::vector<ItemInstance*>();
            for (size_t k = 0; k < def.ingredients.size() && ok; ++k)
            {
                ItemInstance* ing = MakeInstance(def.ingredients[k], 1, true);
                if (ing == 0)
                    ok = false;
                else
                    ingredients->push_back(ing);
            }
            if (ok)
            {
                ShapelessRecipe* r =
                    new ShapelessRecipe(out, ingredients, static_cast<Recipe::_eGroupType>(def.group));
                recipes->addCustomRecipe(r, Recipes::eRecipeCategory_Crafting);
            }
        }
        if (ok)
            ++added;
        else
            printf("[pc] content: %s: a recipe for %s names a block or item that does not exist; it is left out\n",
                   def.mod.c_str(), def.result.name.c_str());
    }
    if (!modRecipes.empty())
        printf("[pc] content: %u of %u mod recipes added\n", added, (unsigned int)modRecipes.size());
}

void PcModContentAddSmelting(FurnaceRecipes* furnace)
{
    for (size_t i = 0; i < modSmelts.size(); ++i)
    {
        const SmeltDef& def = modSmelts[i];
        Item* input = FindItem(def.input.name);
        ItemInstance* out = MakeInstance(def.result, def.count, false);
        if (input == 0 || out == 0)
        {
            printf("[pc] content: %s: smelting %s into %s names a block or item that does not exist\n",
                   def.mod.c_str(), def.input.name.c_str(), def.result.name.c_str());
            delete out;
            continue;
        }
        if (def.input.anyData || def.input.data == 0)
            furnace->addFurnaceRecipe(input, out, def.experience);
        else
            furnace->addFurnaceRecipe(new ItemInstance(input, 1, def.input.data), out, def.experience);
    }
}

const wchar_t* PcModContentString(int id)
{
    if ((id & STRING_MASK) != STRING_BASE)
        return 0;
    const size_t i = (size_t)(id & ~STRING_MASK);
    return i < modStrings.size() ? modStrings[i].c_str() : 0;
}

void PcModContentCreativeItems(std::vector<PcModCreativeEntry>* out)
{
    out->clear();
    for (size_t i = 0; i < modBlocks.size(); ++i)
    {
        if (modBlocks[i].block == 0 || modBlocks[i].tab == ePcModTab_None)
            continue;
        PcModCreativeEntry e;
        e.tab = modBlocks[i].tab;
        e.item = Item::byBlock(modBlocks[i].block);
        if (e.item != 0)
            out->push_back(e);
    }
    for (size_t i = 0; i < modItems.size(); ++i)
    {
        if (modItems[i].item == 0 || modItems[i].tab == ePcModTab_None)
            continue;
        PcModCreativeEntry e;
        e.tab = modItems[i].tab;
        e.item = modItems[i].item;
        out->push_back(e);
    }
}

std::vector<std::wstring> PcModContentAtlasSprites(int type)
{
    std::vector<std::wstring> out;
    const std::deque<AtlasEntry>& atlas = atlasEntries[type == 0 ? 0 : 1];
    for (size_t i = 0; i < atlas.size(); ++i)
        out.push_back(atlas[i].sprite);
    return out;
}

void PcModContentPatchAtlas(int type, BufferedImage* image)
{
    const std::deque<AtlasEntry>& atlas = atlasEntries[type == 0 ? 0 : 1];
    if (image == 0 || atlas.empty())
        return;
    const int baseRows = BASE_ROWS[type == 0 ? 0 : 1];
    const int w = (int)image->getWidth();
    const int tile = w / ATLAS_COLUMNS;
    const int rows = (int)((atlas.size() + ATLAS_COLUMNS - 1) / ATLAS_COLUMNS);
    if (tile <= 0 || (int)image->getHeight() != baseRows * tile)
    {
        printf("[pc] content: the %s atlas is %ux%u, not %d rows of %d; mod textures are not added\n",
               type == 0 ? "block" : "item", image->getWidth(), image->getHeight(), baseRows, tile);
        return;
    }
    image->PcAddRows((unsigned int)((baseRows + rows) * tile));
    for (size_t i = 0; i < atlas.size(); ++i)
    {
        Pixels src;
        if (!LoadPng(atlas[i].file, &src))
            continue;
        const int row = baseRows + (int)i / ATLAS_COLUMNS;
        const int col = (int)i % ATLAS_COLUMNS;
        for (int mip = 0; mip < 10; ++mip)
        {
            int* dst = image->getData(mip);
            const int levelTile = tile >> mip;
            if (dst == 0 || levelTile <= 0)
                break;
            const int levelWidth = w >> mip;
            for (int y = 0; y < levelTile; ++y)
            {
                for (int x = 0; x < levelTile; ++x)
                    dst[(row * levelTile + y) * levelWidth + col * levelTile + x] =
                        (int)Sample(src, x, y, levelTile);
            }
        }
    }
}

void PcModContentReadWorldIds(CompoundTag* tag, std::vector<std::pair<std::wstring, int> >* ids)
{
    ids->clear();
    if (!tag->contains(L"PcModBlocks", Tag::TAG_COMPOUND))
        return;
    CompoundTag* list = tag->getCompound(L"PcModBlocks");
    boost::unordered_map<std::wstring, Tag*>* m = list->getAllTagsMap();
    for (boost::unordered_map<std::wstring, Tag*>::const_iterator it = m->begin(); it != m->end(); ++it)
    {
        if (it->second != 0 && it->second->getId() == Tag::TAG_INT)
            ids->push_back(std::make_pair(it->first, list->getInt(it->first)));
    }
    std::sort(ids->begin(), ids->end());
}

void PcModContentWriteWorldIds(CompoundTag* tag, const std::vector<std::pair<std::wstring, int> >& ids)
{
    if (currentWorld.active ? currentWorld.ids.empty() : ids.empty())
        return;
    CompoundTag* list = new CompoundTag();
    if (currentWorld.active)
    {
        for (std::map<std::string, int>::const_iterator it = currentWorld.ids.begin(); it != currentWorld.ids.end(); ++it)
            list->putInt(Widen(it->first), it->second);
    }
    else
    {
        for (size_t i = 0; i < ids.size(); ++i)
            list->putInt(ids[i].first, ids[i].second);
    }
    tag->put(L"PcModBlocks", list);
}

void PcModContentBeginWorld(LevelData* levelData)
{
    WorldIds& w = currentWorld;
    w.active = false;
    w.ids.clear();
    w.missing.clear();
    for (int id = 0; id < BLOCK_ID_LIMIT; ++id)
    {
        w.load[id] = id < kPcModFirstBlockId ? id : 0;
        w.save[id] = id;
    }
    std::set<int> used;
    if (levelData != 0)
    {
        for (size_t i = 0; i < levelData->mModBlockIds.size(); ++i)
        {
            const int id = levelData->mModBlockIds[i].second;
            if (id < kPcModFirstBlockId || id > kPcModLastBlockId || used.count(id) != 0)
                continue;
            used.insert(id);
            w.ids[Utf8(levelData->mModBlockIds[i].first)] = id;
        }
    }
    const std::map<std::string, int> runtime = RuntimeIds();
    std::map<std::string, unsigned int> missing;
    for (std::map<std::string, int>::const_iterator it = w.ids.begin(); it != w.ids.end(); ++it)
    {
        std::map<std::string, int>::const_iterator match = runtime.find(it->first);
        if (match == runtime.end())
        {
            ++missing[it->first.substr(0, it->first.find(':'))];
            continue;
        }
        w.load[it->second] = match->second;
        w.save[match->second] = it->second;
    }
    int nextId = kPcModFirstBlockId;
    for (std::map<std::string, int>::const_iterator it = runtime.begin(); it != runtime.end(); ++it)
    {
        if (w.ids.count(it->first) != 0)
            continue;
        int id = it->second;
        if (used.count(id) != 0)
        {
            while (nextId <= kPcModLastBlockId && used.count(nextId) != 0)
                ++nextId;
            if (nextId > kPcModLastBlockId)
            {
                printf("[pc] content: this world has no block id left for %s\n", it->first.c_str());
                continue;
            }
            id = nextId;
        }
        used.insert(id);
        w.ids[it->first] = id;
        w.load[id] = it->second;
        w.save[it->second] = id;
    }
    w.identity = true;
    for (std::map<std::string, int>::const_iterator it = runtime.begin(); it != runtime.end(); ++it)
        w.identity = w.identity && w.save[it->second] == it->second;
    if (levelData != 0)
    {
        levelData->mModBlockIds.clear();
        for (std::map<std::string, int>::const_iterator it = w.ids.begin(); it != w.ids.end(); ++it)
            levelData->mModBlockIds.push_back(std::make_pair(Widen(it->first), it->second));
    }
    w.active = true;

    if (!missing.empty())
    {
        std::string names;
        for (std::map<std::string, unsigned int>::const_iterator it = missing.begin(); it != missing.end(); ++it)
        {
            char buf[32];
            snprintf(buf, sizeof(buf), it->second == 1 ? " (1 block)" : " (%u blocks)", it->second);
            names += (names.empty() ? "" : ", ") + it->first + buf;
        }
        printf("[pc] content: this world has blocks of mods that are not installed: %s; they load as air\n",
               names.c_str());
        w.missing = Widen("This world has blocks from mods that are not installed: " + names
                              + ". They turn into air as their chunks load. To keep them, exit without saving and "
                                "install the mods.");
    }
    if (!runtime.empty() || !w.ids.empty())
        printf("[pc] content: world block ids: %u recorded, %s\n", (unsigned int)w.ids.size(),
               w.identity ? "the same as this game's" : "translated on load and save");
    fflush(stdout);
}

void PcModContentEndWorld()
{
    currentWorld.active = false;
    currentWorld.missing.clear();
}

int PcModContentLoadBlockId(int id)
{
    if (!currentWorld.active || id < kPcModFirstBlockId)
        return id;
    return id < BLOCK_ID_LIMIT ? currentWorld.load[id] : 0;
}

int PcModContentSaveBlockId(int id)
{
    if (!currentWorld.active || id < kPcModFirstBlockId || id >= BLOCK_ID_LIMIT)
        return id;
    return currentWorld.save[id];
}

bool PcModContentSaveIsIdentity()
{
    return !currentWorld.active || currentWorld.identity;
}

std::wstring PcModContentMissingNotice()
{
    return currentWorld.active ? currentWorld.missing : std::wstring();
}

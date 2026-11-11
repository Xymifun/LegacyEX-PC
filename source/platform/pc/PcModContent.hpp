#ifndef PLATFORM_PC_PCMODCONTENT_HPP
#define PLATFORM_PC_PCMODCONTENT_HPP

#include <string>
#include <vector>

class BufferedImage;
class CompoundTag;
class FurnaceRecipes;
class Item;
class LevelData;
class Recipes;

const int kPcModFirstBlockId = 319;
const int kPcModLastBlockId = 999;

void PcModContentRegisterBlocks();
void PcModContentRegisterItems();
void PcModContentAddRecipes(Recipes* recipes);
void PcModContentAddSmelting(FurnaceRecipes* furnace);

bool PcModContentAddNativeBlock(const std::string& mod, const char* name, const char* displayName, int tab,
                                void* (*make)(int id));
bool PcModContentAddNativeItem(const std::string& mod, const char* name, const char* displayName, int tab,
                               void* (*make)(int id));
const wchar_t* PcModContentAddNativeTexture(int atlas, const std::string& mod, const char* file);

const wchar_t* PcModContentString(int id);
int PcModContentAddName(const char* text);

enum PcModTab
{
    ePcModTab_None,
    ePcModTab_Building,
    ePcModTab_Decoration,
    ePcModTab_Redstone,
    ePcModTab_Transport,
    ePcModTab_Materials,
    ePcModTab_Food,
    ePcModTab_Tools,
    ePcModTab_Brewing,
    ePcModTab_Misc
};

struct PcModCreativeEntry
{
    PcModTab tab;
    Item* item;
};
void PcModContentCreativeItems(std::vector<PcModCreativeEntry>* entries);

std::vector<std::wstring> PcModContentAtlasSprites(int iconType);
void PcModContentPatchAtlas(int iconType, BufferedImage* image);

void PcModContentBeginWorld(LevelData* levelData);
void PcModContentEndWorld();
void PcModContentReadWorldIds(CompoundTag* tag, std::vector<std::pair<std::wstring, int> >* ids);
void PcModContentWriteWorldIds(CompoundTag* tag, const std::vector<std::pair<std::wstring, int> >& ids);
int PcModContentLoadBlockId(int id);
int PcModContentSaveBlockId(int id);
bool PcModContentSaveIsIdentity();
std::wstring PcModContentMissingNotice();

#endif

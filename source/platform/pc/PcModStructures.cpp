#include "platform/pc/PcModStructures.hpp"

#include "platform/pc/PcNativeMod.h"
#include "platform/pc/PcNativeMods.hpp"

#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

#include <stdio.h>
#include <string.h>
#include <set>

namespace
{
const unsigned int FIRST_MOD_TYPE = 1000;

struct StructDef
{
    std::wstring key;
    int dims;
    PcNativeMakeStructure make;
    PcNativeStructureMobs mobs;
    size_t mod;
};

std::vector<StructDef> structs;
std::set<std::wstring> startNames;
std::set<std::wstring> pieceNames;
unsigned int nextStart = FIRST_MOD_TYPE;
unsigned int nextPiece = FIRST_MOD_TYPE;

std::wstring Widen(const std::string& s)
{
    std::wstring w;
    for (size_t i = 0; i < s.size(); i++)
        w += (wchar_t)(unsigned char)s[i];
    return w;
}

bool GoodName(const char* name)
{
    if (name == 0 || name[0] == 0 || strlen(name) > 64)
        return false;
    for (const char* p = name; *p != 0; p++)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_'))
            return false;
    return true;
}

void Say(const char* what, const char* name)
{
    printf("[pc] native %s: %s \"%s\" not added\n", PcNativeModsCurrentId(), what, name != 0 ? name : "");
    fflush(stdout);
}

int DimBit(int dimension)
{
    if (dimension == 0)
        return PC_NATIVE_OVERWORLD;
    if (dimension == -1)
        return PC_NATIVE_NETHER;
    if (dimension == 1)
        return PC_NATIVE_END;
    return 0;
}

int AddStructure(const char* name, int dims, PcNativeMakeStructure make, PcNativeStructureMobs mobs)
{
    std::wstring key = Widen(std::string(PcNativeModsCurrentId()) + ":" + (name != 0 ? name : ""));
    bool taken = false;
    for (size_t i = 0; i < structs.size(); i++)
        if (structs[i].key == key)
            taken = true;
    if (!GoodName(name) || make == 0 || taken || (dims & (PC_NATIVE_OVERWORLD | PC_NATIVE_NETHER | PC_NATIVE_END)) == 0)
    {
        Say("structure", name);
        return 0;
    }
    StructDef d;
    d.key = key;
    d.dims = dims;
    d.make = make;
    d.mobs = mobs;
    d.mod = PcNativeModsCurrent();
    structs.push_back(d);
    return 1;
}

unsigned int AddStructureStart(const char* name, PcNativeMakeStructureStart make)
{
    std::wstring key = Widen(std::string(PcNativeModsCurrentId()) + ":" + (name != 0 ? name : ""));
    if (!GoodName(name) || make == 0 || startNames.count(key) != 0)
    {
        Say("structure start", name);
        return 0;
    }
    startNames.insert(key);
    unsigned int type = nextStart++;
    StructureFeatureIO::setStartId((EStructureStart)type, (StructureFeatureIO::StructureStartFactory)make, key);
    return type;
}

unsigned int AddStructurePiece(const char* name, PcNativeMakeStructurePiece make)
{
    std::wstring key = Widen(std::string(PcNativeModsCurrentId()) + ":" + (name != 0 ? name : ""));
    if (!GoodName(name) || make == 0 || pieceNames.count(key) != 0)
    {
        Say("structure piece", name);
        return 0;
    }
    pieceNames.insert(key);
    unsigned int type = nextPiece++;
    StructureFeatureIO::setPieceId((EStructurePiece)type, (StructureFeatureIO::StructurePieceFactory)make, key);
    return type;
}
}

PcModFeatures* PcModStructuresMake(Level* level, int dimension, void* levelSource)
{
    PcModFeatures* list = new PcModFeatures();
    int bit = DimBit(dimension);
    for (size_t i = 0; i < structs.size(); i++)
    {
        if ((structs[i].dims & bit) == 0)
            continue;
        size_t prev = PcNativeModsCurrent();
        PcNativeModsSetCurrent(structs[i].mod);
        StructureFeature* f = (StructureFeature*)structs[i].make(level, levelSource);
        PcNativeModsSetCurrent(prev);
        if (f == 0)
            continue;
        PcModFeature mf;
        mf.key = structs[i].key;
        mf.feature = f;
        mf.mobs = structs[i].mobs;
        list->push_back(mf);
    }
    return list;
}

void PcModStructuresFree(PcModFeatures* list)
{
    if (list == 0)
        return;
    for (size_t i = 0; i < list->size(); i++)
        delete (*list)[i].feature;
    delete list;
}

void PcModStructuresApply(PcModFeatures* list, Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
{
    if (list == 0)
        return;
    for (size_t i = 0; i < list->size(); i++)
        (*list)[i].feature->apply(level, chunkX, chunkZ, primer);
}

void PcModStructuresPostProcess(PcModFeatures* list, Level* level, Random& random, ChunkPos* pos)
{
    if (list == 0)
        return;
    for (size_t i = 0; i < list->size(); i++)
        (*list)[i].feature->postProcess(level, random, pos);
}

StructureFeature* PcModStructuresFind(PcModFeatures* list, const std::wstring& name)
{
    if (list == 0)
        return 0;
    for (size_t i = 0; i < list->size(); i++)
        if ((*list)[i].key == name)
            return (*list)[i].feature;
    return 0;
}

std::vector<Biome::MobSpawnerData*>* PcModStructuresMobs(PcModFeatures* list, MobCategory* category, const BlockPos& pos)
{
    if (list == 0)
        return 0;
    for (size_t i = 0; i < list->size(); i++)
    {
        PcModFeature& mf = (*list)[i];
        if (mf.mobs == 0)
            continue;
        void* mobs = mf.mobs(mf.feature, category, &pos);
        if (mobs != 0)
            return (std::vector<Biome::MobSpawnerData*>*)mobs;
    }
    return 0;
}

void PcModStructuresFillRegister(PcNativeRegister* reg)
{
    reg->addStructure = AddStructure;
    reg->addStructureStart = AddStructureStart;
    reg->addStructurePiece = AddStructurePiece;
}

#include "game/core/GameRuleManager.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcTutorials.hpp"
#endif

#include <boost/unordered_set.hpp>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/DLCGameRulesHeader.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/GameRulesSource.hpp"
#include "game/data/StringTable.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/LevelGenerators.hpp"

static const unsigned int CRC_POLYNOMIAL = 0xedb88320U;
static const unsigned int CRC_INITIAL = 0xffffffffU;
static const unsigned int FULL_LICENSE_MASK = 0xffffffffU;

static unsigned int gGameRuleCRCTable[256];
static bool gGameRuleCRCTableReady = false;

const wchar_t* GameRuleManager::sAttributeNames[ConsoleGameRules::eGameRuleAttr_Count] = {L"descriptionName",
                                                                                          L"promptName",
                                                                                          L"dataTag",
                                                                                          L"enchantmentId",
                                                                                          L"enchantmentLevel",
                                                                                          L"itemId",
                                                                                          L"quantity",
                                                                                          L"auxValue",
                                                                                          L"slot",
                                                                                          L"name",
                                                                                          L"food",
                                                                                          L"health",
                                                                                          L"blockId",
                                                                                          L"useCoords",
                                                                                          L"seed",
                                                                                          L"flatworld",
                                                                                          L"filename",
                                                                                          L"rot",
                                                                                          L"data",
                                                                                          L"block",
                                                                                          L"entity",
                                                                                          L"facing",
                                                                                          L"edgeBlock",
                                                                                          L"fillBlock",
                                                                                          L"skipAir",
                                                                                          L"x",
                                                                                          L"x0",
                                                                                          L"x1",
                                                                                          L"y",
                                                                                          L"y0",
                                                                                          L"y1",
                                                                                          L"z",
                                                                                          L"z0",
                                                                                          L"z1",
                                                                                          L"chunkX",
                                                                                          L"chunkZ",
                                                                                          L"yRot",
                                                                                          L"xRot",
                                                                                          L"spawnX",
                                                                                          L"spawnY",
                                                                                          L"spawnZ",
                                                                                          L"orientation",
                                                                                          L"dimension",
                                                                                          L"topblockId",
                                                                                          L"biomeId",
                                                                                          L"feature",
                                                                                          L"minCount",
                                                                                          L"maxCount",
                                                                                          L"weight",
                                                                                          L"id",
                                                                                          L"probability",
                                                                                          L"method",
                                                                                          L"hasBeenInCreative",
                                                                                          L"cloudHeight",
                                                                                          L"fogDistance",
                                                                                          L"dayTime",
                                                                                          L"target",
                                                                                          L"speed",
                                                                                          L"dir",
                                                                                          L"type",
                                                                                          L"pass",
                                                                                          L"for",
                                                                                          L"random",
                                                                                          L"blockAux",
                                                                                          L"size",
                                                                                          L"scale",
                                                                                          L"freq",
                                                                                          L"func",
                                                                                          L"upper",
                                                                                          L"lower",
                                                                                          L"dY",
                                                                                          L"thickness",
                                                                                          L"points",
                                                                                          L"holeSize",
                                                                                          L"variant",
                                                                                          L"startHeight",
                                                                                          L"pattern",
                                                                                          L"colour",
                                                                                          L"primary",
                                                                                          L"laps",
                                                                                          L"liftForceModifier",
                                                                                          L"staticLift",
                                                                                          L"targetHeight",
                                                                                          L"speedBoost",
                                                                                          L"boostDirection",
                                                                                          L"condition_type",
                                                                                          L"condition_value_0",
                                                                                          L"condition_value_1",
                                                                                          L"beam_length",
                                                                                          L"beam_direction",
                                                                                          L"string_IDS"};

const wchar_t* GameRuleManager::sTagNames[ConsoleGameRules::eGameRuleType_Count] = {L"",
                                                                                    L"MapOptions",
                                                                                    L"ApplySchematic",
                                                                                    L"GenerateStructure",
                                                                                    L"GenerateBox",
                                                                                    L"PlaceBlock",
                                                                                    L"PlaceContainer",
                                                                                    L"PlaceSpawner",
                                                                                    L"BiomeOverride",
                                                                                    L"StartFeature",
                                                                                    L"AddItem",
                                                                                    L"AddEnchantment",
                                                                                    L"WeighedTreasureItem",
                                                                                    L"RandomItemSet",
                                                                                    L"DistributeItems",
                                                                                    L"WorldPosition",
                                                                                    L"LevelRules",
                                                                                    L"NamedArea",
                                                                                    L"ActiveChunkArea",
                                                                                    L"TargetArea",
                                                                                    L"ScoreRing",
                                                                                    L"ThermalArea",
                                                                                    L"PlayerBoundsVolume",
                                                                                    L"Killbox",
                                                                                    L"BlockLayer",
                                                                                    L"UseBlock",
                                                                                    L"CollectItem",
                                                                                    L"CompleteAll",
                                                                                    L"UpdatePlayer",
                                                                                    L"OnGameStartSpawnPositions",
                                                                                    L"OnInitialiseWorld",
                                                                                    L"SpawnPositionSet",
                                                                                    L"PopulateContainer",
                                                                                    L"DegradationSequence",
                                                                                    L"RandomDissolveDegrade",
                                                                                    L"DirectionalDegrade",
                                                                                    L"GrantPermissions",
                                                                                    L"AllowIn",
                                                                                    L"LayerGeneration",
                                                                                    L"LayerAsset",
                                                                                    L"AnyCombinationOf",
                                                                                    L"CombinationDefinition",
                                                                                    L"Variations",
                                                                                    L"BlockDef",
                                                                                    L"LayerSize",
                                                                                    L"UniformSize",
                                                                                    L"RandomizeSize",
                                                                                    L"LinearBlendSize",
                                                                                    L"LayerShape",
                                                                                    L"BasicShape",
                                                                                    L"StarShape",
                                                                                    L"PatchyShape",
                                                                                    L"RingShape",
                                                                                    L"SpiralShape",
                                                                                    L"LayerFill",
                                                                                    L"BasicLayerFill",
                                                                                    L"CurvedLayerFill",
                                                                                    L"WarpedLayerFill",
                                                                                    L"LayerTheme",
                                                                                    L"NullTheme",
                                                                                    L"FilterTheme",
                                                                                    L"ShaftsTheme",
                                                                                    L"BasicPatchesTheme",
                                                                                    L"BlockStackTheme",
                                                                                    L"RainbowTheme",
                                                                                    L"TerracottaTheme",
                                                                                    L"FunctionPatchesTheme",
                                                                                    L"SimplePatchesTheme",
                                                                                    L"CarpetTrapTheme",
                                                                                    L"MushroomBlockTheme",
                                                                                    L"TextureTheme",
                                                                                    L"SchematicTheme",
                                                                                    L"BlockCollisionException",
                                                                                    L"Powerup",
                                                                                    L"Checkpoint",
                                                                                    L"CustomBeacon",
                                                                                    L"ActiveViewArea"};

GameRuleManager::GameRuleManager()
{
    mDebugGameRuleDefinitions = NULL;
    mGameRuleDefinitions = NULL;
    mLevelGenerationOptions = NULL;
    mLevelGenerators = new LevelGenerators();
    mLevelRules = new LevelRules();
}

void GameRuleManager::unloadCurrentGameRules()
{
    LevelRuleset* definitions = mGameRuleDefinitions;
    LevelGenerationOptions* options;
    if (definitions != NULL)
    {
        options = mLevelGenerationOptions;
        if (options->isFromSave())
        {
            mLevelRules->removeLevelRule(definitions);
            definitions = mGameRuleDefinitions;
            delete definitions;
            options = mLevelGenerationOptions;
        }
    }
    else
    {
        options = mLevelGenerationOptions;
    }
    if (options != NULL)
    {
        if (options->isFromSave())
        {
            mLevelGenerators->removeLevelGenerator(options);
            options = mLevelGenerationOptions;
            if (options != NULL)
            {
                delete options;
                mLevelGenerationOptions = NULL;
                mGameRuleDefinitions = NULL;
                return;
            }
        }
        else if (options->isFromDLC())
        {
            options->reset_finish();
        }
    }
    mLevelGenerationOptions = NULL;
    mGameRuleDefinitions = NULL;
}

void GameRuleManager::setLevelGenerationOptions(LevelGenerationOptions* options)
{
    unloadCurrentGameRules();
    mGameRuleDefinitions = NULL;
    mLevelGenerationOptions = options;
    mDebugGameRuleDefinitions = NULL;
    if (mLevelGenerationOptions != NULL)
    {
        if (mLevelGenerationOptions->requiresGameRules())
        {
            mGameRuleDefinitions = options->getRequiredGameRules();
        }
        options->reset_start();
    }
}

void GameRuleManager::MakeCRCTable()
{
    unsigned int c;
    int n;
    int k;
    for (n = 0; n < 256; n++)
    {
        c = (unsigned int)n;
        for (k = 0; k < 8; k++)
        {
            if (c & 1)
                c = CRC_POLYNOMIAL ^ (c >> 1);
            else
                c = c >> 1;
        }
        gGameRuleCRCTable[n] = c;
    }
    gGameRuleCRCTableReady = true;
}

unsigned int GameRuleManager::UpdateCRC(unsigned int crc, unsigned char* data, int length)
{
    static const unsigned char increments[56] = {5, 6, 7, 3, 4, 5, 6, 7, 1, 2, 6, 1, 1, 2, 3, 4, 5, 5, 6,
                                                 7, 3, 4, 5, 6, 7, 1, 2, 6, 1, 1, 2, 3, 4, 5, 5, 6, 7, 3,
                                                 4, 5, 6, 7, 7, 5, 4, 7, 5, 4, 7, 5, 4, 7, 6, 5, 4, 3};
    if (!gGameRuleCRCTableReady)
        MakeCRCTable();
    int n = 0;
    unsigned int increment = 0;
    while (n < length)
    {
        crc = gGameRuleCRCTable[(crc ^ data[n]) & 0xff] ^ (crc >> 8);
        n += increments[increment++];
        increment %= 56;
    }
    return crc;
}

unsigned int GameRuleManager::CRC(unsigned char* data, int length)
{
    return ~UpdateCRC(CRC_INITIAL, data, length);
}

unsigned int GameRuleManager::addLevelGenerationOptions(LevelGenerationOptions* options)
{
    std::vector<LevelGenerationOptions*>* generators = mLevelGenerators->getLevelGenerators();
    unsigned int i;
    for (i = 0; i < generators->size(); ++i)
    {
        if (generators->at(i) == options)
        {
            return i;
        }
    }
    generators->push_back(options);
    return generators->size() - 1;
}

void GameRuleManager::readAttributes(DataInputStream* input, std::vector<std::wstring>* names,
                                     GameRuleDefinition* definition)
{
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        unsigned int nameIndex = input->readInt();
        std::wstring value = input->readUTF();
        if (definition != NULL)
        {
            definition->addAttribute(names->at(nameIndex), value);
        }
    }
    definition->onAttributesAdded();
}

void GameRuleManager::readChildren(DataInputStream* input, std::vector<std::wstring>* names,
                                   boost::unordered_map<int, ConsoleGameRules::EGameRuleType>* types,
                                   GameRuleDefinition* definition)
{
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        int id = input->readInt();
        ConsoleGameRules::EGameRuleType type = static_cast<ConsoleGameRules::EGameRuleType>(-1);
        boost::unordered_map<int, ConsoleGameRules::EGameRuleType>::iterator found = types->find(id);
        if (found != types->end())
        {
            type = found->second;
        }
        GameRuleDefinition* child = definition == NULL ? NULL : definition->addChild(type);
        readAttributes(input, names, child);
        readChildren(input, names, types, child);
    }
}

unsigned int GameRuleManager::readRuleFile(LevelGenerationOptions* options, unsigned char* data, unsigned int size,
                                           StringTable* table)
{
    arrayWithLength<unsigned char> inputData(data, size);
    ByteArrayInputStream bytes(inputData);
    DataInputStream input(&bytes);
    long long version = input.readShort();
    unsigned char compression = 0;
    unsigned int expectedCRC = 0;
    if (version == 0)
    {
        for (int i = 0; i < 14; ++i)
            input.readByte();
    }
    else
    {
        compression = input.readByte();
        expectedCRC = input.readInt();
        input.readBoolean();
        input.readBoolean();
        input.readBoolean();
        input.readBoolean();
    }
    ByteArrayInputStream* expandedBytes = NULL;
    DataInputStream* rules;
    if (compression == 0)
    {
        rules = &input;
    }
    else
    {
        unsigned int expandedSize = input.readInt();
        unsigned int compressedSize = input.readInt();
        arrayWithLength<unsigned char> compressed(compressedSize, true);
        input.read(compressed);
        options->setCRCIsValid(CRC(compressed.data, compressed.length) == expectedCRC);
        arrayWithLength<unsigned char> expanded(expandedSize, true);
        switch (compression)
        {
        case 0:
            memcpy(expanded.data, compressed.data, expandedSize);
            break;
        case 1:
            Compression::getCompression()->Decompress(expanded.data, &expanded.length, compressed.data,
                                                      compressed.length);
            break;
        default:
            Compression::getCompression()->DecompressLZXRLE(expanded.data, &expanded.length, compressed.data,
                                                            compressed.length);
            break;
        }
        delete[] compressed.data;
        expandedBytes = new ByteArrayInputStream(expanded);
        rules = new DataInputStream(expandedBytes);
    }
    int stringCount = rules->readInt();
    std::vector<std::wstring> names;
    names.reserve(stringCount);
    for (int i = 0; i < stringCount; ++i)
    {
        names.push_back(rules->readUTF());
    }
    boost::unordered_map<int, ConsoleGameRules::EGameRuleType> types;
    for (int type = 0; type < ConsoleGameRules::eGameRuleType_Count; ++type)
    {
        for (int name = 0; name < stringCount; ++name)
        {
            if (names[name] == sTagNames[type])
            {
                types[name] = static_cast<ConsoleGameRules::EGameRuleType>(type);
                break;
            }
        }
    }
    int schematicCount = rules->readInt();
    for (int schematic = 0; schematic < schematicCount; ++schematic)
    {
        std::wstring name = rules->readUTF();
        unsigned int schematicSize = rules->readInt();
        arrayWithLength<unsigned char> schematicData(schematicSize, true);
        rules->read(schematicData);
        options->loadSchematicFile(name, schematicData.data, schematicData.length);
        delete[] schematicData.data;
    }
    int rootCount = rules->readInt();
    for (int root = 0; root < rootCount; ++root)
    {
        int nameIndex = rules->readInt();
        ConsoleGameRules::EGameRuleType type = static_cast<ConsoleGameRules::EGameRuleType>(-1);
        boost::unordered_map<int, ConsoleGameRules::EGameRuleType>::iterator found = types.find(nameIndex);
        if (found != types.end())
            type = found->second;
        GameRuleDefinition* definition = NULL;
        if (type == ConsoleGameRules::eGameRuleType_MapOptions)
        {
            addLevelGenerationOptions(options);
            options->loadStringTable(table);
            definition = options;
        }
        else if (type == ConsoleGameRules::eGameRuleType_LevelRules)
        {
            LevelRuleset* ruleset = new LevelRuleset();
            options->setRequiredGameRules(ruleset);
            ruleset->setGenerationOptions(options);
            ruleset->loadStringTable(table);
            mLevelRules->addLevelRule(L"", ruleset);
            definition = ruleset;
        }
        readAttributes(rules, &names, definition);
        readChildren(rules, &names, &types, definition);
    }
    if (compression != 0)
    {
        rules->close();
        delete expandedBytes;
        delete rules;
    }
    input.close();
    bytes.mBuffer.data = NULL;
    bytes.mBuffer.length = 0;
    bytes.mCount = 0;
    bytes.mMark = 0;
    bytes.mPosition = 0;
    return 1;
}

void GameRuleManager::loadGameRules(DLCPack* pack)
{
    StringTable* table = NULL;
    if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, std::wstring(L"languages.loc"), true, true))
    {
        DLCLocalisationFile* localisation = static_cast<DLCLocalisationFile*>(
            pack->getFile(DLCManager::eDLCType_Localisation, std::wstring(L"languages.loc"), true, true));
        table = localisation->getStringTable();
    }
    int count = pack->getDLCItemsCount(DLCManager::eDLCType_GameRulesHeader);
    for (int i = 0; i < count; ++i)
    {
        DLCGameRulesHeader* header = static_cast<DLCGameRulesHeader*>(
            pack->getFile(DLCManager::eDLCType_GameRulesHeader, i, true, true));
        unsigned int size;
        unsigned char* data = header->getData(size);
        LevelGenerationOptions* options = new LevelGenerationOptions(pack);
        options->setGrSource(header);
        options->setSrc(LevelGenerationOptions::eSrc_DLC);
        readRuleFile(options, data, size, table);
        header->m_levelGenerationOptions = options;
    }
    count = pack->getDLCItemsCount(DLCManager::eDLCType_GameRules);
    for (int i = 0; i < count; ++i)
    {
        DLCFile* file = pack->getFile(DLCManager::eDLCType_GameRules, i, true, true);
        unsigned int size;
        unsigned char* data = file->getData(size);
        LevelGenerationOptions* options = new LevelGenerationOptions(pack);
        options->setGrSource(new GameRulesSource());
        options->setSrc(LevelGenerationOptions::eSrc_Local);
        readRuleFile(options, data, size, table);
        options->setLoadedData();
    }
}

void GameRuleManager::loadGameRules(LevelGenerationOptions* options, unsigned char* data, unsigned int size)
{
    arrayWithLength<unsigned char> inputData;
    inputData.data = data;
    inputData.length = size;
    ByteArrayInputStream bytes(inputData);
    DataInputStream input(&bytes);
    input.readShort();
    for (int i = 0; i < 8; ++i)
        input.readByte();
    unsigned char decompressionType = input.readByte();
    unsigned int compressedSize = input.readInt();
    unsigned int expandedSize = input.readInt();
    arrayWithLength<unsigned char> expanded(expandedSize, true);
    arrayWithLength<unsigned char> compressed(compressedSize, true);
    input.read(compressed);
    Compression* compression = Compression::getCompression();
    compression->m_decompressionType = static_cast<Compression::ECompressionTypes>(decompressionType);
    compression->DecompressLZXRLE(expanded.data, &expanded.length, compressed.data, compressed.length);
    compression->SetDecompressionType(eSavePlatform_WIIU);
    input.close();
    bytes.close();
    delete[] compressed.data;
    compressed.data = NULL;
    ByteArrayInputStream rulesBytes(expanded);
    DataInputStream rules(&rulesBytes);
    unsigned int tableSize = rules.readInt();
    arrayWithLength<unsigned char> tableData(tableSize, true);
    rules.read(tableData);
    StringTable* table = new StringTable(tableData, true, true);
    unsigned int remaining = expanded.length - tableSize;
    arrayWithLength<unsigned char> ruleData(remaining, true);
    rules.read(ruleData);
    if (readRuleFile(options, ruleData.data, ruleData.length, table) != 0)
    {
        options->setSrc(LevelGenerationOptions::eSrc_Save);
        setLevelGenerationOptions(options);
    }
    else
    {
        delete options;
        delete table;
    }
    rules.close();
    rulesBytes.close();
}

LevelGenerationOptions* GameRuleManager::loadGameRules(unsigned char* data, unsigned int size)
{
    LevelGenerationOptions* options = new LevelGenerationOptions();
    options->setGrSource(new GameRulesSource());
    options->setSrc(LevelGenerationOptions::eSrc_Save);
    loadGameRules(options, data, size);
    options->setLoadedData();
    return options;
}

LevelGenerationOptions* GameRuleManager::getLevelGenerationOptions()
{
    return mLevelGenerationOptions;
}

void GameRuleManager::writeRuleFile(DataOutputStream* output)
{
    output->writeShort(2);
    output->writeByte(0);
    for (int i = 0; i < 8; ++i)
        output->writeBoolean(false);
    boost::unordered_map<int, int> tags;
    {
        boost::unordered_set<int> tagTypes;
        boost::unordered_set<int> attributes;
        mLevelGenerationOptions->findDistinctTagsAndAttributes(tagTypes, attributes);
        mGameRuleDefinitions->findDistinctTagsAndAttributes(tagTypes, attributes);
        output->writeInt(tagTypes.size() + attributes.size());
        int key = 0;
        for (int type = 0; type < ConsoleGameRules::eGameRuleType_Count; ++type)
        {
            if (tagTypes.find(type) != tagTypes.end())
            {
                output->writeUTF(sTagNames[type]);
                tags[static_cast<ConsoleGameRules::EGameRuleType>(type)] = key++;
            }
        }
        for (int attribute = 0; attribute < ConsoleGameRules::eGameRuleAttr_Count; ++attribute)
        {
            if (attributes.find(attribute) != attributes.end())
            {
                output->writeUTF(sAttributeNames[attribute]);
                tags[attribute + ConsoleGameRules::eGameRuleType_Count] = key++;
            }
        }
    }
    LevelGenerationOptions::SchematicFileMap* schematics = getLevelGenerationOptions()->getUnfinishedSchematicFiles();
    output->writeInt(schematics->size());
    LevelGenerationOptions::SchematicFileMap::iterator schematic;
    for (schematic = schematics->begin(); schematic != schematics->end(); ++schematic)
    {
        std::wstring name = schematic->first;
        ConsoleSchematicFile* file = schematic->second;
        ByteArrayOutputStream schematicBytes;
        DataOutputStream schematicOutput(&schematicBytes);
        file->save(&schematicOutput);
        output->writeUTF(name);
        output->writeInt(schematicBytes.mBuffer.length);
        output->write(schematicBytes.mBuffer);
        schematicOutput.close();
        schematicBytes.close();
    }
    delete schematics;
    output->writeInt(2);
    mLevelGenerationOptions->write(output, &tags);
    mGameRuleDefinitions->write(output, &tags);
}

void GameRuleManager::saveGameRules(unsigned char** data, unsigned int* size)
{
    if (mGameRuleDefinitions == NULL && mLevelGenerationOptions == NULL)
    {
        *data = NULL;
        *size = 0;
        return;
    }
    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeShort(1);
    for (int i = 0; i < 8; ++i)
        output.writeByte(0);
    output.writeByte(3);
    ByteArrayOutputStream rulesBytes;
    DataOutputStream rules(&rulesBytes);
    LevelRuleset* definitions = mGameRuleDefinitions;
    if (definitions == NULL)
    {
        rules.writeInt(0);
        rules.writeInt(2);
        rules.writeByte(0);
        rules.writeByte(0);
        rules.writeByte(0);
        rules.writeInt(0);
        rules.writeInt(0);
        rules.writeInt(0);
    }
    else if (definitions->getStringTable() != NULL)
    {
        arrayWithLength<unsigned char> stringData;
        definitions->getStringTable()->getData(&stringData.data, &stringData.length);
        rules.writeInt(stringData.length);
        rules.write(stringData);
        writeRuleFile(&rules);
    }
    arrayWithLength<unsigned char> compressed(rulesBytes.mBuffer.length, false);
    Compression::getCompression()->CompressLZXRLE(compressed.data, &compressed.length, rulesBytes.mBuffer.data,
                                                  rulesBytes.mBuffer.length, NULL);
    output.writeInt(compressed.length);
    output.writeInt(rulesBytes.mBuffer.length);
    output.write(compressed);
    delete[] compressed.data;
    rules.close();
    rulesBytes.close();
    *data = bytes.mBuffer.data;
    *size = bytes.mBuffer.length;
    bytes.mBuffer.data = NULL;
    output.close();
    bytes.close();
}

void GameRuleManager::processSchematics(LevelChunk* chunk)
{
    LevelGenerationOptions* options = getLevelGenerationOptions();
    if (options != NULL)
    {
        options->processSchematics(chunk);
    }
}

void GameRuleManager::processSchematicsLighting(LevelChunk* chunk)
{
    LevelGenerationOptions* options = getLevelGenerationOptions();
    if (options != NULL)
    {
        options->processSchematicsLighting(chunk);
    }
}

unsigned int GameRuleManager::loadGameRulesPack(File* file)
{
    bool loaded = false;
    if (file->exists())
    {
        DLCPack* pack = new DLCPack(std::wstring(L""), FULL_LICENSE_MASK);
        unsigned int count = 0;
        if (gApp.mDLCManager.readDLCDataFile(count, file->getPath(), pack, 0, false))
        {
            gApp.mDLCManager.addPack(pack);
            loaded = true;
        }
        else
        {
            delete pack;
        }
    }
    return loaded;
}

void GameRuleManager::loadDefaultGameRules()
{
    unsigned int index = mLevelGenerators->getLevelGenerators()->size();
    std::wstring root(L"WiiU/Lobby/");
    File file(root + L"Lobby.pck");
    if (loadGameRulesPack(&file) != 0)
    {
        mLevelGenerators->getLevelGenerators()->at(index)->setBaseSaveRoot(root);
        mLevelGenerators->getLevelGenerators()->at(index)->setIsGameModeLobby(true);
    }
#ifdef MINECRAFT_PC
    for (int version = 0; PcTutorialVersionFolder(version) != 0; ++version)
    {
        std::wstring folder = std::wstring(L"PC/Tutorials/") + PcTutorialVersionFolder(version) + L"/";
        File pack(folder + L"Tutorial.pck");
        index = mLevelGenerators->getLevelGenerators()->size();
        if (loadGameRulesPack(&pack) == 0 || mLevelGenerators->getLevelGenerators()->size() <= index)
            continue;
        LevelGenerationOptions* options = mLevelGenerators->getLevelGenerators()->at(index);
        options->setBaseSaveRoot(folder);
        PcTutorialRegister(version, options);
    }
#endif
}

std::vector<LevelGenerationOptions*>* GameRuleManager::getLevelGenerators()
{
    return mLevelGenerators->getLevelGenerators();
}

LevelRuleset* GameRuleManager::getGameRuleDefinitions()
{
    return mGameRuleDefinitions;
}

const wchar_t* GameRuleManager::GetGameRulesString(const std::wstring& key)
{
    if (mGameRuleDefinitions != NULL && !key.empty())
        return mGameRuleDefinitions->getString(key);
    return L"";
}

#ifndef GAME_WORLD_GENERATION_STRUCTUREFEATUREIO_HPP
#define GAME_WORLD_GENERATION_STRUCTUREFEATUREIO_HPP

#include <string>

#include <boost/unordered_map.hpp>

enum EStructureStart
{
    eStructureStart_MineShaft = 0,
    eStructureStart_Village = 1,
    eStructureStart_NetherBridge = 2,
    eStructureStart_Stronghold = 3,
    eStructureStart_ScatteredFeature = 4,
    eStructureStart_OceanMonument = 5,
    eStructureStart_EndCity = 6,
    eStructureStart_WoodlandMansion = 7,
    eStructureStart_BuriedTreasure = 8,
    eStructureStart_Shipwreck = 9,
    eStructureStart_OceanRuin = 10
};

#include "game/world/generation/StructurePiece.hpp"

class ByteArrayTag;
class CompoundTag;
class DataInputStream;
class Level;
class StructureStart;

class StructureFeatureIO
{
public:
    typedef StructureStart* (*StructureStartFactory)();
    typedef StructurePiece* (*StructurePieceFactory)();

    static void staticCtor();

    static void setStartId(EStructureStart id, StructureStartFactory factory, const std::wstring& name);
    static void setPieceId(EStructurePiece id, StructurePieceFactory factory, const std::wstring& name);

    static int getVersion();

    static std::wstring getEncodeId(StructureStart* start);
    static std::wstring getEncodeId(StructurePiece* piece);

    static StructureStart* loadStaticStart(CompoundTag* tag, Level* level);
    static StructureStart* loadStaticStart(ByteArrayTag* tag, Level* level);

    static StructurePiece* loadStaticPiece(CompoundTag* tag, Level* level);
    static StructurePiece* loadStaticPiece(DataInputStream* input, Level* level);

private:
    static boost::unordered_map<std::wstring, StructureStartFactory> startFactories;
    static boost::unordered_map<unsigned int, std::wstring> startIds;
    static boost::unordered_map<std::wstring, StructurePieceFactory> pieceFactories;
    static boost::unordered_map<unsigned int, std::wstring> pieceIds;
    static int version;
};

#endif

#ifndef GAME_WORLD_BLOCK_MONSTEREGGBLOCK_HPP
#define GAME_WORLD_BLOCK_MONSTEREGGBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class Random;
class TextureAtlasSprite;
template <class T> class EnumProperty;

class MonsterEggBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name, int translationKey);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getData();
        int getTranslationKey();
        const BlockState* getHostBlockState();
        static Variant* byData(int data);
        static Variant* byHostBlock(const BlockState* state);

        static Variant* STONE;
        static Variant* COBBLESTONE;
        static Variant* STONEBRICK;
        static Variant* MOSSY_STONEBRICK;
        static Variant* CRACKED_STONEBRICK;
        static Variant* CHISELED_STONEBRICK;
        static Variant* VALUES[6];

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    static EnumProperty<Variant*>* VARIANT;

    MonsterEggBlock();
    static void blockStaticCtor();
    virtual int getResourceCount(Random* random);
    static bool isCompatibleHostBlock(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif

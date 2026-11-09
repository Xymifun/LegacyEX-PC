#ifndef GAME_WORLD_BLOCK_LOGBLOCK_HPP
#define GAME_WORLD_BLOCK_LOGBLOCK_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"

class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LivingEntity;
class Rotation;
class TextureAtlasSprite;

template <typename T>
class EnumProperty;

class LogBlock : public RotatedPillarBlock
{
public:
    class LogAxis;
    static LogAxis* X;
    static LogAxis* Y;
    static LogAxis* Z;
    static LogAxis* NONE;
    static EnumProperty<LogAxis*>* AXIS;

    class LogAxis : public StringRepresentable
    {
    public:
        static LogAxis* values[4];

        LogAxis(const std::wstring& name);

        static LogAxis* fromAxis(Direction::Axis* axis);

        unsigned int getId() const;
        Direction::Axis* getAxis();

        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

    private:
        std::wstring mName;
    };

    LogBlock();
    virtual ~LogBlock() {}

    static void blockStaticCtor();

    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual int getDescriptionId(int auxValue);
    virtual bool isStrippable();
    virtual const BlockState* getStrippedBlock(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual TextureAtlasSprite* getTypeTexture(const BlockState* state);
    virtual TextureAtlasSprite* getTopTexture(const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual Direction::Axis* getAxis(const BlockState* state);
    virtual int getWoodType(const BlockState* state) = 0;

private:
    static std::wstring TEXTURE_NAMES[6];

    TextureAtlasSprite* mTypeTextures[6];
    TextureAtlasSprite* mTopTextures[6];
};

#endif

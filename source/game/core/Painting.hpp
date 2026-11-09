#ifndef GAME_CORE_PAINTING_HPP
#define GAME_CORE_PAINTING_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/HangingEntity.hpp"

class BlockPos;
class CompoundTag;
class Direction;
class Entity;
class Level;

class Painting : public HangingEntity
{
public:
    class Motive
    {
    public:
        Motive(std::wstring name, int width, int height, int textureX, int textureY);

        static const int MAX_MOTIVE_NAME_LENGTH;

        std::wstring name;
        int width;
        int height;
        int textureX;
        int textureY;
    };

    Painting(Level* level);
    Painting(Level* level, const BlockPos& pos, const Direction* direction);
    Painting(Level* level, const BlockPos& pos, const Direction* direction, const std::wstring& motiveName);

    static Entity* create(Level* level);
    void _init(Level* level);
    void PaintingPostConstructor(const Direction* direction, int unused);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual int getWidth();
    virtual int getHeight();
    virtual void dropItem(boost::shared_ptr<Entity> breaker);
    virtual void playPlacementSound();
    virtual void moveTo(double x, double y, double z, float yRot, float xRot);
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual unsigned long long GetType();

    Motive* motive;
};

#endif

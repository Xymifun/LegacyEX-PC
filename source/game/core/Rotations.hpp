#ifndef GAME_CORE_ROTATIONS_HPP
#define GAME_CORE_ROTATIONS_HPP

class ListTag;

class Rotations
{
public:
    Rotations(float x, float y, float z);
    Rotations(const ListTag* tag);

    ListTag* save() const;

    bool equals(const Rotations* other) const;

    float getX() const;
    float getY() const;
    float getZ() const;

    float getWrappedX() const;
    float getWrappedY() const;
    float getWrappedZ() const;

private:
    float x;
    float y;
    float z;
};

#endif

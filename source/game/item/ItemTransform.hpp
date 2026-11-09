#ifndef GAME_ITEM_ITEMTRANSFORM_HPP
#define GAME_ITEM_ITEMTRANSFORM_HPP

#include "game/core/Vector3f.hpp"

class ItemTransform
{
public:
    static ItemTransform* NO_TRANSFORM;

    Vector3f rotation;
    Vector3f translation;
    Vector3f scale;

    ItemTransform(Vector3f rotation, Vector3f translation, Vector3f scale, bool clampValues);

    static ItemTransform* deserialize(float rotationX, float rotationY, float rotationZ, float translationX,
                                      float translationY, float translationZ, float scaleX, float scaleY, float scaleZ);
};

#endif
